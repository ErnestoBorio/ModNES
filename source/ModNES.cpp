
#include "ModNES.h"
#include "Nes.h"
#include <stdio.h>
#include <fstream>
using namespace std;

const string ModNES::config_filename = "ModNES_config";

// Esto debería estar en Nes.c pero por alguna razón se trulalea
const byte Nes_rgb[64][3] =
{ //      0                 1                 2                 3
   {0x80,0x80,0x80}, {0x00,0x00,0xBB}, {0x37,0x00,0xBF}, {0x84,0x00,0xA6}, // 00
   {0xBB,0x00,0x6A}, {0xB7,0x00,0x1E}, {0xB3,0x00,0x00}, {0x91,0x26,0x00}, // 04
   {0x7B,0x2B,0x00}, {0x00,0x3E,0x00}, {0x00,0x48,0x0D}, {0x00,0x3C,0x22}, // 08
   {0x00,0x2F,0x66}, {0x00,0x00,0x00}, {0x05,0x05,0x05}, {0x05,0x05,0x05}, // 0C

   {0xC8,0xC8,0xC8}, {0x00,0x59,0xFF}, {0x44,0x3C,0xFF}, {0xB7,0x33,0xCC}, // 10
   {0xFF,0x33,0xAA}, {0xFF,0x37,0x5E}, {0xFF,0x37,0x1A}, {0xD5,0x4B,0x00}, // 14
   {0xC4,0x62,0x00}, {0x3C,0x7B,0x00}, {0x1E,0x84,0x15}, {0x00,0x95,0x66}, // 18
   {0x00,0x84,0xC4}, {0x11,0x11,0x11}, {0x09,0x09,0x09}, {0x09,0x09,0x09}, // 1C

   {0xFF,0xFF,0xFF}, {0x00,0x95,0xFF}, {0x6F,0x84,0xFF}, {0xD5,0x6F,0xFF}, // 20
   {0xFF,0x77,0xCC}, {0xFF,0x6F,0x99}, {0xFF,0x7B,0x59}, {0xFF,0xB6,0x00}, // 24
   {0xFF,0xA2,0x33}, {0xA6,0xBF,0x00}, {0x51,0xD9,0x6A}, {0x4D,0xD5,0xAE}, // 28
   {0x00,0xD9,0xFF}, {0x66,0x66,0x66}, {0x0D,0x0D,0x0D}, {0x0D,0x0D,0x0D}, // 2C

   {0xFF,0xFF,0xFF}, {0x84,0xBF,0xFF}, {0xBB,0xBB,0xFF}, {0xD0,0xBB,0xFF}, // 30
   {0xFF,0xBF,0xEA}, {0xFF,0xBF,0xCC}, {0xFF,0xC4,0xB7}, {0xFF,0xCC,0xAE}, // 34
   {0xFF,0xD9,0xA2}, {0xCC,0xE1,0x99}, {0xAE,0xEE,0xB7}, {0xAA,0xF7,0xEE}, // 38
   {0xB3,0xEE,0xFF}, {0xDD,0xDD,0xDD}, {0x11,0x11,0x11}, {0x11,0x11,0x11}  // 3C
};

extern "C" char* openFile( char* path, int length );

