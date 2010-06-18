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
// handy_sdl_main.cpp                                                       //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This is the main Handy/SDL source. It manages the main functions for     //
// emulating the Atari Lynx emulator using the SDL Library.                 //
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
//                                                                          //
// June 2010 :                                                              //
//  Pandora port with Goomba menu system by Steve Maddison.                 //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cctype>
#include <errno.h>
#include <SDL.h>
#include <SDL_main.h>
#include <SDL_timer.h>

#include "handy_sdl_main.h"
#include "handy_sdl_graphics.h"
#include "handy_sdl_handling.h"
#include "handy_sdl_sound.h"
#include "handy_sdl_usage.h"
#ifndef NOGL
#include "sdlemu/sdlemu_opengl.h"
#endif

/* SDL declarations */
SDL_Surface     *HandyBuffer;           // Our Handy/SDL display buffer
SDL_Surface     *mainSurface;           // Our Handy/SDL primary display

/* Handy declarations */
Uint32          *mpLynxBuffer;
CSystem         *mpLynx;
int              mFrameSkip = 0;
int              mpBpp;                 // Lynx rendering bpp

/* Handy/SDL declarations */
int             LynxWidth;              // Lynx SDL screen width
int             LynxHeight;             // Lynx SDL screen height
#ifdef PANDORA
int             LynxScale = 4;          // Factor to scale the display
#else
int             LynxScale = 1;          // Factor to scale the display
#endif
int             LynxLCD = 1;            // Emulate LCD Display
int             LynxFormat;             // Lynx ROM format type
int             LynxRotate;             // Lynx ROM rotation type
Uint32          overlay_format = SDL_YV12_OVERLAY; // YUV Overlay format

int             Throttle  = 1;  // Throttle to 60FPS
int             framecounter = 0; // FPS Counter
int             Autoskip = 0; // Autoskip
int             Skipped = 0;
int             frameskip = 0;   // Frameskip
int             req_frameskip = 0;  // Reuested frameskip
float           fps_counter = 0.0;
#ifdef PANDORA
int             Fullscreen = 1;
#else
int             Fullscreen = 0;
#endif
int             bpp = 0; // BPP -> 8,16 or 32. 0 = autodetect (default)
int             fsaa = 0;   // OpenGL FSAA (default off)
int             accel = 1;  // OpenGL Hardware accel (default on)
int             vsync  =    0;  // OpenGL VSYNC (default off)
int             yuvoverlay  = 1; // YUV Overlay format
char            overlaytype[4];   // Overlay Format

#define ROM_FILE_LEN 256
char            rom_file[ROM_FILE_LEN];

int             emulation = 0;
int             suspend = 0;
int             quit = 0;
int             stop = 0;
Uint8          *delta;
/*
	Handy/SDL Rendering output

	1 = SDL rendering
	2 = OpenGL rendering
	3 = YUV Overlay rendering

	Default = 1 (SDL)
*/
int             rendertype = 1;

/*
	Handy/SDL Scaling/Scanline routine

	1 = SDLEmu v1 (compatible with al SDL versions)
	2 = SDLEmu v2 (faster but might break in future SDL versions or on certain platforms)
	3 = Pierre Doucet v1 (compatible but possiby slow)

	Default = 1 (SDLEmu v1)
*/

int             stype = 1;              // Scaling/Scanline routine.


/*
	Handy/SDL Filter selection

	1 = SDLEmu v1 (compatible with al SDL versions)
	2 = SDLEmu v2 (faster but might break in future SDL versions or on certain platforms)
	3 = Pierre Doucet v1 (compatible but possiby slow)

	Default = 1 (SDLEmu v1)
*/

int             filter = 0;             // Scaling/Scanline routine.


#ifdef USE_GOOMBA
#include <goomba/control.h>
#include <goomba/gui.h>

#define MENU_ROM_NAME_LEN 256

char *config_file = "handy.cfg";

struct goomba_gui *menu_gui = NULL;

struct goomba_item *menu_main = NULL;

/* ROM selector */
struct goomba_item *menu_rom = NULL;

