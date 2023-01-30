/* WINNMETH.H - WINDOWS.H without the methods */

/* If defined, the following flags inhibit definition
   of the indicated constants.

    NOGDICAPMASKS       DT_*, CC_*, LC_*, PC_*, CP_*, TC_*, RC_
    NOVIRTUALKEYCODES   VK_*
    NOWINMESSAGES       WM_*
    NOWINSTYLES         WS_*, CS_*
    */

#ifndef PASCAL
#define PASCAL  pascal
#endif

#define FALSE   0
#define TRUE    1
#define NULL    0

#define FAR     far
#define NEAR    near
#define LONG    long
#define VOID    void

typedef unsigned char   BYTE;
typedef unsigned short  WORD;
typedef unsigned long  DWORD;
typedef int       BOOL;
typedef char     *PSTR;
typedef char FAR *LPSTR;
typedef int  FAR *LPINT;

#define max(a,b)        ((a) > (b) ? (a) : (b))
#define min(a,b)        ((a) < (b) ? (a) : (b))
#define abs(x)          ((x) < 0 ? -(x) : (x))
#define MAKELONG(a, b)  ((long)(((unsigned)a) | ((unsigned long)((unsigned)b)) << 16))
#define LOWORD(l)       ((WORD)l)
#define HIWORD(l)       ((WORD)(((DWORD)l >> 16) & 0xffff))
#define LOBYTE(w)       ((BYTE)w)
#define HIBYTE(w)       (((WORD)w >> 8) & 0xff)
#define MAKEPOINT(l)    (*((POINT *)&l))

/*  Scroll bar constants */
#define SB_HORZ             0
#define SB_VERT             1
#define SB_CTL              2

/*  Scroll Commands */
#define SB_LINEUP           0
#define SB_LINEDOWN         1
#define SB_PAGEUP           2
#define SB_PAGEDOWN         3
#define SB_THUMBPOSITION    4
#define SB_THUMBTRACK       5
#define SB_TOP              6
#define SB_BOTTOM           7

/* ShowWindow commands */
#define HIDE_WINDOW     0
#define SHOW_OPENWINDOW 1
#define SHOW_ICONWINDOW 2

/* flags for regions */
#define ERROR           0
#define NULLREGION      1
#define SIMPLEREGION    2
#define COMPLEXREGION   3

/* styles for CombineRgn */
#define RGN_AND         1
#define RGN_OR          2
#define RGN_XOR         3
#define RGN_DIFF        4
#define RGN_COPY        5

#ifndef NOVIRTUALKEYCODES
/* Virtual key codes */
#define VK_MOUSE         0x00
#define VK_LBUTTON       0x01
#define VK_RBUTTON       0x02
#define VK_BACK          0x08
#define VK_TAB           0x09
#define VK_RETURN        0x0d
#define VK_ENTER         0x0e
#define VK_SHIFT         0x10
#define VK_CONTROL       0x11
#define VK_ALTERNATE     0x12
#define VK_CAPITAL       0x13
#define VK_INSERT        0x15
#define VK_DELETE        0x16
#define VK_CLEAR         0x17
#define VK_STOP          0x1a
#define VK_QUIT          0x1b
#define VK_ESCAPE        0x1c
#define VK_PRINT         0x1d
#define VK_HELP          0x1e
#define VK_SPACE         0x20
#define VK_PRIOR         0x27
#define VK_NEXT          0x28
#define VK_END           0x29
#define VK_HOME          0x2b
#define VK_LEFT          0x2c
#define VK_UP            0x2d
#define VK_RIGHT         0x2e
#define VK_DOWN          0x2f
#define VK_SEMICOLON     0x3b
#define VK_COMMA         0x3c
#define VK_EQUAL         0x3d
#define VK_PERIOD        0x3e
#define VK_SLASH         0x3f
#define VK_LEFTBRACKET   0x5b
#define VK_BACKSLASH     0x5c
#define VK_RIGHTBRACKET  0x5d
#define VK_MINUS         0x5e
#define VK_APOSTROPHE    0x5f
#define VK_BACKAPOSTROPHE 0x60
#define VK_F1            0x74
#define VK_F2            0x75
#define VK_F3            0x76
#define VK_F4            0x77
#define VK_F5            0x78
#define VK_F6            0x79
#define VK_F7            0x7a
#define VK_F8            0x7b
#define VK_F9            0x7c
#define VK_F10           0x7d
#define VK_F11           0x7e
#define VK_F12           0x7f
#endif

/*  Binary raster ops */
#define R2_BLACK            1               /*  0       */
#define R2_NOTMERGEPEN      2               /* DPon     */
#define R2_MASKNOTPEN       3               /* DPna     */
#define R2_NOTCOPYPEN       4               /* PN       */
#define R2_MASKPENNOT       5               /* PDna     */
#define R2_NOT              6               /* Dn       */
#define R2_XORPEN           7               /* DPx      */
#define R2_NOTMASKPEN       8               /* DPan     */
#define R2_MASKPEN          9               /* DPa      */
#define R2_NOTXORPEN       10               /* DPxn     */
#define R2_NOP             11               /* D        */
#define R2_MERGENOTPEN     12               /* DPno     */
#define R2_COPYPEN         13               /* P        */
#define R2_MERGEPENNOT     14               /* PDno     */
#define R2_MERGEPEN        15               /* DPo      */
#define R2_WHITE           16               /*  1       */

