
#import "ModNesOpenGLView.h"
#include <OpenGL/gl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

@implementation ModNesOpenGLView

-(void) awakeFromNib {
   self->pixels = malloc( self->width * self->height * 3 );
   memset( (void *)self->pixels, 0, self->width * self->height * 3 );
}

-(void) drawRect: (NSRect) bounds
{
   glClear(GL_COLOR_BUFFER_BIT );
   
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
   glPixelStorei(GL_PACK_ALIGNMENT, 1);

   glTexImage2D( GL_TEXTURE_2D, 0, 3, self->width, self->height, 0,GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*) self->pixels );
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
   // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP); 
   glEnable(GL_TEXTURE_2D);
   
   glBegin( GL_QUADS );
      glTexCoord2i( 0, 0 );
      // glVertex2i(0, 0);
      glVertex2i(0, 0);
      
      glTexCoord2i( 1, 0 );
      // glVertex2i( self->width, 0 );
      glVertex2i(1, 0);
      
      glTexCoord2d( 1, 1 );
      // glVertex2i( self->width, self->height );
      glVertex2i(1, 1);
      
      glTexCoord2d( 0, 1 );
      // glVertex2i( 0, self->height );
      glVertex2i(0, 1);
   glEnd();
   
   glFlush();
}

- (void)prepareOpenGL
{
   // Synchronize buffer swaps with vertical refresh rate
   GLint swapInt = 1;
   [[self openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
   
   // glMatrixMode( GL_PROJECTION );
   // glLoadIdentity();
   // glOrtho( 0, self->width, self->height, 0, 1, -1 );
   glOrtho( 0, 1, 1, 0, 1, -1 );
   // glMatrixMode( GL_MODELVIEW );
   // glClearColor(0.0f, 0.0f, 0.0f, 1.0f);   
}

-(void) free {
   free( self->pixels );
}

@end