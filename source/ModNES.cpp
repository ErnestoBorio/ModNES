
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
        
        memset( this->config.romFileName, 0, 1024 );
        
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
    this->timer_id = 0;
    this->render_sprites = true;
    this->hide_top_bottom = false;
    
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        fprintf(stderr, "SDL_Init() failed: %s\n", SDL_GetError());
        return APP_FAILED;
    }
    
    this->patterns_win = SDL_CreateWindow( "Pattern tables", 
        config.patterns_win.pos.x, config.patterns_win.pos.y, 
        config.patterns_win.size <= 1 ? 257 : 514, 
        config.patterns_win.size <= 1 ? 128 : 256, 
        config.patterns_win.size == 0 ? SDL_WINDOW_HIDDEN : SDL_WINDOW_SHOWN );
    this->patterns_win_id = SDL_GetWindowID( this->patterns_win );

    this->nametables_win = SDL_CreateWindow( "Name tables", 
        config.nametables_win.pos.x, config.nametables_win.pos.y, 
        256*2, 240*2, 
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
    
    // Offscreen buffer for the NES screen. Then BlitScaled 2x from this to screen_win
    this->screen_surf   = SDL_CreateRGBSurface( 0, 256, 240, 32, 0, 0, 0, 0 );
    
    this->patterns_surf = SDL_CreateRGBSurface( 0, 257, 128,  8, 0, 0, 0, 0 );
    this->temp_pal  = SDL_AllocPalette( 0x100 );
    this->patterns_pal  = SDL_AllocPalette( 0x100 );
        // Even when using only 4 indices, all 256 colors have to be allocated or it won't work.
    
    this->nametables_surf = SDL_CreateRGBSurface( 0, 256*2, 240*2, 32, 0, 0, 0, 0 );

    this->nes = Nes_Create();
    
    if( *config.romFileName != '\0' ) {
        loadCartridge( config.romFileName );
    }
    
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
            {
                switch( event.key.keysym.sym )
                {
                    case SDLK_ESCAPE:
                        this->quit = true;
                        break;
                    
                    case SDLK_SPACE:
                        this->running = ! this->running;
                        // printf( running? "PLAY\n": "PAUSE\n" );
                        break;
                        
                    case SDLK_o:
                        openFile( path, 1024 );
                        if( path[0] ) {
                            this->loadCartridge( path );
                        }
                        break;

                    case SDLK_r:
                        Nes_Reset( this->nes );
                        this->running = true;
                        break;
                    
                    case SDLK_s:
                        this->render_sprites = ! this->render_sprites;
                        break;
                    
                    case SDLK_h:
                        this->hide_top_bottom = ! this->hide_top_bottom;
                        break;
                    //------------------------------------------------------------------------------------------------
                    // Basic input WIP
                    case SDLK_x:     Nes_SetInputState( this->nes, 0, Nes_A,      1 ); break;
                    case SDLK_z:     Nes_SetInputState( this->nes, 0, Nes_B,      1 ); break;
                    case SDLK_c:     Nes_SetInputState( this->nes, 0, Nes_Select, 1 ); break;
                    case SDLK_v:     Nes_SetInputState( this->nes, 0, Nes_Start,  1 ); break;
                    case SDLK_UP:    Nes_SetInputState( this->nes, 0, Nes_Up,     1 ); break;
                    case SDLK_DOWN:  Nes_SetInputState( this->nes, 0, Nes_Down,   1 ); break;
                    case SDLK_LEFT:  Nes_SetInputState( this->nes, 0, Nes_Left,   1 ); break;
                    case SDLK_RIGHT: Nes_SetInputState( this->nes, 0, Nes_Right,  1 ); break;
                    
                    //------------------------------------------------------------------------------------------------
                    // resize Pattern tables window
                    case SDLK_p:
                        if( config.patterns_win.size == 0 ) {
                            SDL_ShowWindow( this->patterns_win );
                            SDL_SetWindowSize( this->patterns_win, 257, 128 );
                            this->presentPatterns();
                            config.patterns_win.size = 1;
                        }
                        else if( config.patterns_win.size == 1 ) {
                            SDL_SetWindowSize( this->patterns_win, 514, 256 );
                            this->presentPatterns();
                            config.patterns_win.size = 2;
                        }
                        else if( config.patterns_win.size == 2 ) {
                            SDL_HideWindow( this->patterns_win );
                            config.patterns_win.size = 0;
                        }
                        this->write_config();
                        break;
                    //------------------------------------------------------------------------------------------------
                    // resize Nametable window
                    case SDLK_n:
                        if( config.nametables_win.size == 0 ) {
                            SDL_ShowWindow( this->nametables_win );
                            config.nametables_win.size = 1;
                        }
                        else if( config.nametables_win.size == 1 ) {
                            SDL_HideWindow( this->nametables_win );
                            config.nametables_win.size = 0;
                        }
                        this->write_config();
                        break;
                }
                break;
            }
            //------------------------------------------------------------------------------------------------
            case SDL_KEYUP:
                // Basic input WIP
                switch( event.key.keysym.sym )
                {
                    case SDLK_x:     Nes_SetInputState( this->nes, 0, Nes_A,      0 ); break;
                    case SDLK_z:     Nes_SetInputState( this->nes, 0, Nes_B,      0 ); break;
                    case SDLK_c:     Nes_SetInputState( this->nes, 0, Nes_Select, 0 ); break;
                    case SDLK_v:     Nes_SetInputState( this->nes, 0, Nes_Start,  0 ); break;
                    case SDLK_UP:    Nes_SetInputState( this->nes, 0, Nes_Up,     0 ); break;
                    case SDLK_DOWN:  Nes_SetInputState( this->nes, 0, Nes_Down,   0 ); break;
                    case SDLK_LEFT:  Nes_SetInputState( this->nes, 0, Nes_Left,   0 ); break;
                    case SDLK_RIGHT: Nes_SetInputState( this->nes, 0, Nes_Right,  0 ); break;
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
                    render();
                    
                    // WIP mid-frame scroll debug
                    // printf( "X:%3d Y:%3d ", this->nes->ppu.scroll.last_frame.scroll_x[0].value, this->nes->ppu.scroll.last_frame.start_y );
                    // for( int i = 1; i < this->nes->ppu.scroll.last_frame.count; ++i ) {
                    //     printf( "< %3d %3d > . ", 
                    //         this->nes->ppu.scroll.last_frame.scroll_x[ i ].scanline,
                    //         this->nes->ppu.scroll.last_frame.scroll_x[ i ].value );
                    // }
                    // printf( "\n" );
                }
                break;
        }
    }
}
//------------------------------------------------------------------------------------------------------------
void drawRect( SDL_Surface *surface, SDL_Rect *rect, Uint32 color );

