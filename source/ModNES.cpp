
#include <SDL2/SDL.h>
#include <assert.h>
#include <stdio.h>

void error( char *msg )
{
   printf( "%s SDL_Error: %s\n", msg, SDL_GetError() );
}

int main( int argc, char* args[] )
{
}