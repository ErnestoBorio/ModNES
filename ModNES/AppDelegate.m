
#import "AppDelegate.h"
#include <stdio.h>
#include <assert.h>
#include <OpenGL/gl.h>
#include <string.h>

@implementation AppDelegate

char *romfiles[] = {
   "/Volumes/Archive/Emulators/NES/NES 1089 ROMS/Mario Bros. (U) [!].nes",        //0
   "/Volumes/Archive/Emulators/NES/NES 1089 ROMS/Super Mario Bros. (JU) [!].nes", //1
   "/Volumes/Archive/Emulators/NES/NES 1089 ROMS/Popeye (W) (PRG1) [!].nes",      //2
   "/Volumes/Archive/Emulators/NES/NES 1089 ROMS/Spy Vs Spy (U) [!].nes",         //3
   "/Volumes/Archive/Emulators/NES/NES 1089 ROMS/Ice Climber (U) [!].nes",        //4
   "/Volumes/Archive/Emulators/NES/NES 1089 ROMS/Balloon Fight (U) [!].nes",      //5
   "/Volumes/Archive/Emulators/NES/NES 1089 ROMS/1942 (U) [!].nes",               //6
   "/Volumes/Archive/Emulators/NES/NES 1089 ROMS/Lode Runner (U) [!].nes",        //7
   "/Volumes/Archive/Emulators/NES/NES 1089 ROMS/Pac-Man (U) (Namco) [!p].nes",   //8
   "/Volumes/Archive/Emulators/NES/NES 1089 ROMS/NES Test Cart (Official Nintendo) (U) [!].nes", //9
   "/Volumes/Archive/Source/NES ROMS/Zelda title screen demo/Zelda.nes", //10   
   "/Volumes/Archive/Emulators/NES/NES 1089 ROMS/",
   "/Volumes/Archive/Emulators/NES/NES 1089 ROMS/",
   "/Volumes/Archive/Emulators/NES/NES 1089 ROMS/",
   "/Volumes/Archive/Emulators/NES/NES 1089 ROMS/",
   "/Volumes/Archive/Emulators/NES/NES 1089 ROMS/",
   "/Volumes/Archive/Emulators/NES/NES 1089 ROMS/",
   "/Volumes/Archive/Emulators/NES/NES 1089 ROMS/",
   "/Volumes/Archive/Emulators/NES/NES 1089 ROMS/",
   "/Volumes/Archive/Emulators/NES/NES 1089 ROMS/",
   "/Volumes/Archive/Emulators/NES/NES 1089 ROMS/",
   "/Volumes/Archive/Emulators/NES/NES 1089 ROMS/",
   "/Volumes/Archive/Emulators/NES/NES 1089 ROMS/"};

-(IBAction) onResetButton: (id)sender {
   Nes_Reset( self->nes );
}

- (void) applicationDidFinishLaunching:(NSNotification *)aNotification
{
//   NSLog( @"\nStarting ModNES\n" );
   self->nes = Nes_Create();
   assert( self->nes );
   FILE *rom_file = fopen( romfiles[10], "rb" );
   int loaded = Nes_LoadRom( self->nes, rom_file );
   assert( loaded );
   fclose( rom_file );
   
   _Nametables->vertical_mirroring = ( nes->ppu.mirroring == mirroring_vertical ? true : false );
   
   // Render CHR_ROMs
   
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
               
               byte r=0,g=0,b=0;
               switch( color_index ) {
                  case 2: r = 255; g = 148; b = 41; break;
                  case 1: r = 0xFF; break;
                  case 3: r = 136; g = 115; break;
               }
               
               int destx = ((tilen % 16) * 8) + tilex;
               int desty = ((tilen / 16) * 8) + tiley;
               
               _CHR_ROM[chrom]->pixels[ ( desty * _CHR_ROM[chrom]->width + destx ) * 3 ]     = r;
               _CHR_ROM[chrom]->pixels[ ( desty * _CHR_ROM[chrom]->width + destx ) * 3 + 1 ] = g;
               _CHR_ROM[chrom]->pixels[ ( desty * _CHR_ROM[chrom]->width + destx ) * 3 + 2 ] = b;
               
               unpacked++;
            }
         }
      }
   }

   [_CHR_ROM_0 setNeedsDisplay: YES];
   [_CHR_ROM_1 setNeedsDisplay: YES];
   
   Nes_Reset( self->nes );
   
   [NSTimer scheduledTimerWithTimeInterval: 1/60
      target: self selector:@selector( onNesFrame: )
      userInfo: nil repeats: YES ];
}
// #include <stdlib.h>
- (void) onNesFrame: (NSTimer*) timer {
   Nes_DoFrame( self->nes );
   
   // WIP Warning, this won't work with horizontal mirroring, nametable address mirroring is wrong
   
   char *pixels = _Nametables->name0_pixels;
   byte *name_ptr = self->nes->ppu.name_ptr[0];
   byte *attr_ptr = self->nes->ppu.attr_ptr[0];
   for( int i = 0; i <= 1; ++i )      
   {
      byte *unpacked = self->nes->chr_unpacked_ptr[1];
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
                  
                  int attrx = destx / 16; // translate screen x,y into attribute xtile, ytile
                  int attry = desty / 16;
                  byte attribute = attr_ptr[ attry * 16 + attrx ];
                  
                  byte palette_index;
                  if( desty % 16 < 8 ) {
                     if( destx % 16 < 8 ) {
                        palette_index = attribute & 0x03; // bits 0,1
                     }
                     else {
                        palette_index = attribute & 0xC;  // bits 2,3
                     }
                  }
                  else {
                     if( destx % 16 < 8 ) {
                        palette_index = attribute & 0x30; // bits 4,5
                     }
                     else {
                        palette_index = attribute & 0xC0; // bits 6,7
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
   [_Nametables setNeedsDisplay: YES];
   
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

- (void) applicationWillTerminate: (NSNotification *)aNotification
{
   [self->_CHR_ROM_0 free];
   [self->_CHR_ROM_1 free];
   [_Nametables free];
   Nes_Free( self->nes );
}

@end