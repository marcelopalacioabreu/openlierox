/*
	OpenLieroX

	input (keyboard, mouse, ...) events and related stuff
	
	code under LGPL
	created 01-05-2007
	by Albert Zeyer and Dark Charlie
*/

#ifndef __INPUTEVENTS_H__
#define __INPUTEVENTS_H__

#include "Unicode.h"



#define     MAX_KEYQUEUE        32
#define     MAX_MOUSEBUTTONS    8 // SDL_GetMouseState returns UInt8 bitmask


// State of modifier keys (ctrl, alt and shift)
struct ModifiersState  { 
	ModifiersState() { clear(); }
	void clear()  { bShift = bCtrl = bAlt = false; }

	bool bShift;
	bool bCtrl;
	bool bAlt;
};

struct KeyboardEvent {
	int sym;
    UnicodeChar ch;
	bool down;
	ModifiersState state;
};

// Keyboard structure
// HINT: KeyDown is the state of the keyboard
// KeyUp is like an event and will only be true once
struct keyboard_t {
	Uint8	*keys;
	UnicodeChar	KeyUp[SDLK_LAST];
	UnicodeChar	KeyDown[SDLK_LAST];
    int     queueLength;
    KeyboardEvent keyQueue[MAX_KEYQUEUE];
};


// Mouse structure
struct mouse_t {
	int		X,Y;
	int		deltaX,deltaY;
	int		Button;

	// HINT: the following three variables cannot be bool
	//       because they contain info about the pressed button (left, right, ...)
	//		 as well
	// TODO: change this immediatly
	int		Up;
	int		Down;
    int     FirstDown;

	bool	WheelUp;
	bool	WheelDown;
	bool	WheelScrollUp;
	bool	WheelScrollDown;
};


void 		InitEventSystem();
bool		ProcessEvents(); // returns false if no new event
bool		WaitForNextEvent(); // returns false if no new event

keyboard_t	*GetKeyboard();
mouse_t		*GetMouse();
SDL_Event	*GetEvent();

#endif
