#include <string.h>
#include <assert.h>
#include "Nes.h"

#define NES ((Nes*)sys) // some syntax de-clutter to compensate for the unfortunate void *sys

// -------------------------------------------------------------------------------
// $0..$7FF unmirrored RAM
byte read_ram( void *sys, word address )
{
	return NES->ram[address];
}

void write_ram( void *sys, word address, byte value )
{
	NES->ram[address] = value;
}

// -------------------------------------------------------------------------------
// $800..$1FFF mirrored RAM
byte read_ram_mirror( void *sys, word address )
{
	return NES->ram[address & 0x7FF]; // Convert mirrors to actual address
}

void write_ram_mirror( void *sys, word address, byte value )
{
	NES->ram[address & 0x7FF] = value; // Convert mirrors to actual address
}

// -------------------------------------------------------------------------------
byte read_save_ram( void *sys, word address )
{
   return NES->save_ram[ address - 0x6000 ];
}

void write_save_ram( void *sys, word address, byte value )
{
   NES->save_ram[ address - 0x6000 ] = value;
}

// -------------------------------------------------------------------------------
// $8000..$FFFF PRG-ROM
byte read_prg_rom( void *sys, word address )
{
	address -= 0x8000; // make address zero-based ( $0..$7FFF )
	if( NES->prg_rom_count == 1 ) // WIP bankswitching hell here in the future
	{
		address &= 0x3FFF; // Convert mirror in actual ROM address ( $0..$3FFF )		
	}
	return NES->prg_rom[address];
}

