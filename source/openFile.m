
#import <Cocoa/Cocoa.h>
#include <string.h>
@import AppKit;

char* openFile( char *path, int length )
{
    NSOpenPanel *openPanel = [NSOpenPanel openPanel];
    // openPanel.canChooseFiles = YES;
    // openPanel.allowsOtherFileTypes = YES;
    // openPanel.message = @"a message on the panel body";
    openPanel.resolvesAliases = YES;
    openPanel.canChooseDirectories = NO;
    openPanel.allowsMultipleSelection = NO;
    openPanel.title = @"Open NES ROM";
    openPanel.directoryURL = [NSURL URLWithString:@"file:///Volumes/Archive/Emulators/NES/NES 1089 ROMS/!Mapper0"];
        
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