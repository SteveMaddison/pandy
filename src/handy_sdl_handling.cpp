//
// Copyright (c) 2004 SDLemu Team
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from
// the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//

//////////////////////////////////////////////////////////////////////////////
//                       Handy/SDL - An Atari Lynx Emulator                 //
//                             Copyright (c) 2005                           //
//                                SDLemu Team                               //
//                                                                          //
//                          Based upon Handy v0.90 WIN32                    // 
//                            Copyright (c) 1996,1997                       //
//                                  K. Wilkins                              //
//////////////////////////////////////////////////////////////////////////////
// handy_sdl_graphics.cpp                                                   //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This is the Handy/SDL handling. It manages the handling functions        //
// of the keyboard and/or joypad for emulating the Atari Lynx emulator      //
// using the SDL Library.             										//
//                                                                          //
//    N. Wagenaar                                                           //
// December 2005                                                            //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
// Revision History:                                                        //
// -----------------                                                        //
//                                                                          //
// December 2005 :                                                          //
//  Since the 14th of April, the WIN32 of Handy (written by Keith Wilkins)  //
//  Handy has become OpenSource. Handy/SDL v0.82 R1 was based upon the old  //
//  v0.82 sources and was released closed source.                           //
//                                                                          //
//  Because of this event, the new Handy/SDL will be released as OpenSource //
//  but is rewritten from scratch because of lost sources (tm). The SDLemu  //
//  team has tried to bring Handy/SDL v0.1 with al the functions from the   //
//  closed source version.                                                  //
//////////////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cctype>
#include <SDL.h>
#include <SDL_main.h>
#include <SDL_timer.h>
#ifdef USE_GOOMBA
#include <goomba/gui.h>
#endif

#include "handy_sdl_main.h"
#include "handy_sdl_handling.h"

static int controls[HANDY_NUM_CTRLS] = {
#ifndef PANDORA
	SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT,
	SDLK_z,			/* A */
	SDLK_x,			/* B */
	SDLK_F1,		/* Option 1 */
	SDLK_F2,		/* Option 2 */
	SDLK_RETURN,	/* Pause */
#ifdef USE_GOOMBA
	SDLK_m,			/* Menu */
#endif
	SDLK_ESCAPE		/* Quit */
#else /* ifndef PANDORA */
	SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT,
	SDLK_PAGEDOWN,	/* A */
	SDLK_HOME,		/* B */
	SDLK_PAGEUP,	/* Option 1 */
	SDLK_END,		/* Option 2 */
	SDLK_LALT,		/* Pause */
#ifdef USE_GOOMBA
	SDLK_LCTRL,		/* Menu */
#endif
	SDLK_ESCAPE		/* Quit */
#endif /* ifndef PANDORA */
};

#ifdef USE_GOOMBA
extern struct goomba_gui *menu_gui;
extern SDL_Surface *mainSurface;
#endif

int  handy_sdl_get_key( int ctrl ) {
	if( ctrl >= 0 && ctrl < HANDY_NUM_CTRLS ) {
		return controls[ctrl];
	}
	return -1;
}

int  handy_sdl_set_key( int ctrl, int key ) {
	if( ctrl >= 0 && ctrl < HANDY_NUM_CTRLS ) {
		controls[ctrl] = key;
		return 0;
	}
	return -1;
}