// -------------------------------------------------------------------------------
// $2000
void write_ppu_control1( void *sys, word address, byte value )
{
    NES->ppu.nmi_enabled    = ( value & (1<<7) ) ? 1 : 0;
    NES->ppu.sprite_height  = ( value & (1<<5) ) ? 16 : 8;
    NES->ppu.back_pattern   = ( value & (1<<4) ) ? 0x1000 : 0;
    NES->ppu.sprite_pattern = ( value & (1<<3) ) ? 0x1000 : 0;
    NES->ppu.increment_vram = ( value & (1<<2) ) ? 32 : 1;
    
    NES->ppu.scroll.horizontal = 
        ( NES->ppu.scroll.horizontal & 0xFF ) | // keep 8 lower bits
        (( value & 1 ) << 8 ); // write bit 0 of port to bit 8 of coarse X
    
    NES->ppu.scroll.vertical = 
        ( NES->ppu.scroll.vertical & 0xFF ) | // keep 8 lower bits
        (( value & 2 ) <<7 ); // write bit 1 of port to bit 8 of coarse Y
}
// -------------------------------------------------------------------------------
// $2001
void write_ppu_control2( void *sys, word address, byte value )
{
    NES->ppu.color_emphasis     = ( value & 0xE0 ) >>5; // & %11100000
    NES->ppu.sprites_visible    = ( value & (1<<4) ) ? 1 : 0;
    NES->ppu.background_visible = ( value & (1<<3) ) ? 1 : 0;
    NES->ppu.sprite_clip        = ( value & (1<<2) ) ? 0 : 1;
    NES->ppu.background_clip    = ( value & (1<<1) ) ? 0 : 1;
    NES->ppu.monochrome         = ( value & (1<<1) ) ? 1 : 0;
    if( NES->ppu.sprites_visible != NES->ppu.background_visible ) {
        printf( "Visibility sprites: %d != background: %d (Not handled yet)\n", NES->ppu.sprites_visible, NES->ppu.background_visible );
    }
}
// -------------------------------------------------------------------------------
// $2002
byte read_ppu_status( void *sys, word address )
{
   // Unused bits should actually return the open bus
   byte value = 
      ( NES->ppu.vblank_flag  <<7 ) |
      ( NES->ppu.sprite0_hit  <<6 ) |
      ( NES->ppu.sprites_lost <<5 );
   
   NES->ppu.vblank_flag = 0; // reset flag once read
   NES->ppu.write_count = 0; // writes count is reset
   
   // #ifdef _Cpu6502_Disassembler
   //    NES->cpu->disasm.value = value;
   // #endif
   
   return value;
}
// -------------------------------------------------------------------------------
// $2003
void write_spr_ram_address( void *sys, word address, byte value  )
{
//   assert( 0 && "sprite RAM address register not yet implemented"  );
}
// -------------------------------------------------------------------------------
// $2004
byte read_spr_ram_io( void *sys, word address )
{
//   assert( 0 && "Read from sprite RAM not yet implemented"  );
   return 0;
}
// -------------------------------------------------------------------------------
void write_spr_ram_io( void *sys, word address, byte value  )
{
//   assert( 0 && "Write to sprite RAM not yet implemented"  );
}
// -------------------------------------------------------------------------------
// $2005
void write_scroll( void *sys, word address, byte value  )
{
    if( NES->ppu.write_count == 0 ) {
        
        NES->ppu.scroll.horizontal = 
            ( NES->ppu.scroll.horizontal & (1<<8) ) | value; // keep bit 8 coarse X

        NES->ppu.write_count = 1;
    }
    else {
        int int_value = value;
        NES->ppu.scroll.vertical = NES->ppu.scroll.vertical & (1<<8); // keep bit 8 coarse Y
        if( int_value > 240 ) {
            int_value = -1 * ( int_value - 240 ); // past 240, values are conssidered negative
        }
        NES->ppu.scroll.vertical += int_value;
        NES->ppu.write_count = 0;
    }
}
// -------------------------------------------------------------------------------
// $2006
void write_vram_address( void *sys, word register_address, byte value  )
{
    // WIP: It's actually more complex, but this will do for games without raster FX
    // See: http://wiki.nesdev.com/w/index.php/PPU_scrolling
    NES->ppu.scroll.horizontal = NES->ppu.scroll.vertical = value;
    
    if( NES->ppu.write_count == 0 ) {
        NES->ppu.vram_address = ((word) value & 0x3F ) <<8; // put 6 bits of value in vram_address msb
        NES->ppu.write_count = 1;
    }
    else {
        NES->ppu.vram_address |= value;
        NES->ppu.write_count = 0;
    }
}
// -------------------------------------------------------------------------------
// $2007
byte read_vram_io( void *sys, word register_address )
{
    byte old_latch = NES->ppu.vram_latch;

    if( NES->ppu.write_count > 0 ) {
        assert( 0 && "Trying to read from VRAM after only setting half of VRAM address, what to do here?" );
    }

    word vram_address = NES->ppu.vram_address;

    // Palettes
    if( vram_address >= 0x3F00 )
    {
        vram_address &= 0x1F; // Make VRAM address zero based and Unmirror
        if( vram_address == 0x10 || vram_address == 0x14 || vram_address == 0x18 || vram_address == 0x1C ) {
            vram_address -= 0x10; // Sprite colors 0 mirror background colors 0
        }
        // NES->ppu.vram_latch = NES->ppu.palettes[ vram_address ];
        // return NES->ppu.vram_latch;
        return NES->ppu.palettes[ vram_address ];
    }
    // Name tables and attributes
    else if( vram_address >= 0x2000 ) {
        vram_address &= 0x7FF; // Make VRAM address zero based
        NES->ppu.vram_latch = NES->ppu.name_attr[ vram_address ];
    }
    // WIP: else tries to read from pattern tables, do nothing for now
    else {
        NES->ppu.vram_latch = NES->chr_rom[ vram_address ];
        // printf( "Reading VRAM $%4x < $2000\n", vram_address );
        // assert(0);
    }

    NES->ppu.vram_address += NES->ppu.increment_vram;
    NES->ppu.vram_address &= 0x3FFF; // Wrap around $4000

    return old_latch;
}
// -------------------------------------------------------------------------------
void write_vram_io( void *sys, word register_address, byte value  )
{
    if( NES->ppu.write_count > 0 ) {
        assert( 0 && "Trying to write to VRAM after only setting half of VRAM address, what to do here?" );
    }

    word vram_address = NES->ppu.vram_address;

    // Palettes
    if( vram_address >= 0x3F00 )
    {
        assert( NES->ppu.vram_address < 0x4000 ); // WIP remove this once checked
        vram_address &= 0x1F; // Make VRAM address zero based and Unmirror
        if( vram_address == 0x10 || vram_address == 0x14 || vram_address == 0x18 || vram_address == 0x1C ) {
            vram_address -= 0x10; // Sprite colors 0 mirror background colors 0
        }
        NES->ppu.palettes[ vram_address ] = value & 0x3F;
    }
    // Name tables and attributes
    else if( NES->ppu.vram_address > 0x2000 ) {
        vram_address &= 0x7FF; // Make VRAM address zero based and Unmirror
        NES->ppu.name_attr[ vram_address ] = value;
    }
    // WIP: else tries to write to pattern tables, do nothing for now

    NES->ppu.vram_address += NES->ppu.increment_vram;
    NES->ppu.vram_address &= 0x3FFF; // Wrap around $4000
}
// -------------------------------------------------------------------------------
// $4014
// WIP: OAM DMA starts on RAM address written to $2003
void write_sprite_dma( void *sys, word address, byte value )
{
   if( value > 0x1F ) {
      assert( 0 && "Copying sprite DMA from outside RAM, weird." );
   }
   address = ( value & 0x7 ) <<8; // shamelessly reusing unused `address`
   memcpy( NES->ppu.sprites, &NES->ram[address], 0x100 );
   int cpu_cycles = ( NES->cpu_cycles % 2 == 1 ) ? 514 : 513; // +1 cycle on odd CPU cycles
   NES->cpu_cycles += cpu_cycles;
   NES->ppu_cycles += 3 * cpu_cycles;
}
// -------------------------------------------------------------------------------
// $4016
byte read_gamepad( void *sys, word address )
{
   byte value = 0;
   if( NES->input.strobe_state == Nes_Strobe_clear )
   {
      NES->input.strobe_state = Nes_Strobe_reading;
      NES->input.read_count[0] = NES->input.read_count[1] = 0;
   }
   if( NES->input.strobe_state == Nes_Strobe_reading )
   {
      if(( address == 0x4016 ) && ( NES->input.read_count[0] <= 7 )) {
         value = NES->input.gamepad[0][ NES->input.read_count[0] ];
         NES->input.read_count[0]++;
      }
      else if(( address == 0x4017 ) && ( NES->input.read_count[1] <= 7 )) {
         value = NES->input.gamepad[1][ NES->input.read_count[1] ];
         NES->input.read_count[1]++;
      }
   }
   return value;
}

void write_gamepad( void *sys, word address, byte value )
{
   // Ignore writes to 0x4017 for now
   if( address == 0x4016 )
   {
      if( value == 1 ) {
         NES->input.strobe_state = Nes_Strobe_reset;
      }
      else if( value == 0 ) {
         if( NES->input.strobe_state == Nes_Strobe_reset ) {
            NES->input.strobe_state = Nes_Strobe_clear;
         }
         // else: what should happen if 0 is written when not in reset state? WIP
      }
      // else: Some games write $C0 here for APU thingies. Ignore for now WIP
   }
}
// -------------------------------------------------------------------------------
byte read_unimplemented( void *sys, word address )
{
    printf( "Memory read not implemented for address: $%X\n", address );
	assert( 0 );
	return 0;
}
// -------------------------------------------------------------------------------
void write_unimplemented( void *sys, word address, byte value )
{
	printf( "Memory write not implemented for address: $%X, value: $%X\n", address, value );
    assert( 0 );
}