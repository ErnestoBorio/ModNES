
#include "ModNES.h"
#include "Nes.h"
#include <stdio.h>
#include <fstream>
using namespace std;

const string ModNES::config_filename = "ModNES_config";
extern const byte Nes_rgb[64][3];
extern "C" char* openFile( char* path, int length );

//------------------------------------------------------------------------------------------------------------
Uint32 vblank_callback( Uint32 interval, void* param )
{
    // Push a user event to avoid multithreading problems
    SDL_Event event;
    SDL_UserEvent userevent;

    userevent.type = SDL_USEREVENT;
    userevent.code = 0;
    userevent.data1 = NULL;
    userevent.data2 = NULL;

    event.type = SDL_USEREVENT;
    event.user = userevent;

    SDL_PushEvent(&event);
    return(interval);
}
//------------------------------------------------------------------------------------------------------------
ModNES::ModNES()
{
    this->running = false;
}
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
        
        this->config.patterns_win.size = 0;
        this->config.patterns_win.pos.x = 400;
        this->config.patterns_win.pos.y = 200;
        
        this->config.nametables_win.size = 1;
        this->config.nametables_win.pos.x = 400;
        this->config.nametables_win.pos.y = 20;
        
        this->config.screen_win.size = 1;
        this->config.screen_win.pos.x = 20;
        this->config.screen_win.pos.y = 20;
        
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
    this->patterns_win = SDL_CreateWindow( "Pattern tables", 
        config.patterns_win.pos.x, config.patterns_win.pos.y, 
        config.patterns_win.size <= 1 ? 256 : 512, 
        config.patterns_win.size <= 1 ? 128 : 256, 
        config.patterns_win.size == 0 ? SDL_WINDOW_HIDDEN : SDL_WINDOW_SHOWN );
    this->patterns_win_id = SDL_GetWindowID( this->patterns_win );

    this->nametables_win = SDL_CreateWindow( "Name tables", 
        config.nametables_win.pos.x, config.nametables_win.pos.y, 
        256*2, 240, 
        config.nametables_win.size == 0 ? SDL_WINDOW_HIDDEN : SDL_WINDOW_SHOWN );
    this->nametables_win_id = SDL_GetWindowID( this->nametables_win );
    
    this->screen_win = SDL_CreateWindow( "ModNES", 
        config.screen_win.pos.x, config.screen_win.pos.y,
        256*2, 240*2, 
        config.screen_win.size == 0 ? SDL_WINDOW_HIDDEN : SDL_WINDOW_SHOWN );
    this->screen_win_id = SDL_GetWindowID( this->screen_win );
    
    // SDL_Surface *windSurf = SDL_GetWindowSurface( this->patterns_win );
    // this->patterns_ren = SDL_CreateRenderer( this->patterns_win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
    // La papota parece ser que o creas un renderer (3D, texturas, GPU) del window, o geteas un winSurface, pero no ambos.
    // Probemos primero con get win surface, onda más software mente, después probemos con el renderer y textures.
    
    this->patterns_surf = SDL_CreateRGBSurface( 0, 256, 128, 8, 0, 0, 0, 0 );
    this->temp_pal  = SDL_AllocPalette( 0x100 );
    this->patterns_pal  = SDL_AllocPalette( 0x100 );
        // Even when using only 4 indices, all 256 colors have to be allocated or it won't work.
    
    this->nametables_surf = SDL_CreateRGBSurface( 0, 256*2, 240, 32, 0, 0, 0, 0 );

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
    SDL_RemoveTimer( this->timer_id );
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
                else if( event.key.keysym.sym == SDLK_r ) {
                    Nes_Reset( this->nes );
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
                        this->presentPatterns();
                        config.patterns_win.size = 1;
                    }
                    else if( config.patterns_win.size == 1 ) {
                        rect.w = 512;
                        rect.h = 256;
                        SDL_SetWindowSize( this->patterns_win, 512, 256 );
                        // this->renderPatterns();
                        this->presentPatterns();
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
                    else if( event.window.windowID == this->screen_win_id ) {
                        this->config.screen_win.pos.x = event.window.data1;
                        this->config.screen_win.pos.y = event.window.data2;
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
            //------------------------------------------------------------------------------------------------
            // VBlank
            case SDL_USEREVENT:
                if( this->running ) {
                    Nes_DoFrame( this->nes );
                    
                    SDL_SetColorKey( this->patterns_surf, SDL_FALSE, 0 );
                    this->renderNametables();
                    
                    SDL_SetColorKey( this->patterns_surf, SDL_TRUE, 0 );
                    this->renderSprites();
                    
                    SDL_BlitSurface( this->nametables_surf, NULL, SDL_GetWindowSurface( this->nametables_win ), NULL );
                    
                    SDL_Rect viewport = { nes->ppu.horz_scroll, 0, 256, 240 };
                    SDL_BlitScaled( this->nametables_surf, &viewport, SDL_GetWindowSurface( this->screen_win ), NULL );
                    
                    // Draw viewport rectangle WIP get this out of here!
                    SDL_Rect bounds[2] = {
                        { nes->ppu.horz_scroll, 0, 1, 240 },
                        { nes->ppu.horz_scroll + 256, 0, 1, 240 } };
                    SDL_Surface *windSurf = SDL_GetWindowSurface( this->nametables_win );
                    SDL_FillRects( windSurf, bounds, 2, SDL_MapRGB( windSurf->format, 0, 255, 0 ) );
                    
                    SDL_UpdateWindowSurface( this->nametables_win );
                    SDL_UpdateWindowSurface( this->screen_win );
                }
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
    this->presentPatterns(); //WIP move this outta here
    
    if( ! this->running ) {
        this->running = true;
        this->timer_id = SDL_AddTimer( 1000 / 60, vblank_callback, NULL );
    }
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
void ModNES::presentPatterns()
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
//------------------------------------------------------------------------------------------------------------
void ModNES::renderNametables()
{
    SDL_Rect patt, name;
    patt.w = patt.h = name.w = name.h = 8;
    SDL_Color colors[4];
    SDL_SetSurfacePalette( this->patterns_surf, this->temp_pal );
    
    for( int table = 0; table <= 1; ++table )
    {
        byte *name_ptr = this->nes->ppu.name_ptr[table];
        byte *attr_ptr = this->nes->ppu.attr_ptr[table];
        
        for( int tiley = 0; tiley < 30; ++tiley )
        {
            for( int tilex = 0; tilex < 32; ++tilex )
            {
                byte tilen = *name_ptr;
                
                patt.x = 128 + (tilen % 16) * 8;
                patt.y = (tilen / 16) * 8;
                
                name.x = (table * 256) + (tilex * 8);
                name.y = tiley * 8;
                
                byte attribute = attr_ptr[ (tilex/4) + (tiley/4) * 8 ];
                
                byte palette_index;
                if( tiley % 4 < 2 ) {
                    if( tilex % 4 < 2 ) {
                        palette_index = attribute & 0x03; // bits 0,1
                    }
                    else {
                        palette_index = ( attribute & 0x0C ) >>2;  // bits 2,3
                    }
                }
                else {
                    if( tilex % 4 < 2 ) {
                        palette_index = ( attribute & 0x30 ) >>4; // bits 4,5
                    }
                    else {
                        palette_index = ( attribute & 0xC0 ) >>6; // bits 6,7
                    }
                }
                
                
                // WIP IIRC back and sprite pallettes can be switched? or not?
                for( int i = 0; i <= 3; ++i )
                {
                    int rgb_index = nes->ppu.palettes[ palette_index * 4 + i ]; // should +0x10 if it were sprite palettes
                    colors[i].r = Nes_rgb[rgb_index][0];
                    colors[i].g = Nes_rgb[rgb_index][1];
                    colors[i].b = Nes_rgb[rgb_index][2];
                }
                
                SDL_SetPaletteColors( this->temp_pal, colors, 0, 4 );
                // SDL_SetSurfacePalette( this->patterns_surf, this->temp_pal );
                SDL_BlitSurface( this->patterns_surf, &patt, this->nametables_surf, &name );

                ++name_ptr;
            }
        }
    }
}
//------------------------------------------------------------------------------------------------------------
void ModNES::renderSprites()
{
    SDL_Rect patt, name;
    patt.w = patt.h = name.w = name.h = 8;
    
    SDL_Color colors[4] = {{0,0,0}};
    
    SDL_SetSurfacePalette( this->patterns_surf, this->temp_pal );
    
    for( byte *sprite = nes->ppu.sprites; sprite < &nes->ppu.sprites[0x100]; sprite += 4 )
    {
        name.y = sprite[0];
        name.x = nes->ppu.horz_scroll + sprite[3];
        int tilen = sprite[1];
        int paln  = sprite[2] & 3; // 2 lsb
        int xflip = sprite[2] & ( 1<<6 );
        int yflip = sprite[2] & ( 1<<7 );
        
        // WIP this is assuming sprites are in pattern table 0
        patt.x = (tilen % 16) * 8;
        patt.y = (tilen / 16) * 8;
        
        // WIP IIRC back and sprite pallettes can be switched? or not?
        for( int i = 0; i <= 3; ++i )
        {
            int rgb_index = nes->ppu.palettes[ 0x10 + paln * 4 + i ];
            colors[i].r = Nes_rgb[rgb_index][0];
            colors[i].g = Nes_rgb[rgb_index][1];
            colors[i].b = Nes_rgb[rgb_index][2];
        }
        
        SDL_SetPaletteColors( this->temp_pal, colors, 0, 4 );
        SDL_BlitSurface( this->patterns_surf, &patt, this->nametables_surf, &name );
    }
}
//------------------------------------------------------------------------------------------------------------