void ModNES::render()
{
    SDL_SetColorKey( this->patterns_surf, SDL_FALSE, 0 );
    this->renderNametables();
    
    // Clear the screen to magenta to spot blitting problems
    SDL_FillRect( screen_surf, NULL, SDL_MapRGB( screen_surf->format, 0xFF, 0, 0xFF ));
    
    // ---- begin split scroll code ----
    SDL_Rect viewport = {
        nes->ppu.scroll.last_frame.scroll_x[0].value,
        nes->ppu.scroll.last_frame.start_y,
        256, 240 };
    SDL_Rect destport = { 0, 0, 256, 240 }; // width and height shouldn't care, blit unscaled doesn't use them
    
    for( int i = 0; i < nes->ppu.scroll.last_frame.count; ++i )
    {
        destport.y = nes->ppu.scroll.last_frame.scroll_x[ i ].scanline +1;
        
        viewport.x = nes->ppu.scroll.last_frame.scroll_x[ i ].value;
        
        viewport.y = nes->ppu.scroll.last_frame.start_y + destport.y;
        
        if( i < nes->ppu.scroll.last_frame.count -1 ) {
            viewport.h = nes->ppu.scroll.last_frame.start_y +
                nes->ppu.scroll.last_frame.scroll_x[ i+1 ].scanline - viewport.y +1;
        }
        else {
            viewport.h = nes->ppu.scroll.last_frame.start_y + 240 - viewport.y;
        }
        
        SDL_BlitSurface( nametables_surf, &viewport, screen_surf, &destport );
        drawRect( nametables_surf, &viewport, 0x00FF00 );
        
        // Horizontal scroll wrap around
        if( viewport.x >= 256 )
        {
            SDL_Rect hor_viewport = viewport;
            SDL_Rect hor_destport = destport;

            hor_viewport.x -= 512;
            hor_destport.x = 0;

            SDL_BlitSurface( nametables_surf, &hor_viewport, screen_surf, &hor_destport );
            drawRect( nametables_surf, &hor_viewport, 0x00FF00 );
        }
        
        // Vertical scroll wrap around
        if( viewport.y >= 240 )
        {
            SDL_Rect ver_viewport = viewport;
            SDL_Rect ver_destport = destport;
            
            ver_viewport.y -= 480;
            ver_destport.y = 0;
            
            SDL_BlitSurface( nametables_surf, &ver_viewport, screen_surf, &ver_destport );
            drawRect( nametables_surf, &ver_viewport, 0x00FF00 );
            
            // Vertical and horizontal wrap around
            if( viewport.x >= 256 )
            {
                ver_destport = destport;
                
                ver_viewport.x -= 512;
                ver_destport.y = ver_destport.x = 0;
                
                SDL_BlitSurface( nametables_surf, &ver_viewport, screen_surf, &ver_destport );
                drawRect( nametables_surf, &ver_viewport, 0x00FF00 );
            }
        }
    }
    // ---- end split scroll code ----
    
    SDL_SetColorKey( this->patterns_surf, SDL_TRUE, 0 );
    this->renderSprites();
    
    // Present nametables
    SDL_BlitSurface( this->nametables_surf, NULL, SDL_GetWindowSurface( nametables_win ), NULL );
    
    // Hide top and bottom tile rows. WIP: totally unoptimal solution, better not to render there at all
    SDL_Rect rect = { 0, 0, 256, 8 };
    if( this->hide_top_bottom ) {
        SDL_FillRect( screen_surf, &rect, 0 );
        rect.y = 240 - 8;
        SDL_FillRect( screen_surf, &rect, 0 );
    }
    
    SDL_BlitScaled( screen_surf, NULL, SDL_GetWindowSurface( screen_win ), NULL );
    SDL_UpdateWindowSurface( nametables_win );
    SDL_UpdateWindowSurface( screen_win );
}