/*  Ternary raster operations */
#define SRCCOPY     (DWORD)0x00CC0020  /* dest=source                       */
#define SRCPAINT    (DWORD)0x00EE0086  /* dest=source OR dest               */
#define SRCAND      (DWORD)0x008800C6  /* dest = source AND dest             */
#define SRCINVERT   (DWORD)0x00660046  /* dest = source XOR      dest        */
#define SRCERASE    (DWORD)0x00440328  /* dest = source AND (not dest )      */
#define NOTSRCCOPY  (DWORD)0x00330008  /* dest = (not source)                */
#define NOTSRCERASE (DWORD)0x001100A6  /* dest = (not source) AND (not dest) */
#define MERGECOPY   (DWORD)0x00C000CA  /* dest = (source AND pattern)        */
#define MERGEPAINT  (DWORD)0x00BB0226  /* dest = (source AND pattern) OR dest*/
#define PATCOPY     (DWORD)0x00F00021  /* dest = pattern                     */
#define PATPAINT    (DWORD)0x00FB0A09  /* dest = DPSnoo                      */
#define PATINVERT   (DWORD)0x005A0049  /* dest = pattern XOR     dest        */
#define DSTINVERT   (DWORD)0x00550009  /* dest = (not dest)                  */
#define BLACKNESS   (DWORD)0x00000042  /* dest = BLACK                       */
#define WHITENESS   (DWORD)0x00FF0062  /* dest = WHITE                       */

/* StretchBlt() modes */
#define BLACKONWHITE    1
#define WHITEONBLACK    2
#define COLORONCOLOR    3

/* PolyFill modes */
#define ALTERNATE       1
#define WINDING         2

/* GDI escapes */
#define NEWFRAME        1
#define ABORTPIC        2
#define NEXTBAND        3
#define SETCOLORTABLE   4
#define GETCOLORTABLE   5

typedef char *HANDLE;
typedef HANDLE  *SPHANDLE;
typedef HANDLE  FAR *LPHANDLE;

typedef int (FAR PASCAL *FARPROC)();
typedef int (NEAR PASCAL *NEARPROC)();
typedef HANDLE GLOBALHANDLE;
typedef HANDLE LOCALHANDLE;

typedef struct {
    short      bmType;
    short      bmWidth;
    short      bmHeight;
    short      bmWidthBytes;
    BYTE       bmPlanes;
    BYTE       bmBitsPixel;
    char FAR * bmBits;
} BITMAP;
typedef BITMAP *PBITMAP;
typedef BITMAP FAR *LPBITMAP;

typedef HANDLE  HSTR;
typedef HANDLE  HICON;
typedef HANDLE  HDC;
typedef HANDLE  HMENU;
typedef HANDLE  HPEN;
typedef HANDLE  HFONT;
typedef HANDLE  HBRUSH;
typedef HANDLE  HBITMAP;
typedef HANDLE  HCURSOR;
typedef HANDLE  HRGN;

typedef struct {
        int x;
        int y;
} POINT;
typedef POINT *PPOINT;
typedef POINT FAR *LPPOINT;

#ifndef WIN_INTERNAL
typedef struct {
        int left;
        int top;
        int right;
        int bottom;
} RECT;
#endif
typedef RECT *PRECT;
typedef RECT FAR *LPRECT;

typedef struct {
    WORD    style;
    long    (far *lpfnWndProc)();
    int     cbClsExtra;
    int     cbWndExtra;
    HANDLE  hInstance;
    HICON   hIcon;
    HCURSOR hCursor;
    HBRUSH  hbrBackground;
    LPSTR   lpszMenuName;
    LPSTR   lpszClassName;
} WNDCLASS;
typedef WNDCLASS     *PWNDCLASS;
typedef WNDCLASS FAR *LPWNDCLASS;

#ifndef WIN_INTERNAL
typedef HANDLE HWND;
#endif

/* Message structure */
typedef struct {
    HWND hwnd;
    WORD message;
    WORD wParam;
    LONG lParam;
    DWORD time;
    POINT pt;
} MSG;
typedef MSG *PMSG;
typedef MSG FAR *LPMSG;

typedef struct {
    DWORD style;
    int ID;
    HANDLE hText;
    HWND hwndParent;
    HANDLE hInstance;
    long (far *lpfnWndProc)();
} WNDSTRUCT;
typedef WNDSTRUCT FAR *LPWNDSTRUCT;

#ifndef NOWINMESSAGES
/* ** Window Procedure Messages */

#define WM_STATE            0x0000              /* window state messages */

#define WM_NULL             0x0000
#define WM_CREATE           0x0001
#define WM_DESTROY          0x0002
#define WM_SIZE             0x0003
#define WM_SETVISIBLE       0x0004
#define WM_ENABLE           0x0005
#define WM_SETREDRAW        0x0006
#define WM_SETTEXT          0x0007
#define WM_GETTEXT          0x0008
#define WM_GETTEXTLENGTH    0x0009
#define WM_PAINT            0x000a
#define WM_GETDLGCODE       0x000b
#define WM_CLOSEBOX         0x000c
#define WM_QUIT             0x000d