/* Video options */
struct goomba_item *menu_video = NULL;
struct goomba_item *menu_frameskip = NULL;
struct goomba_item *menu_showfps = NULL;
struct goomba_item *menu_lcd = NULL;
struct goomba_item *menu_filter = NULL;

/* Sound options */
struct goomba_item *menu_sound = NULL;

/* Control options */
struct goomba_item *menu_controls = NULL;
struct goomba_item *menu_ctrl_option[HANDY_NUM_CTRLS];
struct goomba_control menu_ctrl[HANDY_NUM_CTRLS];

char menu_rom_name[MENU_ROM_NAME_LEN] = "";
char menu_rom_dir[MENU_ROM_NAME_LEN] = "";

const char *menu_ctrl_name[HANDY_NUM_CTRLS] = {
	"Up", "Down", "Left", "Right", "A", "B",
	"Option1", "Option2", "Pause", "Menu", "Quit"
};

typedef enum {
	CONFIG_ROMDIR,
	CONFIG_VIDEO,
	CONFIG_VIDEO_FRAMESKIP,
	CONFIG_VIDEO_FPS,	
	CONFIG_VIDEO_LCD,
	CONFIG_VIDEO_FILTER,
	CONFIG_SOUND,
	CONFIG_CONTROL
};

const char *config_name[] = {
	"romdir",
	"video",
	"frameskip",
	"fps",
	"lcd",
	"filter",
	"sound",
	"control"
};

int menu_rom_callback( struct goomba_item *item ) {
	/* Reset with new game when ROM is loaded. */
	strncpy( rom_file, item->filesel_data.value, ROM_FILE_LEN );
	stop = 1;
	return 0;   
}

int menu_frameskip_callback( struct goomba_item *item ) {
	if( req_frameskip == 0 ) {
		Autoskip = 1;
	}
	else {
		Autoskip = 0;
		frameskip = req_frameskip;
	}
	return 0;   
}

int menu_control_callback( struct goomba_item *item ) {
	int  i = 0;

	/* Only key events work at the moment. */
	if( item->control_data.control->device_type != GOOMBA_DEV_KEYBOARD ) {
		item->control_data.control->device_type = GOOMBA_DEV_UNKNOWN;
		return -1;
	}

	for( i = 0 ; i < HANDY_NUM_CTRLS ; i++ ) {
		if( item == menu_ctrl_option[i] ) {
			handy_sdl_set_key( i, item->control_data.control->value );
			break;
		}
	}
	return 0;
}

int handy_config_read_item( char *name, char *subname, char *value ) {
	if( strcmp( name, config_name[CONFIG_ROMDIR] ) == 0 ) {
		if( value && *value ) {
			strncpy( menu_rom_dir, value, MENU_ROM_NAME_LEN );
		}
	}
	else if( strcmp( name, config_name[CONFIG_VIDEO] ) == 0 ) {
		if( strcmp( subname, config_name[CONFIG_VIDEO_FRAMESKIP] ) == 0 ) {
			*(menu_frameskip->enum_data.value) = atoi( value );
		}
		else if( strcmp( subname, config_name[CONFIG_VIDEO_FPS] ) == 0 ) {
			*(menu_showfps->enum_data.value) = atoi( value );
		}
		else if( strcmp( subname, config_name[CONFIG_VIDEO_LCD] ) == 0 ) {
			*(menu_lcd->enum_data.value) = atoi( value );
		}
		else if( strcmp( subname, config_name[CONFIG_VIDEO_FILTER] ) == 0 ) {
			*(menu_filter->enum_data.value) = atoi( value );
		}
		else {
			fprintf( stderr, "Unrecognised config item '%s.%s'.\n", name, subname );
			return -1;
		}
	}
	else if( strcmp( name, config_name[CONFIG_SOUND] ) == 0 ) {
		if( value && *value == '1' ) {
			gAudioEnabled = TRUE;
		}
		else {
			gAudioEnabled = FALSE;		
		}
	}
	else if( strcmp( name, config_name[CONFIG_CONTROL] ) == 0 ) {
		int i;
		int match = 0;
		for( i = 0 ; i < HANDY_NUM_CTRLS ; i++ ) {
			if( strcmp( subname, menu_ctrl_name[i] ) == 0 ) {
				menu_ctrl_option[i]->control_data.control->value = atoi( value );
				match = 1;
			}
		}
		if( !match ) {
			fprintf( stderr, "Unrecognised config item '%s.%s'.\n", name, subname );
			return -1;
		}
	}
	else {
		fprintf( stderr, "Unrecognised config item '%s'.\n", name );
		return -1;
	}
	
	return 0;
}

