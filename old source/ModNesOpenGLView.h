
// My MacOS uses OpenGL version: 2.1 NVIDIA-8.16.81 310.40.00.20f04
#import <Cocoa/Cocoa.h>

@interface ModNesOpenGLView : NSOpenGLView {
   @public
      char *pixels;
      int width;
      int height;
}
- (void) drawRect: (NSRect) bounds;
- (void) free;
@end