#define WM_QUERYDEFPROC     0x0080
#define WM_NCSIZE           0x0081
#define WM_NCPAINT          0x0082
#define WM_NCHITTEST        0x0083
#define WM_NCMOUSEMOVE      0x0090
#define WM_NCLBUTTONDOWN    0x0091
#define WM_NCLBUTTONUP      0x0092
#define WM_NCLBUTTONDBLCLK  0x0093
#define WM_NCRBUTTONDOWN    0x0094
#define WM_NCRBUTTONUP      0x0095
#define WM_NCRBUTTONDBLCLK  0x0096
#define WM_NCCREATE         0x0097
#define WM_NCDESTROY        0x0098
#define WM_ENDDIALOG        0x0099

#define WM_INPUT            0x0100

#define WM_SETFOCUS         0x0100      /* input related stuff */
#define WM_KILLFOCUS        0x0101
#define WM_KEYBOARD         0x0102
#define WM_CHAR             0x0103
#define WM_DEADCHAR         0x0104
#define WM_COMMAND          0x0105
#define WM_TIMER            0x0106
#define WM_HSCROLL          0x0107
#define WM_VSCROLL          0x0108
#define WM_INITDIALOG       0x0109
#define WM_INITMENU         0x010a

#define WM_MOUSE            0x0200

#define WM_MOUSEMOVE        0x0200      /* mouse related stuff */
#define WM_LBUTTONDOWN      0x0201
#define WM_LBUTTONUP        0x0202
#define WM_LBUTTONDBLCLK    0x0203
#define WM_RBUTTONDOWN      0x0204
#define WM_RBUTTONUP        0x0205
#define WM_RBUTTONDBLCLK    0x0206

/* clipboard messages */
#define WM_CLIPBOARD        0x0300
#define WM_CUT              0x0300
#define WM_COPY             0x0301
#define WM_PASTE            0x0302
#define WM_CLEAR            0x0303
#define WM_UNDO             0x0304
#define WM_RENDERFORMAT     0x0305
#define WM_RENDERALLFORMATS 0x0306
#define WM_DESTROYCLIPBOARD 0x0307
#define WM_DRAWCLIPBOARD    0x0308

/* private window messages start here */
#define WM_USER             0x0400
#endif

/* Size message commands */
#define SIZENORMAL      0
#define SIZEICONIC      1
#define SIZEFULLSCREEN  2

/* Key state masks for mouse messages */
#define MK_LBUTTON      0x0001
#define MK_RBUTTON      0x0002
#define MK_SHIFT        0x0004
#define MK_ALTERNATE    0x0008
#define MK_CONTROL      0x0010

#ifndef NOWINSTYLES
/* Window styles */
#define WS_TILED        0x00000000L
#define WS_POPUP        0x80000000L
#define WS_CHILD        0x40000000L
#define WS_ICONIC       0x20000000L
#define WS_VISIBLE      0x10000000L
#define WS_DISABLED     0x08000000L
#define WS_CLIPSIBLINGS 0x04000000L
#define WS_CLIPCHILDREN 0x02000000L

#define WS_BORDER       0x00800000L
#define WS_CAPTION      0x00400000L
#define WS_VSCROLL      0x00200000L
#define WS_HSCROLL      0x00100000L
#define WS_CLOSEBOX     0x00080000L
#define WS_SIZEBOX      0x00040000L

/* Class styles */
#define CS_VREDRAW      0x0001
#define CS_HREDRAW      0x0002
#define CS_KEYCVTWINDOW 0x0004
#define CS_DBLCLKS      0x0008
#define CS_OEMCHARS     0x0010
#define CS_OWNDC        0x0020
#define CS_CLASSDC      0x0040
#define CS_MENUPOPUP    0x0080

/* Shorthand for the common cases */
#define WS_TILEDWINDOW   (WS_TILED | WS_BORDER | WS_CAPTION | WS_CLOSEBOX | WS_SIZEBOX)
#define WS_POPUPWINDOW   (WS_POPUP | WS_BORDER)
#define WS_CHILDWINDOW   (WS_CHILD)
#endif

/* predefined clipboard formats */
#define CF_TEXT         1
#define CF_BITMAP       2
#define CF_METAFILE     3
#define CF_BINARY       4
#define CF_SYLK         5
#define CF_DIF          6
#define CF_LINK         7
#define CF_DSPTEXT      0x81       /* display text */
#define CF_DSPBITMAP    0x82       /* display bitmap */
#define CF_DSPMETAFILE  0x83       /* display metafile */

/* Private clipboard format range */
#define CF_PRIVATEFIRST 0x200       /* Anything in this range doesn't */
#define CF_PRIVATELAST  0x2ff       /* get GlobalFree'd */
#define CF_GDIOBJFIRST  0x300       /* Anything in this range gets */
#define CF_GDIOBJLAST   0x3ff       /* DeleteObject'ed */

typedef struct {
    HDC hdc;
    BOOL fErase;
    RECT rcPaint;
    BOOL fRestore;
    BYTE rgbReserved[16];
} PAINTSTRUCT;
typedef PAINTSTRUCT *PPAINTSTRUCT;
typedef PAINTSTRUCT far *LPPAINTSTRUCT;

