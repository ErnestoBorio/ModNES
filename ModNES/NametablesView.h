
#import <Cocoa/Cocoa.h>

@interface NametablesView : NSOpenGLView {
   @public
      char *name0_pixels;
      char *name1_pixels;
      bool vertical_mirroring;
      int width;
      int height;
}
- (void) drawRect: (NSRect) bounds;
- (void) free;
@end