//------------------------------------------------------------------------------------------------------------
int ModNES::run()
{
    this->init();
    // this->renderPatterns();
    this->loop();
    return APP_OK;
}
//------------------------------------------------------------------------------------------------------------
void ModNES::read_config()
{
    fstream config_file( ModNES::config_filename, fstream::binary | fstream::in );
    if( ! config_file ) {
        config_file.open( ModNES::config_filename, fstream::trunc | fstream::binary | fstream::out );
        
        this->config.patterns_win.size = 1;
        this->config.patterns_win.pos.x = 400;
        this->config.patterns_win.pos.y = 20;
        this->config.nametables_win.size = 1;
        this->config.nametables_win.pos.x = 20;
        this->config.nametables_win.pos.y = 20;
        
        config_file.write( (const char*)&this->config, sizeof( this->config ));
    }
    else {
        config_file.read( (char*)&this->config, sizeof( this->config ));
    }
    config_file.close();
}
//------------------------------------------------------------------------------------------------------------
void ModNES::write_config()
{
    fstream config_file( ModNES::config_filename, fstream::trunc | fstream::binary | fstream::out );
    config_file.write( (const char*)&this->config, sizeof( this->config ));
    config_file.close();
}
//------------------------------------------------------------------------------------------------------------
int ModNES::init()
{
    this->read_config();
    
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        fprintf(stderr, "SDL_Init() failed: %s\n", SDL_GetError());
        return APP_FAILED;
    }
    this->patterns_win = SDL_CreateWindow( "Pattern tables", config.patterns_win.pos.x, config.patterns_win.pos.y, 
        config.patterns_win.size <= 1 ? 256 : 512, 
        config.patterns_win.size <= 1 ? 128 : 256, 
        config.patterns_win.size == 0 ? SDL_WINDOW_HIDDEN : SDL_WINDOW_SHOWN );
    this->patterns_win_id = SDL_GetWindowID( this->patterns_win );

    this->nametables_win = SDL_CreateWindow( "Name tables", config.nametables_win.pos.x, config.nametables_win.pos.y, 256*2, 240*2, 
        config.nametables_win.size == 0 ? SDL_WINDOW_HIDDEN : SDL_WINDOW_SHOWN );
    this->nametables_win_id = SDL_GetWindowID( this->nametables_win );
    
    // SDL_Surface *windSurf = SDL_GetWindowSurface( this->patterns_win );
    // this->patterns_ren = SDL_CreateRenderer( this->patterns_win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
    // La papota parece ser que o creas un renderer (3D, texturas, GPU) del window, o geteas un winSurface, pero no ambos.
    // Probemos primero con get win surface, onda más software mente, después probemos con el renderer y textures.
    
    this->patterns_surf = SDL_CreateRGBSurface( 0, 256, 128, 8, 0, 0, 0, 0 );
    this->temp_pal  = SDL_AllocPalette( 0x100 );
    this->patterns_pal  = SDL_AllocPalette( 0x100 );
        // Even when using only 4 indices, all 256 colors have to be allocated or it won't work.
    for( int i=0; i<=1; i++ ) {
        this->nametables_surf[i] = SDL_CreateRGBSurface( 0, 256, 128, 32, 0, 0, 0, 0 );
    }

    this->nes = Nes_Create();
    return APP_OK;
}
//------------------------------------------------------------------------------------------------------------
ModNES::~ModNES()
{   
    if( this->patterns_win ) {
        SDL_DestroyWindow( this->patterns_win );
    }
    // if( this->patterns_ren ) {
    //     SDL_DestroyRenderer( this->patterns_ren );
    // }
    if( this->patterns_surf ) {
        SDL_FreeSurface( this->patterns_surf );
    }
    if( this->patterns_pal ) {
        SDL_FreePalette( this->patterns_pal );
    }
    SDL_Quit();    
}  
//------------------------------------------------------------------------------------------------------------
void ModNES::loop()
{
    SDL_Event event;
    this->quit = false;
    
    static char path[1024];
   
    while( ! this->quit && SDL_WaitEvent(&event))
    {
        switch( event.type )
        {
            //------------------------------------------------------------------------------------------------
            case SDL_KEYDOWN:
                if( event.key.keysym.sym == SDLK_ESCAPE ) {
                    this->quit = true;
                }
                else if( event.key.keysym.sym == SDLK_o ) {
                    openFile( path, 1024 );
                    if( path[0] ) {
                        this->loadCartridge( path );
                    }
                }
                //------------------------------------------------------------------------------------------------
                else if( event.key.keysym.sym == SDLK_p ) // resize Pattern tables window
                {
                    SDL_Rect rect = {0,0,0,0};
                    
                    if( config.patterns_win.size == 0 ) {
                        SDL_ShowWindow( this->patterns_win );
                        SDL_SetWindowSize( this->patterns_win, 256, 128 );
                        rect.w = 256;
                        rect.h = 128;
                        // this->renderPatterns();
                        this->presentPatterns( &rect );
                        config.patterns_win.size = 1;
                    }
                    else if( config.patterns_win.size == 1 ) {
                        rect.w = 512;
                        rect.h = 256;
                        SDL_SetWindowSize( this->patterns_win, 512, 256 );
                        // this->renderPatterns();
                        this->presentPatterns( &rect );
                        config.patterns_win.size = 2;
                    }
                    else if( config.patterns_win.size == 2 ) {
                        SDL_HideWindow( this->patterns_win );
                        config.patterns_win.size = 0;
                    }
                    this->write_config();
                }
                //------------------------------------------------------------------------------------------------
                else if( event.key.keysym.sym == SDLK_n ) // resize Nametable window
                {
                    if( config.nametables_win.size == 0 ) {
                        SDL_ShowWindow( this->nametables_win );
                        config.nametables_win.size = 1;
                    }
                    else if( config.nametables_win.size == 1 ) {
                        SDL_HideWindow( this->nametables_win );
                        config.nametables_win.size = 0;
                    }
                    this->write_config();
                }
                break;
            //------------------------------------------------------------------------------------------------
            case SDL_WINDOWEVENT:
                if( event.window.event == SDL_WINDOWEVENT_MOVED )
                {
                    if( event.window.windowID == this->patterns_win_id ) {
                        this->config.patterns_win.pos.x = event.window.data1;
                        this->config.patterns_win.pos.y = event.window.data2;
                    }
                    else if( event.window.windowID == this->nametables_win_id ) {
                        this->config.nametables_win.pos.x = event.window.data1;
                        this->config.nametables_win.pos.y = event.window.data2;
                    }
                    this->write_config();
                }
                break;
            //------------------------------------------------------------------------------------------------
            case SDL_DROPFILE:
                this->loadCartridge( event.drop.file );
                SDL_free( event.drop.file );
                break;
            //------------------------------------------------------------------------------------------------
            case SDL_QUIT:
                this->quit = true;
                break;
        }
    }
}
 //------------------------------------------------------------------------------------------------------------