typedef struct {
    LPSTR lpCreateParams;
    HANDLE hInstance;
    HANDLE hMenu;
    HWND hwndParent;
    int cy;
    int cx;
    int y;
    int x;
    long style;
    LPSTR lpszName;
    LPSTR lpszClass;
} CREATESTRUCT;
typedef CREATESTRUCT far *LPCREATESTRUCT;

/* GDI logical objects */
/* Pel Array */
typedef struct {
    short int paXCount;
    short int paYCount;
    short int paXExt;
    short int paYExt;
    BYTE      paRGBs;
} PELARRAY;
typedef PELARRAY     *PPELARRAY;
typedef PELARRAY far *LPPELARRAY;

/* RGB Color */
typedef union {
	struct {
		BYTE r;
		BYTE g;
		BYTE b;
		BYTE special;
	} col;
	DWORD num;
} COLOR;

/* Logical Brush */
typedef struct {
    WORD lbStyle;
    COLOR lbColor;
    WORD lbHatch;
	COLOR lbBkColor;
} LOGBRUSH;
typedef LOGBRUSH        *PLOGBRUSH;
typedef LOGBRUSH    far *LPLOGBRUSH;

/* A PATTERN and a LOGBRUSH are the same thing */
typedef LOGBRUSH     PATTERN;
typedef PATTERN     *PPATTERN;
typedef PATTERN FAR *LPPATTERN;

/* Logical Pen */
typedef struct {
    WORD   lopnStyle;
    POINT  lopnWidth;
    COLOR  lopnColor;
} LOGPEN;
typedef LOGPEN      *PLOGPEN;
typedef LOGPEN  far *LPLOGPEN;

/* Logical Font */

#define LF_FACESIZE 32

typedef struct  {
    short int lfHeight;
    short int lfWidth;
    short int lfEscapement;
    short int lfOrientation;
    short int lfWeight;
    BYTE lfItalic;
    BYTE lfUnderline;
    BYTE lfStrikeOut;
    BYTE lfCharSet;
    BYTE lfOutPrecision;
    BYTE lfClipPrecision;
    BYTE lfQuality;
    BYTE lfPitch;
    BYTE lfFaceName[LF_FACESIZE];
} LOGFONT;
typedef LOGFONT     *PLOGFONT;
typedef LOGFONT far *LPLOGFONT;

/* Text Metric */
typedef struct {
    short int tmHeight;
    short int tmAscent;
    short int tmDescent;
    short int tmLeading;
    short int tmAveCharWidth;
    short int tmMaxCharWidth;
    short int tmWeight;
    BYTE      tmItalic;
    BYTE      tmUnderlined;
    BYTE      tmStruckOut;
    BYTE      tmFirstChar;
    BYTE      tmLastChar;
    BYTE      tmDefaultChar;
    BYTE      tmBreakChar;
    BYTE      tmPitch;
    BYTE      tmCharSet;
} TEXTMETRIC;
typedef TEXTMETRIC     *PTEXTMETRIC;
typedef TEXTMETRIC far *LPTEXTMETRIC;

#define RASTER_FONTTYPE 0x0001
#define DEVICE_FONTTYPE 0X0002

/* Logical font constants */

#define OUT_DEFAULT_PRECIS    0
#define OUT_STRING_PRECIS     1
#define OUT_CHARACTER_PRECIS  2
#define OUT_STROKE_PRECIS     3

#define CLIP_DEFAULT_PRECIS   0
#define CLIP_CHARACTER_PRECIS 1
#define CLIP_STROKE_PRECIS    2

#define DEFAULT_QUALITY       0
#define DRAFT_QUALITY         1
#define PROOF_QUALITY         2

#define DEFAULT_PITCH         0
#define FIXED_PITCH           1
#define VARIABLE_PITCH        2

/* GDI rgb values packed into a dword */

#define RGB(r,g,b) (((DWORD) (b << 8 | g) << 8) | r)
#define GetRValue(rgb) ((BYTE)rgb)
#define GetGValue(rgb) ((BYTE)(((WORD)rgb) >> 8))
#define GetBValue(rgb) ((BYTE)(rgb>>16))

/* GDI Background Modes */

#define TRANSPARENT    1
#define OPAQUE         2

/* GDI map modes */
#define MM_TEXT             1
#define MM_LOMETRIC         2
#define MM_HIMETRIC         3
#define MM_LOENGLISH        4
#define MM_HIENGLISH        5
#define MM_TWIPS            6
#define MM_ISOTROPIC        7
#define MM_ANISOTROPIC      8

/* GDI coordinate modes */

#define ABSOLUTE            1
#define RELATIVE            2

/* Stock Logical Objects */

#define  WHITE_BRUSH         0
#define  LTGRAY_BRUSH        1
#define  GRAY_BRUSH          2
#define  DKGRAY_BRUSH        3
#define  BLACK_BRUSH         4
#define  HOLLOW_BRUSH        5
#define  WHITE_PEN           6
#define  BLACK_PEN           7
#define  NULL_PEN            8
#define  OEM_FIXED_FONT     10
#define  ANSI_FIXED_FONT    11
#define  ANSI_VAR_FONT      12