void drawRect( SDL_Surface *surface, SDL_Rect *rect, Uint32 color )
{
    SDL_Rect rects[4] = {
        { rect->x, rect->y, rect->w, 1 },
        { rect->x, rect->y + rect->h -1, rect->w, 1 },
        { rect->x, rect->y, 1, rect->h },
        { rect->x + rect->w -1, rect->y, 1, rect->h }
    };
    SDL_FillRects( surface, rects, 4, color );
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
    
    printf( "Loaded %s\n", strrchr( path, '/' ) +1 );
    strncpy( this->config.romFileName, path, 1024 );
    this->write_config();
    
    Nes_Reset( this->nes );
    this->renderPatterns();
    this->presentPatterns(); //WIP move this outta here
    
    if( ! this->running ) {
        this->running = true;
        if( this->timer_id == 0 ) {
            this->timer_id = SDL_AddTimer( 1000 / 60, vblank_callback, NULL );
        }
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
                int x = tilex * 8 + chrom * 129;
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
    // to keep CMY palette: SDL_SetSurfacePalette( this->patterns_surf, this->patterns_pal );
    
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
    
    // Whether the background tiles are in CHR ROM 0 at $0 or CHR ROM 1 at $1000
    int chrom_shift = nes->ppu.back_pattern == 0 ? 0 : 129;
    
    for( int table = 0; table <= 1; ++table )
    {
        int table_shift_x = 0, table_shift_y = 0;
        if( nes->ppu.mirroring == mirroring_vertical && table == 1 ) {
            table_shift_x = 256;
        }
        else if( nes->ppu.mirroring == mirroring_horizontal && table == 1 ) {
            table_shift_y = 256;
        }
        // else WIP 4 screen mirroring
    
        byte *name_ptr = this->nes->ppu.name_ptr[table];
        byte *attr_ptr = this->nes->ppu.attr_ptr[table];
        
        for( int tiley = 0; tiley < 30; ++tiley )
        {
            for( int tilex = 0; tilex < 32; ++tilex )
            {
                byte tilen = *name_ptr;
                
                patt.x = chrom_shift + (tilen % 16) * 8;
                patt.y = (tilen / 16) * 8;
                
                name.x = table_shift_x + tilex * 8;
                name.y = table_shift_y + tiley * 8;
                
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
                
                colors[0].r = Nes_rgb[ nes->ppu.palettes[0] ][0];
                colors[0].g = Nes_rgb[ nes->ppu.palettes[0] ][1];
                colors[0].b = Nes_rgb[ nes->ppu.palettes[0] ][2];
                // WIP IIRC back and sprite pallettes can be switched? or not? (I don't think so)
                for( int i = 1; i <= 3; ++i )
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
    
    name.x = name.y = 0;
    SDL_Rect mirror = { 0, 0, 0, 0 };
    if( nes->ppu.mirroring == mirroring_vertical )
    {
        mirror.w = name.w = 512;
        mirror.h = name.h = 240;
        mirror.x = 0;
        mirror.y = 240;
    }
    else {
        assert( 0 && "Horizontal mirroring not yet implemented on frontend" );
    }
    // SDL_Surface *temp = SDL_CreateRGBSurface( 0, 512, 240, 8, 0, 0, 0, 0 );
    // SDL_BlitSurface( this->nametables_surf, &name, temp, NULL );
    // SDL_BlitSurface( temp, NULL, this->nametables_surf, &mirror );
    SDL_BlitSurface( this->nametables_surf, &name, this->nametables_surf, &mirror );
}
//------------------------------------------------------------------------------------------------------------
void ModNES::renderSprites()
{
    // WIP dirty hack, handle more graciously
    if( ! this->render_sprites ) {
        return;
    }
    
    SDL_Rect patt, name, screen;
    patt.w = patt.h = name.w = name.h = 8;
    
    // Whether the sprite tiles are in CHR ROM 0 at $0 or CHR ROM 1 at $1000
    int chrom_shift = nes->ppu.sprite_pattern == 0 ? 0 : 129;
    
    SDL_Color colors[4] = {{0,0,0}};
    
    SDL_Surface *flip_surf = SDL_CreateRGBSurface( 0, 8, 8, 8, 0, 0, 0, 0 );
    SDL_Surface *temp_surf = SDL_CreateRGBSurface( 0, 8, 8, 8, 0, 0, 0, 0 );
    
    SDL_SetSurfacePalette( this->patterns_surf, this->temp_pal );
    
    for( byte *sprite = &nes->ppu.sprites[0x100-4]; sprite >= nes->ppu.sprites; sprite -= 4 )
    {
        // Sprite position on Nametables
        name.y = nes->ppu.scroll.vertical + sprite[0] + 1; // Sprite's y position is off by one
        name.x = nes->ppu.scroll.horizontal + sprite[3];
        
        // Sprite position on Screen
        screen.y = sprite[0] + 1; // Sprite's y position is off by one
        screen.x = sprite[3];
        
        if( name.x >= 512 ) {
            name.x -= 512;
        }
        if( name.y >= 480 ) {
            name.y -= 480;
        }
        
        int tilen = sprite[1];
        int paln  = sprite[2] & 3; // 2 lsb
        int xflip = sprite[2] & ( 1<<6 );
        int yflip = sprite[2] & ( 1<<7 );
        
        // WIP this is assuming sprites are in pattern table 0
        patt.x = chrom_shift + (tilen % 16) * 8;
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
        
        if( xflip == 0 && yflip == 0 ) {
            SDL_BlitSurface( this->patterns_surf, &patt, this->nametables_surf, &name );
            SDL_BlitSurface( this->patterns_surf, &patt, this->screen_surf, &screen );
        }
        else // flip the sprite. WIP (this should be pre-rendered and cached)
        {
            SDL_SetColorKey( flip_surf, SDL_TRUE, 0 );
            SDL_SetSurfacePalette( flip_surf, this->temp_pal );
            SDL_FillRect( flip_surf, NULL, 0 );
            
            SDL_SetColorKey( temp_surf, SDL_TRUE, 0 );
            SDL_SetSurfacePalette( temp_surf, this->temp_pal );
            SDL_FillRect( temp_surf, NULL, 0 );
            
            SDL_BlitSurface( this->patterns_surf, &patt, temp_surf, NULL );
            
            SDL_LockSurface( temp_surf );
            SDL_LockSurface( flip_surf );
            
            int xstart = 0, ystart = 0, xstep = 0, ystep = 0;
            
            if( xflip ) {
                xstart = 7;
                xstep  = -1;
            } else {
                xstart = 0;
                xstep  = 1;
            }
            if( yflip ) {
                ystart = 7;
                ystep  = -1;
            } else {
                ystart = 0;
                ystep  = 1;
            }
            
            for( int sourcey = 0, desty = ystart;
                 sourcey <= 7;
                 ++sourcey, desty += ystep )
            {
                for( int sourcex = 0, destx = xstart;
                     sourcex <= 7;
                     ++sourcex, destx += xstep )
                {
                    ((byte*) flip_surf->pixels) [ desty * flip_surf->pitch + destx ] =
                        ((byte*) temp_surf->pixels) [ sourcey * temp_surf->pitch + sourcex ];
                }
            }
            SDL_UnlockSurface( temp_surf );
            SDL_UnlockSurface( flip_surf );
            SDL_BlitSurface( flip_surf, NULL, this->nametables_surf, &name );
            SDL_BlitSurface( flip_surf, NULL, this->screen_surf, &screen );
        }
    }
    SDL_FreeSurface( flip_surf );
    SDL_FreeSurface( temp_surf );
}
//------------------------------------------------------------------------------------------------------------