#ifndef _Nes_h_
    #define _Nes_h_

#include <stdio.h>
#include "Cpu6502.h"

#define bit_value( _byte, bit_order ) ( ( _byte & ( 1 << bit_order ) ) >> bit_order )

#define PRG_ROM_bank_size 0x4000 // PRG-ROM bank is 16kB
#define CHR_ROM_bank_size 0x2000 // CHR-ROM bank is  8kB
#define CHR_UNPACKED_size 0x100 * 8 * 8 // 0x100 tiles * 8 px tall * 8 px wide = 0x4000 bytes at 1 byte per pixel = 16Kb

// const byte nes_palette[64][3];

enum {
    mirroring_vertical   = 0,
    mirroring_horizontal = 1,
    mirroring_4screens   = 2
};

enum Nes_Buttons {
    Nes_A      = 0,
    Nes_B      = 1,
    Nes_Select = 2,
    Nes_Start  = 3,
    Nes_Up     = 4,
    Nes_Down   = 5,
    Nes_Left   = 6,
    Nes_Right  = 7
};

enum Nes_Strobe {
    Nes_Strobe_clear = 0,
    Nes_Strobe_reset = 1,
    Nes_Strobe_reading = 2,
    Nes_Strobe_init = 3
};

typedef struct // Nes
{
    Cpu6502 *cpu;

    byte *chr_rom; // Chunk with all CHR-ROM banks
    int chr_rom_count; // How many 8kB CHR-ROM banks are present
    byte *chr_unpacked; // 1 byte per pixel translation of CHR-ROM
    byte *chr_unpacked_ptr[2];

    byte *prg_rom; // Chunk with all PRG-ROM banks
    int prg_rom_count; // How many 16kB PRG-ROM banks are present

    byte ram[0x800]; // Built-in 2kB of RAM
    byte save_ram[0x2000]; // Battery backed RAM

    int scanline;        // scanline number currently being rendered [-1..260]
    int last_scanline;   // last scanline redndered, to detect scanline change
    int scanpixel;       // pixel number of current scanline being rendered [0..340]
    int last_scanpixel;  // to compare the range of pixels rendered during last step
    int frames;          // frames rendered since reset
    int vblank;          // internal vblank flag that is not reset when read
    long cpu_cycles;     // CPU cycles executed since reset
    long ppu_cycles;     // PPU cycles executed since reset (3 PPU cycles per each CPU cycle)

    struct
    {
        // $2000
        byte nmi_enabled;
        byte sprite_height;  // 8|16
        word back_pattern;   // 0|$1000
        word sprite_pattern; // 0|$1000
        byte increment_vram; // 1|32

        // $2001
        byte color_emphasis; // 0..7
        byte sprites_visible;
        byte background_visible;
        byte sprite_clip;
        byte background_clip;
        byte monochrome;
            
        // $2002
        byte vblank_flag;
        byte sprite0_hit;
        byte sprites_lost;

        byte write_count; // writes counter for $2005 & $2006. 0 = no write yet. 1 = one write done, waiting for 2nd.
        word vram_address; // VRAM address to read from or write to
        byte vram_latch;
        
        // http://wiki.nesdev.com/w/index.php/PPU_registers#Controller_.28.242000.29_.3E_write
        struct {
            byte horizontal_high; // 1 means add 256 to horizontal (or nametable # if you will)
            byte vertical_high;   // 1 means add 240 to vertical
            byte horizontal_low;
            byte vertical_low;
            int horizontal;
            int vertical;
            struct {
                int start_x;
                int start_y;
                struct {
                    int scanline;
                    int scroll_x;
                } midframe_x[10];
                int midframe_count;
            } last_frame;
        } scroll;

        byte mirroring;
        byte *name_attr;   // Chunk of memory for 2 name tables and their attributes
        byte *name_ptr[4]; // pointers to the 4 virtual name tables (2 real)
        byte *attr_ptr[4]; // pointers to the 4 virtual attribute tables (2 real)
        byte palettes[0x20]; // WIP should memory be malloc'ed? the Nes itself is malloc'ed anyway.
        byte sprites[0x100];
   } ppu;
   
    struct {
        byte gamepad[2][8]; // Holds the pressed state [0|1] of the buttons of both gamepads
        byte strobe_state;
        byte read_count[2];
    } input;
   
} Nes;

#ifdef __cplusplus
    extern "C" {
#endif
    
    Nes *Nes_Create();
    void Nes_Reset( Nes *dis );
    void Nes_Free( Nes *dis );
    int  Nes_LoadRom( Nes *dis, FILE *rom_file );
    void Nes_DoFrame( Nes *dis );
    const byte *Nes_GetPaletteColor( Nes *dis, byte area, byte palette, byte index );
    void Nes_SetInputState( Nes *dis, byte gampead, byte button, byte state );

#ifdef __cplusplus
    }
#endif
    
// const byte Nes_rgb[64][3];
// Suddenly gives error: Default initialization of an object of const type 'const byte [64][3]'

#endif // #ifndef _Nes_h_