/* GDI Brush Style definitions. */

#define BS_SOLID            0
#define BS_HOLLOW           1
#define BS_HATCHED          2
#define BS_PATTERN          3

/* GDI Hatch Style definitions. */

#define HS_HORIZONTAL       0       /* ----- */
#define HS_VERTICAL         1       /* ||||| */
#define HS_FDIAGONAL        2       /* ///// */
#define HS_BDIAGONAL        3       /* \\\\\ */
#define HS_CROSS            4       /* +++++ */
#define HS_DIAGCROSS        5       /* xxxxx */


/* Device Parameters for GetDeviceCaps() */

#define DRIVERVERSION     0     /*  Device driver version                 */
#define TECHNOLOGY        2     /*  Device classification                 */
#define HORZSIZE          4     /*  Horizontal size in millimeters        */
#define VERTSIZE          6     /*  Vertical   size in millimeters        */
#define HORZRES           8     /*  Horizontal width in pixels            */
#define VERTRES           10    /*  Vertical   width in pixels            */
#define BITSPIXEL         12    /*  Number of bits per pixel              */
#define PLANES            14    /*  Number of planes                      */
#define NUMBRUSHES        16    /*  Number of brushes the device has      */
#define NUMPENS           18    /*  Number of pens the device has         */
#define NUMMARKERS        20    /*  Number of markers the device has      */
#define NUMFONTS          22    /*  Number of fonts the device has        */
#define NUMCOLORS         24
#define PDEVICESIZE       26    /*  Size required for device descriptor   */
#define CURVECAPS         28    /*  Curves    capabilities                */
#define LINECAPS          30    /*  Line      capabilities                */
#define POLYGONALCAPS     32    /*  Polygonal capabilities                */
#define TEXTCAPS          34    /*  Text      capabilities                */
#define CLIPCAPS          36    /*  Clipping  capabilities                */
#define RASTERCAPS        38    /*  Bitblt    capabilities                */
#define ASPECTX           40    /*  Length of the X leg                   */
#define ASPECTY           42    /*  Length of the Y leg                   */
#define ASPECTXY          44    /*  Length of the hypotenuse              */

#if 0
/* Most of this breaks in C */
/* #ifndef NOGDICAPMASKS */

/* Device capability masks */
/*              Device Technologies                                */

#define DT_PLOTTER          0       /* Vector plotter          */
#define DT_RASDISPLAY       1       /* Raster display          */
#define DT_RASPRINTER       2       /* Raster printer          */
#define DT_RASCAMERA        3       /* Raster camera           */
#define DT_CHARSTREAM       4       /* Character-stream, PLP   */
#define DT_METAFILE         5       /* Metafile, VDM           */
#define DT_DISPFILE         6       /* Display-file            */

/*              Curve Capabilities                                 */

#define CC_NONE         00000000B   /* Curves not supported    */
#define CC_CIRCLES      00000001B   /* Can do circles          */
#define CC_PIE          00000010B   /* Can do pie wedges       */
#define CC_CHORD        00000100B   /* Can do chord arcs       */
#define CC_ELLIPSES     00001000B   /* Can do ellipese         */
#define CC_WIDE         00010000B   /* Can do wide lines       */
#define CC_STYLED       00100000B   /* Can do styled lines     */
#define CC_WIDESTYLED   01000000B   /* Can do wide styled lines*/
#define CC_INTERIORS    10000000B   /* Can do interiors        */

/*           Line Capabilities                                 */

#define LC_NONE         00000000B   /* Lines not supported     */
#define LC_POLYLINE     00000010B   /* Can do polylines        */
#define LC_MARKER       00000100B   /* Can do markers          */
#define LC_POLYMARKER   00001000B   /* Can do polymarkers      */
#define LC_WIDE         00010000B   /* Can do wide lines       */
#define LC_STYLED       00100000B   /* Can do styled lines     */
#define LC_WIDESTYLED   01000000B   /* Can do wide styled lines*/
#define LC_INTERIORS    10000000B   /* Can do interiors        */

/*           Polygonal Capabilities                            */

#define PC_NONE         00000000B   /* Polygonals not supported*/
#define PC_POLYGON      00000001B   /* Can do polygons         */
#define PC_RECTANGLE    00000010B   /* Can do rectangles       */
#define PC_TRAPEZOID    00000100B   /* Can do trapezoids       */
#define PC_SCANLINE     00001000B   /* Can do scanlines        */
#define PC_WIDE         00010000B   /* Can do wide borders     */
#define PC_STYLED       00100000B   /* Can do styled borders   */
#define PC_WIDESTYLED   01000000B   /* Can do wide styled borders*/
#define PC_INTERIORS    10000000B   /* Can do interiors        */

/*           Polygonal Capabilities                            */

#define CP_NONE         00000000B   /* no clipping of Output   */
#define CP_RECTANGLE    00000001B   /* Output clipped to Rects */

#endif

/*           Text Capabilities                                          */

