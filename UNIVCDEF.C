#include "devconst.h"

NAME CURSOR

DESCRIPTION DC_DRIVERDESC Cursor Routines

DATA FIXED SINGLE

EXPORTS
	Inquire             @1
	SetCursor           @2
	ShowCursor          @3
	HideCursor          @4
#if (DC_USES_SAVE_AREA == 1)
	GetSaveAddress      @5
#endif

IMPORTS
	dsSetPixel = DISPLAY.14
	dsGetPixel = DISPLAY.15
#if (DC_MODIFIABLE_RES == 1)
	GetDimensions = DISPLAY.16
#endif
