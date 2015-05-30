#import <Cocoa/Cocoa.h>
#include "ModNesOpenGLView.h"
#include "NametablesView.h"
#include "Nes.h"

@interface AppDelegate : NSObject <NSApplicationDelegate> {
   Nes *nes;
}
   @property (assign) IBOutlet ModNesOpenGLView *CHR_ROM_0;
   @property (assign) IBOutlet ModNesOpenGLView *CHR_ROM_1;
   @property (assign) IBOutlet NametablesView *Nametables;
   @property (assign) IBOutlet NSWindow *window;
@end