#define TC_OP_CHARACTER 0x0001      /* Can do OutputPrecision   CHARACTER      */
#define TC_OP_STROKE    0x0002      /* Can do OutputPrecision   STROKE         */
#define TC_CP_STROKE    0x0004      /* Can do ClipPrecision     STROKE         */
#define TC_CR_90        0x0008      /* Can do CharRotAbility    90             */
#define TC_CR_ANY       0x0010      /* Can do CharRotAbility    ANY            */
#define TC_SF_X_YINDEP  0x0020      /* Can do ScaleFreedom      X_YINDEPENDENT */
#define TC_SA_DOUBLE    0x0040      /* Can do ScaleAbility      DOUBLE         */
#define TC_SA_INTEGER   0x0080      /* Can do ScaleAbility      INTEGER        */
#define TC_SA_CONTIN    0x0100      /* Can do ScaleAbility      CONTINUOUS     */
#define TC_EA_DOUBLE    0x0200      /* Can do EmboldenAbility   DOUBLE         */
#define TC_IA_ABLE      0x0400      /* Can do ItalisizeAbility  ABLE           */
#define TC_UA_ABLE      0x0800      /* Can do UnderlineAbility  ABLE           */
#define TC_SO_ABLE      0x1000      /* Can do StrikeOutAbility  ABLE           */
#define TC_RA_ABLE      0x2000      /* Can do RasterFontAble    ABLE           */
#define TC_VA_ABLE      0x4000      /* Can do VectorFontAble    ABLE           */
#define TC_RESERVED     0x8000      /* Reserved.                               */

/*           Raster Capabilities                                        */

#define RC_BITBLT       0000001     /* Can do standard non-stretching, non-inverting BLT. */

/* DrawText format flags */

#define DT_LEFT         0x00
#define DT_CENTER       0x01
#define DT_RIGHT        0x02
#define DT_TOP          0x00
#define DT_VCENTER      0x04
#define DT_BOTTOM       0x08
#define DT_WORDBREAK    0x10
#define DT_SINGLELINE   0x20
#define DT_EXPANDTABS   0x40
#define DT_TABSTOP      0x80

/* MessageBox type flags */

#define MB_OK                   0x00
#define MB_OKCANCEL             0x01
#define MB_ABORTRETRYIGNORE     0x02
#define MB_CLIPCURSOR           0x04

/* Interface to global memory manager */

#define GMEM_FIXED          0x00
#define GMEM_MOVEABLE       0x02
#define GMEM_ZEROINIT       0x40
#define GMEM_DISCARDABLE    0x0F00
#define GHND    (GMEM_MOVEABLE | GMEM_ZEROINIT)
#define GPTR    (GMEM_FIXED    | GMEM_ZEROINIT)

/* Interface to local memory manager */

#define LMEM_FIXED          0x00
#define LMEM_MOVEABLE       0x02
#define LMEM_ZEROINIT       0x40
#define LMEM_DISCARDABLE    0x0F00
#define LHND            (LMEM_MOVEABLE | LMEM_ZEROINIT)
#define LPTR            (LMEM_FIXED    | LMEM_ZEROINIT)
#define NONZEROLHND     (LMEM_MOVEABLE)
#define NONZEROLPTR     (LMEM_FIXED)

#define LNOTIFY_OUTOFMEM 0
#define LNOTIFY_MOVE     1
#define LNOTIFY_DISCARD  2

#define MAKEINTRESOURCE(i)  (LPSTR)((DWORD)((WORD)i))

/* Predefined resource types */
#define RT_CURSOR       MAKEINTRESOURCE( 1 )
#define RT_BITMAP       MAKEINTRESOURCE( 2 )
#define RT_ICON         MAKEINTRESOURCE( 3 )
#define RT_MENU         MAKEINTRESOURCE( 4 )
#define RT_DIALOG       MAKEINTRESOURCE( 5 )
#define RT_STRING       MAKEINTRESOURCE( 6 )
#define RT_FONTDIR      MAKEINTRESOURCE( 7 )
#define RT_FONT         MAKEINTRESOURCE( 8 )

/* Interface to the atom manager */

typedef WORD ATOM;

#define MAKEINTATOM(i)  (LPSTR)((DWORD)((WORD)i))

/* Interface to the user profile */

int         far PASCAL GetProfileInt( LPSTR, LPSTR, int );
int         far PASCAL GetProfileString( LPSTR, LPSTR, LPSTR, LPSTR, int );
void        far PASCAL WriteProfileString( LPSTR, LPSTR, LPSTR );

/* Menu flags for Add/Check/EnableMenuItem */
#define MF_CHANGE       0x0080
#define MF_INSERT       0x0000
#define MF_APPEND       0x0100
#define MF_DELETE       0x0200
#define MF_BYPOSITION   0x0400
#define MF_BYCOMMAND    0x0000
#define MF_GRAYED       0x0001
#define MF_DISABLED     0x0002
#define MF_ENABLED      0x0000
#define MF_CHECKED      0x0008
#define MF_BITMAP       0x0004
#define MF_STRING       0x0000
#define MF_POPUP        0x0010
#define MF_DIVIDER      0x0020
#define MF_BREAK        0x0040