int  handy_sdl_on_key_down(SDL_KeyboardEvent key, int mask)
{
//	Sint16 x_move = 0, y_move = 0;
	
/*    
    if(joy) {
    	x_move = SDL_JoystickGetAxis(joystick, 0);
		y_move = SDL_JoystickGetAxis(joystick, 1);
    }
*/  

    if( key.keysym.sym == controls[HANDY_CTRL_LEFT] )
		mask|= BUTTON_LEFT;
	else if( key.keysym.sym == controls[HANDY_CTRL_RIGHT] )
		mask|= BUTTON_RIGHT;
	else if( key.keysym.sym == controls[HANDY_CTRL_UP] )
		mask|= BUTTON_UP;
    else if( key.keysym.sym == controls[HANDY_CTRL_DOWN] )
		mask|= BUTTON_DOWN;
	else if( key.keysym.sym == controls[HANDY_CTRL_PAUSE] )
		mask|= BUTTON_PAUSE;
    else if( key.keysym.sym == controls[HANDY_CTRL_B] )
        mask|= BUTTON_B;
    else if( key.keysym.sym == controls[HANDY_CTRL_A] )
		mask|= BUTTON_A; 
	else if( key.keysym.sym == controls[HANDY_CTRL_OPTION1] )
		mask|= BUTTON_OPT1;
	else if( key.keysym.sym == controls[HANDY_CTRL_OPTION2] ) 
		mask|= BUTTON_OPT2;
#ifdef USE_GOOMBA
	else if( key.keysym.sym == controls[HANDY_CTRL_MENU] )
		suspend = 1; /* Display menu */
#endif
    else if( key.keysym.sym == controls[HANDY_CTRL_QUIT] )
       quit = 1;
	
/*    
    if(joy) {
    if(x_move > 32768/2)

    	eventstate |= ( HID_EVENT_RIGHT ); // seems to work fine
    			
    if(x_move < -32768/2)
    	eventstate |= ( HID_EVENT_LEFT );
  
 	if(y_move > 32768/2)
    	eventstate |= ( HID_EVENT_DOWN );
    			
    if(y_move < -32768/2)
    	eventstate |= ( HID_EVENT_UP );
    	
    if(SDL_JoystickGetButton(joystick, 1) == SDL_PRESSED)
    	eventstate |= ( HID_EVENT_A );
    
    if(SDL_JoystickGetButton(joystick, 2) == SDL_PRESSED)
    	eventstate |= ( HID_EVENT_B );
    	
    if(SDL_JoystickGetButton(joystick, 3) == SDL_PRESSED)
    	eventstate |= ( HID_EVENT_L );
    	
    if(SDL_JoystickGetButton(joystick, 4) == SDL_PRESSED)
    	eventstate |= ( HID_EVENT_R );
    }
*/

	return mask;

}

int  handy_sdl_on_key_up(SDL_KeyboardEvent key, int mask)
{
//	Sint16 x_move = 0, y_move = 0;
	
//  Uint8 *keystate = SDL_GetKeyState(NULL); // First to initialize the keystates
//	int mod = SDL_GetModState();

/*    
    if(joy) {
    	x_move = SDL_JoystickGetAxis(joystick, 0);
		y_move = SDL_JoystickGetAxis(joystick, 1);
    }
*/  

    if( key.keysym.sym == controls[HANDY_CTRL_LEFT] )
		mask &= ~BUTTON_LEFT;
	else if( key.keysym.sym == controls[HANDY_CTRL_RIGHT] )
		mask &= ~BUTTON_RIGHT;
	else if( key.keysym.sym == controls[HANDY_CTRL_UP] )
		mask &= ~BUTTON_UP;
    else if( key.keysym.sym == controls[HANDY_CTRL_DOWN] )
		mask &= ~BUTTON_DOWN;
	else if( key.keysym.sym == controls[HANDY_CTRL_PAUSE] )
		mask &= ~BUTTON_PAUSE;
    else if( key.keysym.sym == controls[HANDY_CTRL_B] )
        mask &= ~BUTTON_B;
    else if( key.keysym.sym == controls[HANDY_CTRL_A] )
		mask &= ~BUTTON_A; 
	else if( key.keysym.sym == controls[HANDY_CTRL_OPTION1] )
		mask &= ~BUTTON_OPT1;
	else if( key.keysym.sym == controls[HANDY_CTRL_OPTION2] ) 
		mask &= ~BUTTON_OPT2;
	else if( key.keysym.sym == controls[HANDY_CTRL_QUIT] ) 
       quit = 1;				

/*    
    if(joy) {
    if(x_move > 32768/2)

    	eventstate |= ( HID_EVENT_RIGHT ); // seems to work fine
    			
    if(x_move < -32768/2)
    	eventstate |= ( HID_EVENT_LEFT );
  
 	if(y_move > 32768/2)
    	eventstate |= ( HID_EVENT_DOWN );
    			
    if(y_move < -32768/2)
    	eventstate |= ( HID_EVENT_UP );
    	
    if(SDL_JoystickGetButton(joystick, 1) == SDL_PRESSED)
    	eventstate |= ( HID_EVENT_A );
    
    if(SDL_JoystickGetButton(joystick, 2) == SDL_PRESSED)
    	eventstate |= ( HID_EVENT_B );
    	
    if(SDL_JoystickGetButton(joystick, 3) == SDL_PRESSED)
    	eventstate |= ( HID_EVENT_L );
    	
    if(SDL_JoystickGetButton(joystick, 4) == SDL_PRESSED)
    	eventstate |= ( HID_EVENT_R );
    }
*/

	return mask;
}
