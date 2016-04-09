
#ifndef _ModNES_h_
    #define _ModNES_h_

#include "Nes.h"
#include <SDL2/SDL.h>
#include <string>
#include <map>

class ModNES
{
public:
    Nes *nes;
    enum APP_STATE { APP_OK = 0, APP_FAILED = 1 };
    
    bool running;
    bool quit;
    
    enum Windows {
        Patterns_win = 0,
        Nametables_win = 1,
        Windows_count = 2
    };
    
    SDL_TimerID timer_id;
    
    struct {
        struct {
            int size;
            struct {
                int x;
                int y;
            } pos;
        } patterns_win, nametables_win, screen_win;
        char romFileName[1024];
    } config;
    static const std::string config_filename;
    
    bool render_sprites;
    bool hide_top_bottom;
    
    SDL_Window *patterns_win;
    SDL_Window *nametables_win;
    SDL_Window *screen_win;
    
    int patterns_win_id;
    int nametables_win_id;
    int screen_win_id;
    
    SDL_Window *pijaPuta;
    
    SDL_Surface *patterns_surf;
    SDL_Surface *nametables_surf;
    
    SDL_Palette *patterns_pal;
    SDL_Palette *temp_pal;
    
    int run();
    int init();
    void loop();
    
    void loadCartridge( char *path );
    
    void render();
    void renderPatterns();
    void presentPatterns();
    void renderNametables();
    void renderSprites();
    
    void read_config();
    void write_config();

    ModNES();
    ~ModNES();
};

#endif // #ifndef _ModNES_h_