#define IDC_ARROW       MAKEINTRESOURCE( 1 )
#define IDC_IBEAM       MAKEINTRESOURCE( 2 )
#define IDC_WAIT        MAKEINTRESOURCE( 3 )
#define IDC_CROSS       MAKEINTRESOURCE( 4 )
#define IDC_UPARROW     MAKEINTRESOURCE( 8 )
#define IDC_SIZE        MAKEINTRESOURCE( 9 )
#define IDC_ICON        MAKEINTRESOURCE( 10 )

#define IDI_APPLICATION MAKEINTRESOURCE( 1 )
#define IDI_NOTE        MAKEINTRESOURCE( 2 )
#define IDI_ERROR       MAKEINTRESOURCE( 3 )

#ifndef NOCTLMGR

/* Control manager structures & definitions */
/* Edit control class stuff */

/* styles */
#define ES_LEFT           0L
#define ES_CENTER         1L
#define ES_RIGHT          2L
#define ES_SINGLELINE     4L
#define ES_NOTEFOCUS      8L
#define ES_NOCHANGE       16L
#define ES_NOTECLICK      32L
#define ES_AUTOVSCROLL    64L
#define ES_AUTOHSCROLL    128L

/* notification codes */
#define EN_CHAR       0x0000     /* low byte has char code */
#define EN_SETFOCUS   0x0100
#define EN_KILLFOCUS  0x0200
#define EN_CLICK      0x0300

/* control messages: */
#define EM_GETSEL        WM_USER+0
#define EM_SETSEL        WM_USER+1
#define EM_GETRECT       WM_USER+2
#define EM_SETRECT       WM_USER+3
#define EM_SETRECTNP     WM_USER+4
#define EM_SCROLL        WM_USER+5
#define EM_LINESCROLL    WM_USER+6
#define EM_SETNOTECHARS  WM_USER+7
#define EM_GETMODIFY     WM_USER+8
#define EM_SETMODIFY     WM_USER+9
#define EM_GETLINECOUNT  WM_USER+10
#define EM_LINEINDEX     WM_USER+11
#define EM_SETHANDLE     WM_USER+12
#define EM_GETHANDLE     WM_USER+13
#define EM_GETTHUMB      WM_USER+14
#define EM_SCROLLADJUST  WM_USER+15


/* button control stuff */
#define BS_PUSHBUTTON    0L
#define BS_CHECKBOX      1L
#define BS_RADIOBUTTON   2L
#define BS_USERBUTTON    3L
#define BS_3STATE        4L
#define BS_AUTO3STATE    5L
#define BS_AUTOCHECKBOX  6L
#define BS_GROUPBTN      7L
#define BS_GROUPBOX      8L
#define BS_FRMGROUPBTN   9L
#define BS_FRMGROUPBOX   10L

/* user button notification codes */
#define BN_CLICKED       0
#define BN_PAINT         1
#define BN_HILITE        2
#define BN_UNHILITE      3
#define BN_DISABLE       4

/* control messages */
#define BM_GETCHECK     WM_USER+0
#define BM_SETCHECK     WM_USER+1
#define BM_GETSTATE     WM_USER+2
#define BM_SETSTATE     WM_USER+3

/* Static control stuff */

#define SS_LEFT       0L
#define SS_CENTER     1L
#define SS_RIGHT      2L
#define SS_ICON       3L
#define SS_BLACKRECT  4L
#define SS_GRAYRECT   5L
#define SS_WHITERECT  6L
#define SS_BLACKFRAME 7L
#define SS_GRAYFRAME  8L
#define SS_WHITEFRAME 9L
#define SS_USERITEM   10L

/* Dialog style bits */
#define DS_ABSALIGN   0x000000001L

/* Conventional dialog box command IDs */
#define IDOK     1
#define IDCANCEL 2

#ifndef NOLBOX
#define LB_CTLCODE   0L

/* Listbox control return values */
#define LB_OKAY      0
#define LB_ERR       -1
#define LB_ERRSPACE  -2
#define UNDEF       -1

/* listbox messages */
#define LB_SETCONTENT    0+WM_USER
#define LB_ADDSTRING     1+WM_USER
#define LB_INSERTSTRING  2+WM_USER
#define LB_DELETESTRING  3+WM_USER
#define LB_REPLACESTRING 4+WM_USER
#define LB_RESETCONTENT  5+WM_USER
#define LB_SETSEL        6+WM_USER
#define LB_SETCURSEL     7+WM_USER
#define LB_GETSEL        8+WM_USER
#define LB_GETCURSEL     9+WM_USER
#define LB_GETTEXT       10+WM_USER
#define LB_GETTEXTLEN    11+WM_USER
#define LB_GETCOUNT      12+WM_USER
#define LB_SELECTSTRING  13+WM_USER
#define LB_DIR           14+WM_USER
#define LB_MSGMAX        15+WM_USER

/* listbox style bits */
#define LBS_NOTIFY        0x0001L
#define LBS_SORT          0x0002L
#define LBS_NOREDRAW      0x0004L
#define LBS_MULTIPLESEL   0x0008L
#define LBS_STANDARD      (LBS_NOTIFY | LBS_SORT | WS_VSCROLL | WS_BORDER)

#endif

/* scroll bar styles */
#define SBS_HORZ        0x0000L
#define SBS_VERT        0x0001L
#define SBS_TOPALIGN    0x0002L
#define SBS_LEFTALIGN   0x0002L
#define SBS_BOTTOMALIGN 0x0004L
#define SBS_RIGHTALIGN  0x0004L

