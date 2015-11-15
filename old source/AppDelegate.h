
#import <Cocoa/Cocoa.h>
#import "ModNesOpenGLView.h"
#import "NametablesView.h"
#import "MainView.h"
#import "Nes.h"

@interface AppDelegate : NSObject <NSApplicationDelegate> {
   Nes *nes;
}
   @property (assign) IBOutlet ModNesOpenGLView *CHR_ROM_0;
   @property (assign) IBOutlet ModNesOpenGLView *CHR_ROM_1;
   @property (assign) IBOutlet ModNesOpenGLView *Palettes;
   @property (assign) IBOutlet ModNesOpenGLView *Sprites;
   @property (assign) IBOutlet ModNesOpenGLView *Screen;
   @property (assign) IBOutlet NametablesView   *Nametables;

   @property (assign) IBOutlet NSWindow *window;
   @property (assign) IBOutlet MainView *mainView;
@end
