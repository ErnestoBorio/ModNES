byte read_ram( void *sys, word address );
void write_ram( void *sys, word address, byte value );

byte read_ram_mirror( void *sys, word address );
void write_ram_mirror( void *sys, word address, byte value );

byte read_save_ram( void *sys, word address );
void write_save_ram( void *sys, word address, byte value );

void write_ppu_control1( void *sys, word address, byte value );
void write_ppu_control2( void *sys, word address, byte value );
byte read_ppu_status( void *sys, word address );

void write_spr_ram_address( void *sys, word address, byte value );
byte read_spr_ram_io( void *sys, word address );
void write_spr_ram_io( void *sys, word address, byte value );

void write_scroll( void *sys, word address, byte value  );
void write_vram_address( void *sys, word address, byte value );
byte read_vram_io( void *sys, word address );
void write_vram_io( void *sys, word address, byte value  );
void write_sprite_dma( void *sys, word address, byte value );

byte read_gamepad( void *sys, word address );
void write_gamepad( void *sys, word address, byte value );

void write_prg_switch( void *sys, word address, byte value );
// Writes to $8000-$FFFF select 16kB banks that switch on low PRG $8000-$BFFF
// void write_prg_switch_16kB( void *sys, word address );
// Writes to $8000-$FFFF select 32kB banks that switch on PRG $8000-$FFFF
// void write_prg_switch_32kB( void *sys, word address );

// Reads to $8000-$BFFF read from low 16 kB PRG ROM or Reads to $8000-$FFFF read from full 32kB PRG ROM
byte read_prg_rom_low_bank( void *sys, word address );
// Reads to $C000-$FFFF read from fixed high 16 kB PRG ROM
byte read_prg_rom_high_bank( void *sys, word address );
// Reads to $C000-$FFFF read from low 16 kB PRG ROM as a mirror
byte read_prg_rom_high_mirror( void *sys, word address );

byte read_unimplemented( void *sys, word address );
void write_unimplemented( void *sys, word address, byte value );