
#import <Cocoa/Cocoa.h>
#include <string.h>
@import AppKit;

char* openFile( char *dir, char *path, int length )
{
    NSOpenPanel *openPanel = [NSOpenPanel openPanel];
    openPanel.resolvesAliases = YES;
    openPanel.canChooseDirectories = NO;
    openPanel.allowsMultipleSelection = NO;
    openPanel.title = @"Open NES ROM";
    openPanel.directoryURL = [NSURL URLWithString: [NSString stringWithUTF8String: dir]];
        
    if( [openPanel runModal] == NSOKButton )
    {
        NSURL *nsurl = [[openPanel URLs] objectAtIndex:0];
        const char* str = [nsurl.path UTF8String];
        strncpy( path, str, length-1 );
        path[ length-1 ] = '\0';
    }
    else {
        path[0] = '\0';
    }
   return path;
}