#endif


/* display driver-specific definitions */

typedef struct {
	WORD dpVersion;
	WORD dpTechnology;
	WORD dpHorzSize;
	WORD dpVertSize;
	WORD dpHorzRes;
	WORD dpVertRes;
	WORD dpBitsPixel;
	WORD dpPlanes;
	WORD dpNumBrushes;
	WORD dpNumPens;
	WORD futureuse;
	WORD dpNumFonts;
#if (OS >= W1_ALPHA)
	WORD dpNumColors;
#endif
	WORD dpDEVICEsize;
	WORD dpCurves;
	WORD dpLines;
	WORD dpPolygonals;
	WORD dpText;
	WORD dpClip;
	WORD dpRaster;
	WORD dpAspectX;
	WORD dpAspectY;
	WORD dpAspectXY;
#if (OS >= W1_ALPHA)
	WORD dpStyleLen;
#endif
	POINT dpMLoWin;
	POINT dpMLoVpt;
	POINT dpMHiWin;
	POINT dpMHiVpt;
	POINT dpELoWin;
	POINT dpELoVpt;
	POINT dpEHiWin;
	POINT dpEHiVpt;
	POINT dpTwpWin;
	POINT dpTwpVpt;
#if (OS >= W1_BETA)
	WORD dpLogPixelsX;
	WORD dpLogPixelsY;
	WORD dpDCManage;
	WORD futureuse3;
	WORD futureuse4;
	WORD futureuse5;
	WORD futureuse6;
	WORD futureuse7;
#endif
} GDIINFO;

typedef struct {
	BITMAP bm;
	DWORD bmWidthPlanes;
	DWORD bmlpPDevice;
	WORD bmSegmentIndex;
	WORD bmScanSegment;
	WORD bmFillBytes;
	WORD futureUse4;
	WORD futureUse5;
} DEVBITMAP;

typedef struct {
	WORD dpXRate;
	WORD dpYRate;
#if (OS <= W1_DR5)
	WORD unknown1;
	WORD unknown2;
	WORD unknown3;
	WORD unknown4;
	WORD unknown5;
	WORD unknown6;
#endif
} CURSORINFO;

typedef struct {
	WORD csHotX;
	WORD csHotY;
	WORD csWidth;
	WORD csHeight;
	WORD csWidthBytes;
	WORD csColor;
} CURSORSHAPE;

/* Hack to keep the compiler from complaining about unknown-size array */
typedef struct {
	CURSORSHAPE cur;
	BYTE csBits[1];
} FAR *LPCURSORSHAPE;

typedef struct {
	COLOR color;
	WORD style;
} OEM_PEN;

typedef struct {
	BYTE b0;
	BYTE b1;
	BYTE b2;
	BYTE b3;
	BYTE b4;
	BYTE b5;
	BYTE b6;
	BYTE b7;
} EIGHT_BYTES;

typedef union {
	EIGHT_BYTES cpy; /* To allow copying in one fell swoop */
	BYTE arr[8]; /* To allow index-based access */
} BR_PATTERN;

typedef struct {
	COLOR colors[2];
	BR_PATTERN pat;
	WORD style;
} OEM_BRUSH;

typedef struct {
	WORD ftHeight;
	WORD ftWidth;
	WORD ftEscapement;
	WORD ftOrientation;
	WORD ftWeight;
	BYTE ftItalic;
	BYTE ftUnderline;
	BYTE ftStrikeOut;
	BYTE ftOutPrecision;
	BYTE ftClipPrecision;
	WORD ftAccelerator;
	WORD ftOverhang;
} TEXTXFORM;

typedef struct {
	WORD Rop2;
	WORD bkMode;
	COLOR bkColor;
	COLOR TextColor;
	WORD TBreakExtra;
	WORD BreakExtra;
	WORD BreakErr;
	WORD BreakRem;
	WORD BreakCount;
	WORD CharExtra;
	COLOR LbkColor;
	COLOR LTextColor;
} DRAWMODE;

typedef struct {
	WORD fsType;
	WORD fsPoints;
	WORD fsVertRes;
	WORD fsHorizRes;
	WORD fsAscent;
#if (OS >= W1_BETA)
	WORD fsInternalLeading;
#endif
	WORD fsExternalLeading;
	BYTE fsItalic;
	BYTE fsUnderline;
	BYTE fsStrikeOut;
	WORD fsWeight;
	BYTE fsCharSet;
	WORD fsPixWidth;
	WORD fsPixHeight;
	BYTE fsPitchAndFamily;
	WORD fsAvgWidth;
	WORD fsMaxWidth;
	BYTE fsFirstChar;
	BYTE fsLastChar;
	BYTE fsDefaultChar;
	BYTE fsBreakChar;
	WORD fsWidthBytes;
	DWORD fsDevice;
	DWORD fsFace;
	DWORD fsUnknown;
	WORD fsBitsOffset;
	WORD fsMaybeBitsSegment; /* Can be 0. Don't use. */
	WORD propOffsets[1]; /* Arbitrary size to make the compiler happy */
} FONTSEG;
