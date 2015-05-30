
#import "NametablesView.h"
#include <OpenGL/gl.h>

@implementation NametablesView

-(void) awakeFromNib
{
   int size = self->width * self->height * 3;
   self->name0_pixels = malloc( size );
   self->name1_pixels = malloc( size );
   memset( (void *)self->name0_pixels, 0, size );
   memset( (void *)self->name1_pixels, 0, size );
}

-(void) free {
   free( self->name0_pixels );
   free( self->name1_pixels );
}

- (void)prepareOpenGL
{
   // Synchronize buffer swaps with vertical refresh rate
   GLint swapInt = 1;
   [[self openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
   glOrtho( -1, 1, 1, -1, 1, -1 );
}

-(void) drawRect: (NSRect) bounds
{
   glClear(GL_COLOR_BUFFER_BIT );
   
   glTexImage2D( GL_TEXTURE_2D, 0, 3, self->width, self->height, 0,GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*) self->name0_pixels );
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glEnable(GL_TEXTURE_2D);
   
   glBegin( GL_QUADS );
      glTexCoord2i( 0, 0 );
      glVertex2i(-1, -1);
      
      glTexCoord2i( 1, 0 );
      glVertex2i(0, -1);
      
      glTexCoord2d( 1, 1 );
      glVertex2i(0, 0);
      
      glTexCoord2d( 0, 1 );
      glVertex2i(-1, 0);
      
      if( self->vertical_mirroring )
      {
         glTexCoord2i( 0, 0 );
         glVertex2i(-1, 0);
         
         glTexCoord2i( 1, 0 );
         glVertex2i(0, 0);
         
         glTexCoord2d( 1, 1 );
         glVertex2i(0, 1);
         
         glTexCoord2d( 0, 1 );
         glVertex2i(-1, 1);
      }
      else // horizontal mirroring
      {
         glTexCoord2i( 0, 0 );
         glVertex2i(0, -1);
         
         glTexCoord2i( 1, 0 );
         glVertex2i(1, -1);
         
         glTexCoord2d( 1, 1 );
         glVertex2i(1, 0);
         
         glTexCoord2d( 0, 1 );
         glVertex2i(0, 0);
      }
   glEnd();
   
   glTexImage2D( GL_TEXTURE_2D, 0, 3, self->width, self->height, 0,GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*) self->name1_pixels );
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glEnable(GL_TEXTURE_2D);
   
   glBegin( GL_QUADS );
      if( self->vertical_mirroring )
      {
         glTexCoord2i( 0, 0 );
         glVertex2i(0, -1);
         
         glTexCoord2i( 1, 0 );
         glVertex2i(1, -1);
         
         glTexCoord2d( 1, 1 );
         glVertex2i(1, 0);
         
         glTexCoord2d( 0, 1 );
         glVertex2i(0, 0);
      }
      else // horizontal mirroring
      {
         glTexCoord2i( 0, 0 );
         glVertex2i(-1, 0);
         
         glTexCoord2i( 1, 0 );
         glVertex2i(0, 0);
         
         glTexCoord2d( 1, 1 );
         glVertex2i(0, 1);
         
         glTexCoord2d( 0, 1 );
         glVertex2i(-1, 1);
      }
      
      glTexCoord2i( 0, 0 );
      glVertex2i(0, 0);
      
      glTexCoord2i( 1, 0 );
      glVertex2i(1, 0);
      
      glTexCoord2d( 1, 1 );
      glVertex2i(1, 1);
      
      glTexCoord2d( 0, 1 );
      glVertex2i(0, 1);
   glEnd();
   
   glFlush();
}

@end