int handy_config_read( void ) {
	FILE *file = fopen( config_file, "r" );

	if( file == NULL ) {
		fprintf( stderr, "Couldn't open %s for input: %s\n", config_file, strerror(errno) );
		return -1;
	}
	else {
		char line[128];
		char *name = NULL;
		char *value;
		char *subname = NULL;
		
		while( fgets( line, 128, file ) != NULL ) {
			line[strlen(line)-1] = 0; /* Trim newline. */
			name = &line[0];
			value = strchr( line, '=' );
			
			if( value ) {
				*value = 0;
				value++; /* Point past '=' sign. */
				subname = strchr( name, '.' );
				if( subname ) {
					*subname = 0;
					subname++; /* Point past '.' separator. */
				}
				handy_config_read_item( name, subname, value );
			}
		}
		
		fclose( file );
	}
	
	return 0;
}

int handy_config_write( void ) {
	FILE *file = fopen( config_file, "w" );
	
	if( file == NULL ) {
		fprintf( stderr, "Couldn't open %s for output: %s\n", config_file, strerror(errno) );
		return -1;
	}
	else {
		char rom_dir[MENU_ROM_NAME_LEN];
		char *slash = NULL;
		int i;
		
		strncpy( rom_dir, menu_rom->filesel_data.value, MENU_ROM_NAME_LEN );
		slash = strrchr( rom_dir, '/' );
		if( slash && slash != &rom_dir[0] ) {
			*slash = 0;
		}
		fprintf( file, "%s=%s\n", config_name[CONFIG_ROMDIR], rom_dir );
		
		fprintf( file, "%s.%s=%d\n", config_name[CONFIG_VIDEO], config_name[CONFIG_VIDEO_FRAMESKIP], *(menu_frameskip->enum_data.value) );
		fprintf( file, "%s.%s=%d\n", config_name[CONFIG_VIDEO], config_name[CONFIG_VIDEO_FPS], *(menu_showfps->enum_data.value) );
		fprintf( file, "%s.%s=%d\n", config_name[CONFIG_VIDEO], config_name[CONFIG_VIDEO_LCD], *(menu_lcd->enum_data.value) );
		fprintf( file, "%s.%s=%d\n", config_name[CONFIG_VIDEO], config_name[CONFIG_VIDEO_FILTER], *(menu_filter->enum_data.value) );

		fprintf( file, "%s=%d\n", config_name[CONFIG_SOUND], *(menu_sound->enum_data.value) );

		for( i = 0 ; i < HANDY_NUM_CTRLS ; i++ ) {
			fprintf( file, "%s.%s=%d\n", config_name[CONFIG_CONTROL],
				menu_ctrl_name[i], menu_ctrl_option[i]->control_data.control->value	);
		}

		fclose( file );
	}

	return 0;
}

#endif /* USE_GOOMBA */


