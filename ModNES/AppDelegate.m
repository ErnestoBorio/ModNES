
#import "AppDelegate.h"
#include <stdio.h>
#include <assert.h>
#include <OpenGL/gl.h>
#include <string.h>

@implementation AppDelegate

char *rompath = "/Volumes/Archive/Emulators/NES/NES 1089 ROMS/";

char *roms[] = {
   "10-Yard Fight (U) [!]", // 0
   "1942 (U) [!]", // 1
   "Balloon Fight (U) [!]", // 2
   "Baseball (U) [!]", // 3
   "Bomberman (U) [!]", // 4
   "Brush Roller (Unl)", // 5 // tries to write 2 to $4800 which is Expansion ROM?
   "BurgerTime (U) [!]", // 6
   "Chubby Cherub (U) [!p]", // 7
   "Clu Clu Land (W) [!]", // 8
   "Defender II (U) [!]", // 9
   "Demo Boy 2 (Unl)", // 10
   "Dig Dug II - Trouble in Paradise (U) [!]", // 11
   "Donkey Kong (U) (PRG1) [!p]", // 12
   "Donkey Kong 3 (U) [!]", // 13
   "Donkey Kong Jr. (U) (PRG1) [!]", // 14
   "Donkey Kong Jr. Math (U) [!]", // 15
   "Duck (Unl)", // 16
   "Duck Hunt (JU) [!]", // 17
   "Elevator Action (U) [!]", // 18
   "Excitebike (JU) [!]", // 19
   "Fire Dragon (Unl)", // 20
   "Galaga - Demons of Death (U) [!]", // 21
   "Golf (U) [!]", // 22
   "Gyromite (W) [!]", // 23
   "Hogan's Alley (W) [!]", // 24
   "Hydlide (U) [!]", // 25
   "Ice Climber (U) [!]", // 26
   "Ice Hockey (U) [!]", // 27
   "Joypad Test Cartridge (U)", // 28
   "Kung Fu (U) [!]", // 29
   "Little Red Hood (Sachen-HES) [!]", // 30
   "Lode Runner (U) [!]", // 31
   "Lunar Pool (U) [!]", // 32
   "M.U.S.C.L.E. (U) [!]", // 33
   "Mach Rider (U) [!]", // 34
   "Magic Jewelry (Unl) [!]", // 35
   "Magmax (U) [!]", // 36
   "Mario Bros. (U) [!]", // 37
   "Millipede (U) [!]", // 38
   "Ms. Pac-Man (U) [!]", // 39
   "NES PowerPad Test Cart (U) [!]", // 40
   "NES Test Cart (Official Nintendo) (U) [!]", // 41
   "Othello (U) [!]", // 42
   "Pac-Man (U) (Namco) [!p]", // 43
   "Pac-Man (Unl) [!]", // 44
   "Penguin Kun Wars (J) [T+Eng1.012_Penguin]", // 45
   "Pinball (JU) [!]", // 46
   "Popeye (W) (PRG1) [!]", // 47
   "Popeye no Eigo Asobi (J) [T+Eng1.0_KingMike]", // 48
   "Raid on Bungeling Bay (U) [!]", // 49
   "Seicross (U) [!]", // 50
   "Slalom (U) [!]", // 51
   "Soccer (JU) [!]", // 52
   "Spelunker (U) [!]", // 53
   "Spy Vs Spy (U) [!]", // 54
   "Sqoon (U) [!]", // 55
   "Stack Up (Robot Block) (W) [!]", // 56
   "Super Mario Bros. (JU) [!]", // 57
   "Tag Team Wrestling (U) [!]", // 58
   "Tennis (U) [!]", // 59
   "Urban Champion (UE) [!]", // 60
   "Volleyball (UE) [!]", // 61
   "Wild Gunman (W) (PRG1) [!]", // 62
   "Wrecking Crew (U) [!]", // 63
   "Xevious (U) [!]", // 64   
   "/Volumes/Archive/Source/NES ROMS/Zelda title screen demo/Zelda.nes" // 65
};

-(IBAction) onResetButton: (id)sender {
   Nes_Reset( self->nes );
}

