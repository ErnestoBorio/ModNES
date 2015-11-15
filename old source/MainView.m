//
//  MainView.m
//  ModNES
//
//  Created by Petruza on 19/06/15.
//  Copyright (c) 2015 Petruza. All rights reserved.
//

#import "MainView.h"

@implementation MainView

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        // Initialization code here.
    }
    return self;
}

-(BOOL) acceptsFirstResponder {
   return YES;
}

-(void) keyDown: (NSEvent*) theEvent
{
   switch( [theEvent keyCode] )
   {
      case 0x35: [NSApp terminate: nil]; break;
      case 0x0F: Nes_Reset( self->nes );
      case 0x07: Nes_SetInputState( self->nes, 0, Nes_A,      1 ); break;
      case 0x06: Nes_SetInputState( self->nes, 0, Nes_B,      1 ); break;
      case 0x00: Nes_SetInputState( self->nes, 0, Nes_Select, 1 ); break;
      case 0x01: Nes_SetInputState( self->nes, 0, Nes_Start,  1 ); break;
      case 0x7E: Nes_SetInputState( self->nes, 0, Nes_Up,     1 ); break;
      case 0x7D: Nes_SetInputState( self->nes, 0, Nes_Down,   1 ); break;
      case 0x7B: Nes_SetInputState( self->nes, 0, Nes_Left,   1 ); break;
      case 0x7C: Nes_SetInputState( self->nes, 0, Nes_Right,  1 ); break;
   }
}

-(void) keyUp: (NSEvent*) theEvent
{
   switch( [theEvent keyCode] )
   {
      case 0x07: Nes_SetInputState( self->nes, 0, Nes_A,      0 ); break;
      case 0x06: Nes_SetInputState( self->nes, 0, Nes_B,      0 ); break;
      case 0x00: Nes_SetInputState( self->nes, 0, Nes_Select, 0 ); break;
      case 0x01: Nes_SetInputState( self->nes, 0, Nes_Start,  0 ); break;
      case 0x7E: Nes_SetInputState( self->nes, 0, Nes_Up,     0 ); break;
      case 0x7D: Nes_SetInputState( self->nes, 0, Nes_Down,   0 ); break;
      case 0x7B: Nes_SetInputState( self->nes, 0, Nes_Left,   0 ); break;
      case 0x7C: Nes_SetInputState( self->nes, 0, Nes_Right,  0 ); break;
   }
}

@end
