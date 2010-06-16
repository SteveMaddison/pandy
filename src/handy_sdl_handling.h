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
// handy_sdl_handling.h                                                     //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This is the Handy/SDL handling header file. It manages the handling      //
// functions for emulating the Atari Lynx emulator using the SDL Library.   //
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

#ifndef __HANDY_SDL_HANDLING_H__
#define __HANDY_SDL_HANDLING_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#include <SDL_main.h>
#include <SDL_timer.h>

enum handy_ctrl {
	HANDY_CTRL_UP,
	HANDY_CTRL_DOWN,
	HANDY_CTRL_LEFT,
	HANDY_CTRL_RIGHT,
	HANDY_CTRL_A,
	HANDY_CTRL_B,
	HANDY_CTRL_OPTION1,
	HANDY_CTRL_OPTION2,
	HANDY_CTRL_PAUSE,
#ifdef USE_GOOMBA
	HANDY_CTRL_MENU,
#endif
	HANDY_CTRL_QUIT,
	HANDY_NUM_CTRLS
};

int		handy_sdl_get_key( int ctrl );
int		handy_sdl_set_key( int ctrl, int key );

int		handy_sdl_on_key_up(SDL_KeyboardEvent key, int mask);
int		handy_sdl_on_key_down(SDL_KeyboardEvent key, int mask);
#endif