- (void) applicationDidFinishLaunching:(NSNotification *)aNotification
{
//   NSLog( @"\nStarting ModNES\n" );
   self->nes = Nes_Create();
   assert( self->nes );
   
   _mainView->nes = self->nes;
   
   // Mario Bros. 37 - Super Mario Bros. 57 - Lode Runner 31 - Spy Vs Spy 54 - Donkey Kong 12
   int romn = 12
   ;
   
   char rom_filename[1024];
   if( roms[romn][0] == '/' ) {
      strcpy( rom_filename, roms[romn] );
   }
   else {
      strcpy( rom_filename, rompath );
      strcat( rom_filename, roms[romn] );
      strcat( rom_filename, ".nes" );
   }
   
   FILE *rom_file = fopen( rom_filename, "rb" );
   assert( rom_file );
   int loaded = Nes_LoadRom( self->nes, rom_file );
   assert( loaded );
   fclose( rom_file );
   
   _Nametables->vertical_mirroring = ( nes->ppu.mirroring == mirroring_vertical ? true : false );
   
   [self renderChrRoms];
   
   Nes_Reset( self->nes );
   
   float delta_t = 1.0f/60.0f;
   [NSTimer scheduledTimerWithTimeInterval: delta_t
      target: self selector: @selector( onNesFrame: )
      userInfo: nil repeats: YES ];
}

- (void) renderChrRoms
{
   ModNesOpenGLView *_CHR_ROM[2] = { _CHR_ROM_0, _CHR_ROM_1 };
   
   for( int chrom = 0; chrom <= 1; ++chrom )
   {
      byte *unpacked = self->nes->chr_unpacked_ptr[chrom];
      
      for( int tilen = 0; tilen <0x100; ++tilen )
      {
         for( int tiley = 0; tiley < 8; ++tiley )
         {
            for( int tilex = 0; tilex < 8; ++tilex )
            {
               byte color_index = *unpacked;
               
               byte rgb[3] = {0,0,0};
               switch( color_index ) {
                  case 1: rgb[0] = 0xFF; rgb[1] = 0xFF; rgb[2] = 0xFF; break;
                  case 2: rgb[0] = 0xFF; break;
                  case 3: rgb[2] = 0xFF; break;
               }
               
               int destx = ((tilen % 16) * 8) + tilex;
               int desty = ((tilen / 16) * 8) + tiley;
               
               char *px = &_CHR_ROM[chrom]->pixels[ ( desty * _CHR_ROM[chrom]->width + destx ) * 3 ];
               *px++ = rgb[0];
               *px++ = rgb[1];
               *px++ = rgb[2];
               
               unpacked++;
            }
         }
      }
   }

   [_CHR_ROM_0 setNeedsDisplay: YES];
   [_CHR_ROM_1 setNeedsDisplay: YES];
}

// #include <stdlib.h>
- (void) onNesFrame: (NSTimer*) timer {
   Nes_DoFrame( self->nes );
   
   // WIP Warning, this won't work with horizontal mirroring, nametable address mirroring is wrong
   
   // Nametables
   char *pixels = _Nametables->name0_pixels;
   byte *name_ptr = self->nes->ppu.name_ptr[0];
   byte *attr_ptr = self->nes->ppu.attr_ptr[0];
   
   byte *unpacked = self->nes->ppu.back_pattern == 0 ? self->nes->chr_unpacked_ptr[0] : self->nes->chr_unpacked_ptr[1];
   
   for( int i = 0; i <= 1; ++i )      
   {
      for( int celly = 0; celly < 30; ++celly )
      {
         for( int cellx = 0; cellx < 32; ++cellx )
         {
            byte tilen = *name_ptr;
            
            for( int tiley = 0; tiley < 8; ++tiley )
            {
               for( int tilex = 0; tilex < 8; ++tilex )
               {
                  int sourcex = tilex;
                  int sourcey = (tilen * 8) + tiley;
                  byte color_index = unpacked[ sourcey * 8 + sourcex ];
                  
                  int destx = cellx * 8 + tilex;
                  int desty = celly * 8 + tiley;
                  
                  int attrx = destx / 32; // translate screen x,y into attribute xtile, ytile
                  int attry = desty / 32;
                  byte attribute = attr_ptr[ attry * 8 + attrx ];
                  
                  byte palette_index;
                  if( celly % 4 < 2 ) {
                     if( cellx % 4 < 2 ) {
                        palette_index = attribute & 0x03; // bits 0,1
                     }
                     else {
                        palette_index = ( attribute & 0x0C ) >>2;  // bits 2,3
                     }
                  }
                  else {
                     if( cellx % 4 < 2 ) {
                        palette_index = ( attribute & 0x30 ) >>4; // bits 4,5
                     }
                     else {
                        palette_index = ( attribute & 0xC0 ) >>6; // bits 6,7
                     }
                  }

                  const byte *rgb = Nes_GetPaletteColor(self->nes, 0, palette_index, color_index );

                  pixels[( desty * _Nametables->width + destx ) * 3 + 0] = rgb[0];
                  pixels[( desty * _Nametables->width + destx ) * 3 + 1] = rgb[1];
                  pixels[( desty * _Nametables->width + destx ) * 3 + 2] = rgb[2];
               }
            }
            ++name_ptr;
         }
      }
      pixels = _Nametables->name1_pixels;
      name_ptr = self->nes->ppu.name_ptr[1];
      attr_ptr = self->nes->ppu.attr_ptr[1];
   }
   [self renderSprites];
   _Screen->pixels = _Nametables->name0_pixels;
   [_Screen setNeedsDisplay: YES];
   [_Nametables setNeedsDisplay: YES];
   
   // Palettes
   byte *rgb_index = &self->nes->ppu.palettes[0];
   _Palettes->pixels[0] = _Palettes->pixels[3] = _Palettes->pixels[6] = Nes_rgb[ *rgb_index ][0];
   _Palettes->pixels[1] = _Palettes->pixels[4] = _Palettes->pixels[7] = Nes_rgb[ *rgb_index ][1];
   _Palettes->pixels[2] = _Palettes->pixels[5] = _Palettes->pixels[8] = Nes_rgb[ *rgb_index ][2];
   
   for( int paln = 0; paln < 8; ++paln ) {
      for( int coln = 1; coln <= 3; ++coln ) {
         rgb_index = &self->nes->ppu.palettes[ paln * 4 + coln ];
         _Palettes->pixels[ ( paln + 1 ) * 9 + ( coln - 1 ) * 3 + 0 ] = Nes_rgb[ *rgb_index ][0];
         _Palettes->pixels[ ( paln + 1 ) * 9 + ( coln - 1 ) * 3 + 1 ] = Nes_rgb[ *rgb_index ][1];
         _Palettes->pixels[ ( paln + 1 ) * 9 + ( coln - 1 ) * 3 + 2 ] = Nes_rgb[ *rgb_index ][2];
      }
   }
   [_Palettes setNeedsDisplay: YES];
}