/*
	Name                :   handy_sdl_update
	Parameters          :   N/A
	Function            :   Update/Throttle function for Handy/SDL.

	Uses                :   N/A

	Information         :   This function is basicly the Update() function from
							Handy WIN32 with minor tweaks for SDL. It is used for
							basic throttle of the Handy core.
*/
inline  int handy_sdl_update(void)
{


		// Throttling code
		//
		if(gSystemCycleCount>gThrottleNextCycleCheckpoint)
		{
			static int limiter=0;
			static int flipflop=0;
			int overrun=gSystemCycleCount-gThrottleNextCycleCheckpoint;
			int nextstep=(((HANDY_SYSTEM_FREQ/HANDY_BASE_FPS)*gThrottleMaxPercentage)/100);

			// We've gone thru the checkpoint, so therefore the
			// we must have reached the next timer tick, if the
			// timer hasnt ticked then we've got here early. If
			// so then put the system to sleep by saying there
			// is no more idle work to be done in the idle loop

			if(gThrottleLastTimerCount==gTimerCount)
			{
				// All we know is that we got here earlier than expected as the
				// counter has not yet rolled over
				if(limiter<0) limiter=0; else limiter++;
				if(limiter>40 && mFrameSkip>0)
				{
					mFrameSkip--;
					limiter=0;
				}
				flipflop=1;
				return 0;
			}

			// Frame Skip adjustment
			if(!flipflop)
			{
				if(limiter>0) limiter=0; else limiter--;
				if(limiter<-7 && mFrameSkip<10)
				{
					mFrameSkip++;
					limiter=0;
				}
			}

			flipflop=0;

			//Set the next control point
			gThrottleNextCycleCheckpoint+=nextstep;

			// Set next timer checkpoint
			gThrottleLastTimerCount=gTimerCount;

			// Check if we've overstepped the speed limit
			if(overrun>nextstep)
			{
				// We've exceeded the next timepoint, going way too
				// fast (sprite drawing) so reschedule.
				return 0;
			}

		}

		return 1;

}

/*
	Name                :   handy_sdl_rom_info
	Parameters          :   N/A
	Function            :   Game Image information function for Handy/SDL

	Uses                :   N/A

	Information         :   Basic function for getting information of the
							Atari Lynx game image and for setting up the
							Handy core concerning rotation.
*/
void handy_sdl_rom_info(void)
{

	printf("Atari Lynx ROM Information\n");

	/* Retrieving Game Image information */
	printf("Cartname      : %s\n"   , mpLynx->CartGetName()         );
	printf("ROM Size      : %d kb\n", (int)mpLynx->CartSize()      );
	printf("Manufacturer  : %s\n"   , mpLynx->CartGetManufacturer() );

	/* Retrieving Game Image Rotatation */
	printf("Lynx Rotation : ");
	switch(mpLynx->CartGetRotate())
	{
		case CART_NO_ROTATE:
			LynxRotate = MIKIE_NO_ROTATE;
			printf("NO\n");
			break;
		case CART_ROTATE_LEFT:
			LynxRotate = MIKIE_ROTATE_L;
			printf("LEFT\n");
			break;
		case CART_ROTATE_RIGHT:
			LynxRotate = MIKIE_ROTATE_R;
			printf("RIGHT\n");
			break;
		default:
			// Allright, this shouldn't be necassary. But in case the user is using a
			// bad dump, we use the default rotation as in no rotation.
			LynxRotate = MIKIE_NO_ROTATE;
			printf("NO (forced)\n");
			break;
	}

	/* Retrieving Game Image type */
	printf("ROM Type      : ");
	switch(mpLynx->mFileType)
	{
		case HANDY_FILETYPE_HOMEBREW:
			printf("Homebrew\n");
			break;
		case HANDY_FILETYPE_LNX:
			printf("Commercial and/or .LNX-format\n");
			break;
		case HANDY_FILETYPE_SNAPSHOT:
			printf("Snapshot\n");
			break;
		default:
			// Allright, this shouldn't be necessary, but just in case.
			printf("Unknown format!\n");
			exit(EXIT_FAILURE);
			break;
	}

}

