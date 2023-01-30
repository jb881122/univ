#include "devconst.h"

NAME DISPLAY

DESCRIPTION DC_DRIVERDESC

DATA FIXED SINGLE

EXPORTS
	BitBlt              @1
	ColorInfo           @2
	Control             @3
	Disable             @4
	Enable              @5
	EnumDFonts          @6
	EnumObj             @7
	Output              @8
	Pixel               @9
	RealizeObject       @10
	StrBlt              @11
	ScanLR              @12
	DeviceMode          @13
	_dsSetPixel         @14
	_dsGetPixel         @15
#if (DC_MODIFIABLE_RES == 1)
	GetDimensions       @16
#endif

#if (DC_USES_SAVE_AREA == 1)
IMPORTS
	GetSaveAddress = CURSOR.5
#endif
