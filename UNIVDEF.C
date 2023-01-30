#include "devconst.h"
#define PHASE2 #define QUOTE(a) 'a'
PHASE2

LIBRARY DISPLAY

DESCRIPTION QUOTE(DC_DRIVERDESC)

STUB 'WINSTUB.EXE'

DATA PRELOAD FIXED SINGLE

SEGMENTS
	_TEXT   PRELOAD FIXED SHARED

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

	Inquire             @101
	SetCursor           @102
	MoveCursor          @103
	CheckCursor         @104