int handy_run_game( void )  {
	SDL_Event   handy_sdl_event;
	Uint32      handy_sdl_start_time = 0;
	Uint32      handy_sdl_this_time = 0;
	Uint32      handy_sdl_suspend_time = 0;

	emulation = 0;
	suspend = 0;
	stop = 0;
	quit = 0;

	// Primary initalise of Handy
	printf("Initialising Handy Core...    ");
	try {
		// Ugh, hardcoded lynxboot.img. Will be fixed in future versions.
		mpLynx = new CSystem(rom_file, "lynxboot.img");
	} catch (CLynxException &err) {
		cerr << err.mMsg.str() << ": " << err.mDesc.str() << endl;
		exit(EXIT_FAILURE);
	}
	printf("[DONE]\n\n");

	// Query Rom Image information
	handy_sdl_rom_info();

	// Initialise Handy/SDL video
	if( !handy_sdl_video_setup(rendertype,fsaa,Fullscreen, bpp, LynxScale, accel, vsync) )
	{
		return -1;
	}

	// Setup of Handy Core video
	handy_sdl_video_init(mpBpp);

#ifdef USE_GOOMBA
	// Show menu if no ROM was specified.
	if( *rom_file == 0 ) {
		suspend = 1;
	}
#endif

	if( gAudioEnabled ) {
		SDL_PauseAudio(0);
	}

	handy_sdl_start_time = SDL_GetTicks();
	handy_sdl_suspend_time = 0;

	printf("Starting Lynx Emulation...\n");
	while(!emulation)
	{
		// Initialise Handy button events
		int OldKeyMask, KeyMask = mpLynx->GetButtonData();
		OldKeyMask = KeyMask;

		// Getting events for keyboard and/or joypad handling
		while(SDL_PollEvent(&handy_sdl_event))
		{
			switch(handy_sdl_event.type)
			{
				case SDL_KEYUP:
					KeyMask = handy_sdl_on_key_up(handy_sdl_event.key, KeyMask);
					break;
				case SDL_KEYDOWN:
					KeyMask = handy_sdl_on_key_down(handy_sdl_event.key, KeyMask);
					break;
				default:
					KeyMask = 0;
					break;
			}
		}

		// Checking if we had SDL handling events and then we'll update the Handy button events.
		if (OldKeyMask != KeyMask)
			mpLynx->SetButtonData(KeyMask);

		// Update TimerCount
		gTimerCount++;

		while( handy_sdl_update()  )
		{
			if(!gSystemHalt)
			{
				for(ULONG loop=1024;loop;loop--)
				{
					mpLynx->Update();
				}
			}
			else
			{
#ifdef HANDY_SDL_DEBUG
					printf("gSystemHalt : %d\n", gSystemHalt);
#endif
					gTimerCount++;
			}
		}

		if( quit ) {
			return -1;
		}
		else if( stop ) {
			break;
		}
		else if( suspend ) {
			Uint32 suspend_ticks = SDL_GetTicks();
			if( gAudioEnabled ) {
				SDL_PauseAudio(1);
			}
			
			/* Do stuff here. */
			goomba_gui_start( menu_gui, mainSurface );
			Uint32 col = SDL_MapRGB( mainSurface->format, 0, 0, 0 );
			SDL_FillRect( mainSurface, NULL, col );
			SDL_Flip( mainSurface );
			
			/* resume */
			suspend_ticks = SDL_GetTicks() - suspend_ticks;
			handy_sdl_suspend_time += suspend_ticks;
			
			if( gAudioEnabled ) {
				SDL_PauseAudio(0);
			}
			
			suspend = 0;
		}
		
		handy_sdl_this_time = SDL_GetTicks() - handy_sdl_suspend_time;

		fps_counter = (((float)gTimerCount/(handy_sdl_this_time-handy_sdl_start_time))*1000.0);
#ifdef HANDY_SDL_DEBUG
		printf("fps_counter : %f\n", fps_counter);
#endif

		if( (Throttle) && (fps_counter > 59.99) ) SDL_Delay( (Uint32)fps_counter );

		if(Autoskip)
		{
			if(fps_counter > 60)
			{
				frameskip--;
				Skipped = frameskip;
			}
			else
			{
				if(fps_counter < 60)
				{
					Skipped++;
					frameskip++;
				}
			}
		}


		if ( framecounter )
		{

			if ( handy_sdl_this_time != handy_sdl_start_time )
			{
				static char buffer[256];

				sprintf (buffer, "Handy %0.0f", fps_counter);
				strcat( buffer, "FPS");
				SDL_WM_SetCaption( buffer , "HANDY" );
			}
		}


	}
	
	// Disable audio and set emulation to pause, then quit :)
	SDL_PauseAudio(1);
	emulation   = -1;

	//Let is give some free memory
	if( mpLynx != NULL ) delete mpLynx;
	free(mpLynxBuffer);
	
	return 0;
}