- (void) renderSprites
{
   memset( (void *)_Sprites->pixels, 237, _Sprites->width * _Sprites->height * 3 );
   
   char *namepixels = _Nametables->name0_pixels;
   byte *unpacked   = self->nes->ppu.sprite_pattern == 0 ? self->nes->chr_unpacked_ptr[0] : self->nes->chr_unpacked_ptr[1];
   
   for( byte *sprite = self->nes->ppu.sprites; sprite < &self->nes->ppu.sprites[0x100]; sprite += 4 )
   {
      int posy = sprite[0];
      int posx = sprite[3];
      
      int xflip = sprite[2] & ( 1<<6 );
      int yflip = sprite[2] & ( 1<<7 );
      int xstart, xstop, xstep, ystart, ystop, ystep;

      if( xflip ) {
         xstart = 7;
         xstop  = -1;
         xstep  = -1;
      }
      else {
         xstart = 0;
         xstop  = 8;
         xstep  = 1;
      }
      
      if( yflip ) {
         ystart = 7;
         ystop  = -1;
         ystep  = -1;
      }
      else {
         ystart = 0;
         ystop  = 8;
         ystep  = 1;
      }
      
      int tilen = sprite[1];
      int paln  = sprite[2] & 3; // 2 lsb
      
      for( int spritex = xstart; spritex != xstop; spritex += xstep )
      {
         for( int spritey = ystart; spritey != ystop; spritey += ystep )
         {
            byte pixel = unpacked[ tilen * 64 + spritey * 8 + spritex ];
            if( pixel > 0 )
            {
               const byte *rgb = Nes_GetPaletteColor( self->nes, 1, paln, pixel );
               
               int x = (( tilen % 16 ) * 8 ) + spritex;
               int y = (( tilen / 16 ) * 8 ) + spritey;
               char *pz = &_Sprites->pixels[ ( y * _Sprites->width + x ) * 3 ];
               *pz++ = rgb[0];
               *pz++ = rgb[1];
               *pz++ = rgb[2];
                  
               if( posy + spritey >= 240 || posx + spritex >= 256 ) {
                  continue;
               }
               
               int unflipx = ( xflip ? 7 - spritex : spritex );
               int unflipy = ( yflip ? 7 - spritey : spritey );
               
               char *px = &namepixels[ ( ( posy + unflipy ) * _Nametables->width + ( posx + unflipx ) ) * 3 ];
               *px++ = rgb[0];
               *px++ = rgb[1];
               *px++ = rgb[2];
            }
         }
      }
   }
   [_Sprites setNeedsDisplay: YES];
}

- (void) applicationWillTerminate: (NSNotification *)aNotification
{
   [_CHR_ROM_0 free];
   [_CHR_ROM_1 free];
   [_Nametables free];
   [_Palettes free];
   [_Sprites free];
   // [_Screen free];   
   Nes_Free( self->nes );
}

@end