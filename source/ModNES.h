
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
    
    bool running;
    bool quit;
    
    enum Windows {
        Patterns_win = 0,
        Nametables_win = 1,
        Windows_count = 2
    };
    
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
    
    struct {
        struct {
            int frames;
            int rendered;
            int sleeps;
            int elapsed;
        } second;
        
        const int frame_time = 17; // milliseconds per frame
        int elapsed; // time elapsed since last frame
        long last_time;
    } stats;
    
    bool render_sprites;
    bool hide_top_bottom;
    
    SDL_Window *patterns_win;
    SDL_Window *nametables_win;
    SDL_Window *screen_win; // 2x window to blitscaled from screen_surf
    
    int patterns_win_id;
    int nametables_win_id;
    int screen_win_id;
    
    SDL_Surface *screen_surf; // 1x offscreen buffer for the screen
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
    void renderSprites( int priority );
    
    void read_config();
    void write_config();

    ModNES();
    ~ModNES();
};

#endif // #ifndef _ModNES_h_