int main(int argc, char *argv[])
{
	int i;

	gAudioEnabled = TRUE;

	// Default output
	printf("Handy GCC/SDL Portable Atari Lynx Emulator %s\n", HANDY_SDL_VERSION);
	printf("Based upon %s by Keith Wilkins\n", HANDY_VERSION);
	printf("Written by SDLEmu Team, additions by Pierre Doucet\n");
#ifdef USE_GOOMBA
	printf("Goomba menu system by Steve Maddison\n");
#endif
	printf("Contact: http://sdlemu.ngemu.com | shalafi@xs4all.nl\n\n");

#ifdef USE_GOOMBA
	menu_gui = goomba_gui_create();

	menu_main = goomba_item_create( GOOMBA_ITEM_MENU );
	menu_main->text = "Handy: Main Menu";
	menu_gui->root = menu_main;
	menu_gui->config.font.name = "FreeSans.ttf";
	menu_gui->config.font.size = 20;
	menu_gui->config.help = 1;
	
	menu_gui->config.selector.fill.red = 255;
	menu_gui->config.selector.fill.green = 192;
	menu_gui->config.selector.fill.blue = 0;
	menu_gui->config.selector.alpha = 192;
#ifdef PANDORA
	menu_gui->config.control[GOOMBA_EVENT_SKIP_F].value = SDLK_RCTRL;
	menu_gui->config.control[GOOMBA_EVENT_SKIP_B].value = SDLK_RSHIFT;
	menu_gui->config.control[GOOMBA_EVENT_SELECT].value = SDLK_END;
	menu_gui->config.control[GOOMBA_EVENT_BACK].value = SDLK_PAGEDOWN;
#endif

	/* ROM Selector */
	menu_rom = goomba_item_create( GOOMBA_ITEM_FILESEL );
	goomba_item_append_child( menu_main, menu_rom );
	menu_rom->text = "Load ROM";
	menu_rom->filesel_data.value = menu_rom_name;
	menu_rom->filesel_data.directory = menu_rom_dir;
	menu_rom->callback = menu_rom_callback;
	menu_rom->filesel_data.size = MENU_ROM_NAME_LEN;
	menu_rom->action = GOOMBA_ACTION_EXIT;

	/* Video options */
	menu_video = goomba_item_create( GOOMBA_ITEM_MENU );
	goomba_item_append_child( menu_main, menu_video );
	menu_video->text = "Video Options";

	/* Video: Frameskip */  
	menu_frameskip = goomba_item_create( GOOMBA_ITEM_ENUM );
	goomba_item_append_child( menu_video, menu_frameskip );
	menu_frameskip->text = "Frameskip";
	menu_frameskip->enum_data.value = &req_frameskip;
	menu_frameskip->callback = menu_frameskip_callback;
	goomba_add_enum_option( menu_frameskip, "Auto", 0 );
	goomba_add_enum_option( menu_frameskip, "1", 1 );
	goomba_add_enum_option( menu_frameskip, "2", 2 );
	goomba_add_enum_option( menu_frameskip, "3", 3 );
	goomba_add_enum_option( menu_frameskip, "4", 4 );
	goomba_add_enum_option( menu_frameskip, "5", 5 );
	goomba_add_enum_option( menu_frameskip, "6", 6 );
	goomba_add_enum_option( menu_frameskip, "7", 7 );
	goomba_add_enum_option( menu_frameskip, "8", 8 );
	goomba_add_enum_option( menu_frameskip, "9", 9 );

	/* Video: Show FPS */
	menu_showfps = goomba_item_create( GOOMBA_ITEM_ENUM );
	goomba_item_append_child( menu_video, menu_showfps );
	menu_showfps->text = "Show FPS";
	menu_showfps->enum_data.value = &framecounter;
	goomba_add_enum_option( menu_showfps, "No", 0 );
	goomba_add_enum_option( menu_showfps, "Yes", 1 );   

	/* Video: LCD Effect */
	menu_lcd = goomba_item_create( GOOMBA_ITEM_ENUM );
	goomba_item_append_child( menu_video, menu_lcd );
	menu_lcd->text = "LCD Effect";
	menu_lcd->enum_data.value = &LynxLCD;
	goomba_add_enum_option( menu_lcd, "On", 1 );
	goomba_add_enum_option( menu_lcd, "Off", 0 );

	/* Video: filter */
	menu_filter = goomba_item_create( GOOMBA_ITEM_ENUM );
	goomba_item_append_child( menu_video, menu_filter );
	menu_filter->text = "Filter";
	menu_filter->enum_data.value = &filter;
	goomba_add_enum_option( menu_filter, "None", 0 );
	goomba_add_enum_option( menu_filter, "2xSAI", 2 );
	goomba_add_enum_option( menu_filter, "Average", 10 );
	goomba_add_enum_option( menu_filter, "Bilinear", 7 );
	goomba_add_enum_option( menu_filter, "BilinearPlus", 8 );
	goomba_add_enum_option( menu_filter, "MotionBlur", 5 );
	goomba_add_enum_option( menu_filter, "Pixelate", 9 );
	goomba_add_enum_option( menu_filter, "Simple2x", 6 );
	goomba_add_enum_option( menu_filter, "Super 2xSAI", 3 );
	goomba_add_enum_option( menu_filter, "SuperEagle", 4 );
	goomba_add_enum_option( menu_filter, "TV Mode", 1 );

	/* Sound options */
	menu_sound = goomba_item_create( GOOMBA_ITEM_ENUM );
	goomba_item_append_child( menu_main, menu_sound );
	menu_sound->text = "Sound";
	menu_sound->enum_data.value = (int*)&gAudioEnabled;
	goomba_add_enum_option( menu_sound, "On", (int)TRUE );
	goomba_add_enum_option( menu_sound, "Off", (int)FALSE );

	/* Control options */
	menu_controls = goomba_item_create( GOOMBA_ITEM_MENU );
	goomba_item_append_child( menu_main, menu_controls );
	menu_controls->text = "Controls";

	for( i = 0 ; i < HANDY_NUM_CTRLS ; i++ ) {
		menu_ctrl_option[i] = goomba_item_create( GOOMBA_ITEM_CONTROL );
		menu_ctrl_option[i]->text = (char*)menu_ctrl_name[i];
		menu_ctrl_option[i]->control_data.control = &menu_ctrl[i];
		menu_ctrl[i].device_type = GOOMBA_DEV_KEYBOARD;
		menu_ctrl[i].value = handy_sdl_get_key( i ); 
		menu_ctrl_option[i]->callback = menu_control_callback;
		goomba_item_append_child( menu_controls, menu_ctrl_option[i] );
	}

	handy_config_read();

	*rom_file = 0;
	if( argc >= 2 ) {
		strncpy( rom_file, argv[1], ROM_FILE_LEN );
	}
#else
	if (argc < 2) {
		handy_sdl_usage();
		exit(EXIT_FAILURE);
	}
#endif

	for ( i=0; (i < argc || argv[i] != NULL ); i++ )
	{
		if (!strcmp(argv[i], "-throttle"))  Throttle = 1;
		if (!strcmp(argv[i], "-nothrottle"))    Throttle = 0;
		if (!strcmp(argv[i], "-autoskip"))  Autoskip = 1;
		if (!strcmp(argv[i], "-noautoskip"))    Autoskip = 0;
		if (!strcmp(argv[i], "-fps"))           framecounter = 1;
		if (!strcmp(argv[i], "-nofps"))         framecounter = 0;
		if (!strcmp(argv[i], "-sound"))         gAudioEnabled = TRUE;
		if (!strcmp(argv[i], "-nosound"))       gAudioEnabled = FALSE;
		if (!strcmp(argv[i], "-fullscreen"))    Fullscreen = 1;
		if (!strcmp(argv[i], "-nofullscreen"))  Fullscreen = 0;
		if (!strcmp(argv[i], "-fsaa"))          fsaa = 1;
		if (!strcmp(argv[i], "-nofsaa"))        fsaa = 0;
		if (!strcmp(argv[i], "-accel"))     accel = 1;
		if (!strcmp(argv[i], "-noaccel"))       accel = 0;
		if (!strcmp(argv[i], "-sync"))          vsync = 1;
		if (!strcmp(argv[i], "-nosync"))        vsync = 0;
		if (!strcmp(argv[i], "-2")) LynxScale = 2;
		if (!strcmp(argv[i], "-3")) LynxScale = 3;
		if (!strcmp(argv[i], "-4")) LynxScale = 4;
		if (!strcmp(argv[i], "-lcd")) LynxLCD = 1;
		if (!strcmp(argv[i], "-nolcd")) LynxLCD = 0;
		if (!strcmp(argv[i], "-frameskip"))
		{
			req_frameskip = atoi(argv[++i]);
			if ( req_frameskip > 9 )
				req_frameskip = 9;
			frameskip = req_frameskip;
		}
		if (!strcmp(argv[i], "-bpp"))
		{
			bpp = atoi(argv[++i]);
			if ( (bpp != 0) && (bpp != 8) && (bpp != 15) && (bpp != 16) && (bpp != 24) && (bpp != 32) )
			{
				bpp = 0;
			}
		}
		if (!strcmp(argv[i], "-rtype"))
		{
			rendertype = atoi(argv[++i]);
			if ( (rendertype != 1) && (rendertype != 2) && (rendertype != 3))
			{
				rendertype = 1;
			}
		}

		if (!strcmp(argv[i], "-stype"))
		{
			stype = atoi(argv[++i]);
			if ( (stype != 1) && (stype != 2) && (stype != 3))
			{
				stype = 1;
			}
		}

		if (!strcmp(argv[i], "-filter"))
		{
			filter = atoi(argv[++i]);
			// Check if the filter number is larger then 1 and not more then 10.
			if ( (filter <= 10) && (filter >= 1) )
			{
				rendertype =  1;  // Filter type only works with SDL rendering
				LynxScale  =  2;  // Maximum size is 2 times
				bpp        = 16;  // Maximum BPP is 16.
			}
			// Otherwise disable the filter
			else
			{
				filter = 0;
			}
		}

		if (!strcmp(argv[i], "-format"))
		{
			yuvoverlay  = atoi(argv[++i]);
			if ( ( yuvoverlay <= 5  ) && (yuvoverlay >= 1)  )
			{
				switch(yuvoverlay)  {
					case 1: 
						overlay_format = SDL_YV12_OVERLAY;
						strcpy( overlaytype, "YV12" );
						break;
					case 2:
						overlay_format = SDL_IYUV_OVERLAY;
						strcpy( overlaytype, "IYUV" );
						break;
					case 3:
						overlay_format = SDL_YUY2_OVERLAY;
						strcpy( overlaytype, "YUY2" );
						break;
					case 4:
						overlay_format = SDL_UYVY_OVERLAY;
						strcpy( overlaytype, "UYVY" );
						break;
					case 5:
						overlay_format = SDL_YVYU_OVERLAY;
						strcpy( overlaytype, "YVYU" );
						break;
				}
			}   
			else
			{
				overlay_format = SDL_YV12_OVERLAY;
				strcpy( overlaytype, "YV12" );
				
			}
			printf("Using YUV Overlay format: %s\n",overlaytype);
		}
	}

#ifdef USE_GOOMBA
	// Ensure menu reflects current config values.
	goomba_item_refresh( menu_gui->root );
#endif

	// Initalising SDL for Audio and Video support
	printf("Initialising SDL...           ");
	if (SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "FAILED : Unable to init SDL: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	printf("[DONE]\n");

	// Initialise Handy/SDL audio
	printf("\nInitialising SDL Audio...     ");
	if(handy_sdl_audio_init())
	{
		gAudioEnabled = TRUE;
	}
	printf("[DONE]\n");

	while( handy_run_game() != -1 );
	
	//Remove YUV Overlay
	if ( rendertype == 3 )
		handy_sdl_video_close();

	// Destroy SDL Surface's
	SDL_FreeSurface(HandyBuffer);
	SDL_FreeSurface(mainSurface);

	// Close SDL Subsystems
	SDL_QuitSubSystem(SDL_INIT_VIDEO|SDL_INIT_AUDIO);
	SDL_Quit();

#ifdef USE_GOOMBA
	handy_config_write();
#endif
	
	return EXIT_SUCCESS;
}