void ModNES::loadCartridge( char *path )
{
    FILE *romFile = fopen( path, "rb" );
    if( ! romFile ) {
        SDL_Log( "Rom file couldn't be opened: %s\n", path );
    }
    if( ! Nes_LoadRom( this->nes, romFile )) {
        SDL_Log( "Rom image couldn't be loaded: %s\n", path );
    }
    fclose( romFile );
    Nes_Reset( this->nes );
    this->renderPatterns();
    SDL_Rect rect = {0, 0, 256, 128};
    this->presentPatterns( &rect ); //WIP move this outta here
}
//------------------------------------------------------------------------------------------------------------
void ModNES::renderPatterns()
{
    SDL_Color colors[] = {
        { 0, 0, 0 },
        { 0xFF, 0xFF, 0 },
        { 0, 0xFF, 0xFF },
        { 0xFF, 0, 0xFF }
    };
    SDL_SetPaletteColors( this->patterns_pal, colors, 0, 4 );
    SDL_SetSurfacePalette( this->patterns_surf, this->patterns_pal );
   
    SDL_LockSurface( this->patterns_surf );
    SDL_Surface *surf = this->patterns_surf;
    byte *pixels = (byte *) surf->pixels;
    
    for( int chrom = 0; chrom <= 1; ++chrom )
    {
        byte *unpacked = this->nes->chr_unpacked_ptr[ chrom ];
        for( int tilen = 0; tilen < 0x100; ++tilen )
        {
            for( int line = 0; line <= 7; ++line )
            {
                byte *target;
                
                int tilex = tilen % 16;
                int tiley = tilen / 16;
                int x = tilex * 8 + chrom * 128;
                int y = tiley * 8 + line;
                target = pixels + surf->pitch * y + x;
                
                memcpy( target, unpacked, 8 );

                unpacked += 8;
            }
        }
    }
    SDL_UnlockSurface( this->patterns_surf );
}
//------------------------------------------------------------------------------------------------------------
void ModNES::presentPatterns( SDL_Rect* rect )
{
    SDL_Surface *buffer = SDL_CreateRGBSurface( 0, this->patterns_surf->w, this->patterns_surf->h, 32, 0, 0, 0, 0 );
    SDL_BlitSurface( this->patterns_surf, NULL, buffer, NULL );
    SDL_Surface *windSurf = SDL_GetWindowSurface( this->patterns_win );
    
    SDL_Rect r = {0,0,0,0};
    r.w = windSurf->w;
    r.h = windSurf->h;
    SDL_BlitScaled( buffer, NULL, windSurf, &r );
    
    SDL_UpdateWindowSurface( this->patterns_win );
    SDL_FreeSurface( buffer );
}