#include "os.h"
#include "winnmeth.h"
#include "devconst.h"
#include "devspec.h"
#include "execrop.h"
#include "misc.h"

#if (OS <= W1_DR5)
/* Older versions exclude the cursor already, we don't need to do it */
#define Exclude(a, b, c, d)
#define Unexclude()
#else
void NEAR PASCAL Exclude(WORD, WORD, WORD, WORD);
void NEAR PASCAL Unexclude(void);
#endif

#if (DC_USES_SAVE_AREA == 1)
LPSTR saveAddr;
#if (OS <= W1_DR5)
/*	Cursor is separate, so use a method to obtain the pointer */
LPSTR FAR PASCAL GetSaveAddress(void);
#else
extern COLOR temp_buffer[];
#endif
#endif

#if (DC_WIDTH >= 240)
#define WIDTH_MM 240
#else
/* To accomodate extremely low-res modes */
#define WIDTH_MM 40
#endif
#if (DC_WIDTH / 2 >= DC_HEIGHT)
#define HEIGHT_MM ((WIDTH_MM * DC_HEIGHT / DC_WIDTH) / 2)
#define DPI_Y 48
#else
#define HEIGHT_MM (WIDTH_MM * DC_HEIGHT / DC_WIDTH)
#define DPI_Y 96
#endif
#define LC_POLYLINE 2
#define PC_SCANLINE 8
GDIINFO gdiInfo = {
	/* dpVersion = */ 0x0100,
	/* dpTechnology = */ 1,
	/* dpHorzSize = */ WIDTH_MM,
	/* dpVertSize = */ HEIGHT_MM,
	/* dpHorzRes = */ DC_WIDTH,
	/* dpVertRes = */ DC_HEIGHT,
	/* dpBitsPixel = */ DC_BITS,
	/* dpPlanes = */ 1,
	/* dpNumBrushes = */ -1,
	/* dpNumPens = */ 1 * (1 << DC_BITS),
	/* futureuse = */ 0,
	/* dpNumFonts = */ 0,
#if (OS >= W1_ALPHA)
	/* dpNumColors = */ 1 << DC_BITS,
#endif
	/* dpDEVICEsize = */ sizeof(DEVBITMAP),
	/* dpCurves = */ 0,
	/* dpLines = */ LC_POLYLINE,
	/* dpPolygonals = */ PC_SCANLINE,
	/* dpText = */ TC_RA_ABLE | TC_CP_STROKE,
	/* dpClip = */ 0,
	/* dpRaster = */ RC_BITBLT,
	/* dpAspectX = */ 36,
	/* dpAspectY = */ 36,
	/* dpAspectXY = */ 51,
#if (OS >= W1_ALPHA)
	/* dpStyleLen = */ 102,
#endif
	/* dpMLoWin = */ {10 * WIDTH_MM, 10 * HEIGHT_MM},
	/* dpMLoVpt = */ {DC_WIDTH, -DC_HEIGHT},
	/* dpMHiWin = */ {100 * WIDTH_MM, 100 * HEIGHT_MM},
	/* dpMHiVpt = */ {DC_WIDTH, -DC_HEIGHT},
	/* dpELoWin = */ {WIDTH_MM * 1000 / 640, HEIGHT_MM * 1000 / 700},
	/* dpELoVpt = */ {DC_WIDTH * 254 / 640, -DC_HEIGHT * 254 / 700},
	/* dpEHiWin = */ {WIDTH_MM * 10000 / 640, HEIGHT_MM * 10000 / 700},
	/* dpEHiVpt = */ {DC_WIDTH * 254 / 640, -DC_HEIGHT * 254 / 700},
	/* dpTwpWin = */ {WIDTH_MM * 14400 / 640, HEIGHT_MM * 14400 / 700},
	/* dpTwpVpt = */ {DC_WIDTH * 254 / 640, -DC_HEIGHT * 254 / 700}
#if (OS >= W1_BETA)
	,
	/* dpLogPixelsX = */ 96,
	/* dpLogPixelsY = */ DPI_Y,
	/* dpDCManage = */ 4,
	/* futureuse3 = */ 0,
	/* futureuse4 = */ 0,
	/* futureuse5 = */ 0,
	/* futureuse6 = */ 0,
	/* futureuse7 = */ 0
#endif
};

#define PHYS_DEV_ID 0xBEEF
DEVBITMAP deviceInfo = {
	{
		/* bm.bmType = */ PHYS_DEV_ID,
		/* bm.bmWidth = */ DC_WIDTH,
		/* bm.bmHeight = */ DC_HEIGHT,
		/* bm.bmWidthBytes = */ DC_WIDTH * DC_BITS / 8,
		/* bm.bmPlanes = */ 1,
		/* bm.bmBitsPixel = */ DC_BITS,
		/* bm.bmBits = */ (char FAR *)NULL
	},
	/* bmWidthPlanes = */ DC_HEIGHT * DC_WIDTH * DC_BITS / 8,
	/* bmlpPDevice = */ NULL,
	/* bmSegmentIndex = */ 0,
	/* bmScanSegment = */ 0,
	/* bmFillBytes = */ 0,
	/* futureUse4 = */ 0,
	/* futureUse5 = */ 0
};

#if (DC_MODIFIABLE_RES == 1)
#define DR_WIDTH 0
#define DR_HEIGHT 1

int devRes[2];
BOOL enableCalled = FALSE;
char prfFileName[] = "DISPLAY.INI";
char prfTemplate1[] = "[display]\r\n\twidth = ";
char prfTemplate2[] = "\r\n\theight = ";
char prfTemplate3[] = "\r\n";
char prfDelims[] = "\r\n\t= ";
char prfFileBuffer[16];

WORD NEAR PASCAL toString(WORD, LPSTR);
WORD NEAR PASCAL toInt(LPSTR);
BOOL NEAR PASCAL NextToken(int, int, LPSTR, LPSTR, BOOL);
BOOL NEAR PASCAL Str1nEqual(LPSTR, int, LPSTR);
void NEAR PASCAL GetProfileRes(LPINT);
void NEAR PASCAL SetProfileRes(LPINT);

WORD NEAR PASCAL toString(num, buf)
WORD num;
LPSTR buf;
{
	WORD tempNum;
	WORD numChars;

	numChars = 0;
	tempNum = num;
	do {
		buf++;
		numChars++;
		tempNum /= 10;
	} while(tempNum != 0);
	*buf = 0;
	do {
		buf--;
		*buf = num % 10 + '0';
		num /= 10;
	} while(num != 0);
	return numChars;
}

WORD NEAR PASCAL toInt(str)
LPSTR str;
{
	WORD output;
	WORD i;

	output = 0;
	for(i = 0; str[i] >= '0' && str[i] <= '9'; i++) {
		output *= 10;
		output += str[i] - '0';
	}
	return output;
}

char tokenBuffer[16];
int bytesTooMany = 0;
int tooManyEnd = 0;
BOOL NEAR PASCAL NextToken(handle, maxLength, buffer, delims, newLine)
int handle;
int maxLength;
LPSTR buffer;
LPSTR delims;
BOOL newLine;
{
	int bytesRead;
	int i;
	int j;
	int k;
	BOOL hitToken;
	BOOL hitNewLine;
	BOOL ret;

	hitNewLine = FALSE;
	do {
		if(!bytesTooMany) {
			bytesRead = msReadFile(handle, sizeof(tokenBuffer), tokenBuffer);
			i = 0;
		} else {
			bytesRead = tooManyEnd;
			i = tooManyEnd - bytesTooMany;
			bytesTooMany = 0;
		}
		if(!bytesRead) {
			return FALSE;
		}
		for(; i < bytesRead; i++) {
			hitToken = TRUE;
			if(!hitNewLine && (tokenBuffer[i] == '\r' || tokenBuffer[i] == '\n')) {
				if(!newLine) {
					ret = FALSE;
					goto exitToken;
				}
				hitNewLine = TRUE;
			}
			for(j = 0; delims[j] != 0; j++) {
				if(tokenBuffer[i] == delims[j]) {
					hitToken = FALSE;
					break;
				}
			}
			if(hitToken && !(newLine && !hitNewLine)) {
				break;
			}
		}
	} while(!hitToken || (newLine && !hitNewLine));
	k = 0;
	do {
		if(i == bytesRead) {
			bytesRead = msReadFile(handle, sizeof(tokenBuffer), tokenBuffer);
			i = 0;
		}
		if(!bytesRead) {
			return TRUE;
		}
		hitToken = TRUE;
		for(; i < bytesRead; i++) {
			for(j = 0; delims[j] != 0; j++) {
				if(tokenBuffer[i] == delims[j]) {
					hitToken = FALSE;
					break;
				}
			}
			if(hitToken && k < maxLength - 1) {
				buffer[k++] = tokenBuffer[i];
			} else {
				hitToken = FALSE;
				break;
			}
		}
	} while(hitToken);
	buffer[k] = 0;
	ret = TRUE;

	exitToken:
	bytesTooMany = bytesRead - i;
	tooManyEnd = bytesRead;
	return ret;
}

BOOL NEAR PASCAL Str1nEqual(str1, len1, str2)
LPSTR str1;
int len1;
LPSTR str2;
{
	int i;

	for(i = 0; i < len1; i++) {
		if(str1[i] != str2[i]) {
			return FALSE;
		}
	}
	return !str2[i];
}

void NEAR PASCAL GetProfileRes(res)
LPINT res;
{
	BOOL dispFound;
	int prfHandle;
	int resIndex;

	prfHandle = msOpenFile(prfFileName);
	if(prfHandle < 0) {
		return;
	}
	if(!NextToken(prfHandle, sizeof(prfFileBuffer), prfFileBuffer, prfDelims, FALSE)
			|| !Str1nEqual(prfTemplate1, sizeof("[display]") - 1, prfFileBuffer)) {
		dispFound = FALSE;
		while(NextToken(prfHandle, sizeof(prfFileBuffer), prfFileBuffer, prfDelims, TRUE)) {
			if(Str1nEqual(prfTemplate1, sizeof("[display]") - 1, prfFileBuffer)) {
				dispFound = TRUE;
				break;
			}
		}
		if(!dispFound) {
			goto fail;
		}
	}
	while(NextToken(prfHandle, sizeof(prfFileBuffer), prfFileBuffer, prfDelims, TRUE)) {
		if(Str1nEqual(prfTemplate1 + sizeof("[display]\r\n\t") - 1, sizeof("width") - 1,
				prfFileBuffer)) {
			resIndex = DR_WIDTH;
		} else if(Str1nEqual(prfTemplate2 + sizeof("\r\n\t") - 1, sizeof("height") - 1,
				prfFileBuffer)) {
			resIndex = DR_HEIGHT;
		} else {
			continue;
		}
		if(!NextToken(prfHandle, sizeof(prfFileBuffer), prfFileBuffer, prfDelims, FALSE)) {
			continue;
		}
		res[resIndex] = toInt(prfFileBuffer);
	}

	fail:
	msCloseFile(prfHandle);
}

void NEAR PASCAL SetProfileRes(res)
LPINT res;
{
	int prfHandle;
	BOOL writeFailed;
	char buf[6]; /* Max WORD is 5 chars plus null byte */

	prfHandle = msCreateFile(prfFileName);
	if(prfHandle < 0) {
		return;
	}
	writeFailed = msWriteFile(prfHandle, sizeof(prfTemplate1) - 1, prfTemplate1)
			|| msWriteFile(prfHandle, toString(res[DR_WIDTH], buf), buf)
			|| msWriteFile(prfHandle, sizeof(prfTemplate2) - 1, prfTemplate2)
			|| msWriteFile(prfHandle, toString(res[DR_HEIGHT], buf), buf)
			|| msWriteFile(prfHandle, sizeof(prfTemplate3) - 1, prfTemplate3);
	msCloseFile(prfHandle);
	if(writeFailed) {
		msDeleteFile(prfFileName);
	}
}
#endif

/* MAKEBITS(num): represents a constant with num rightmost bits turned on */
#define MAKEBITS(num) ((1 << (num)) - 1)
#define MAGICMUL_T (0xFFFF / MAKEBITS(DC_BITS))

/* RGB Color */
#if (DC_TYPE == 0)

/* BITS_*: number of bits in each color channel */
#define BITS_R ((DC_BITS + 1) / 3)
#define BITS_G ((DC_BITS + 2) / 3)
#define BITS_B (DC_BITS / 3)

/* MAGICMUL_*: constants that when multiplied by a number, produce a repeating
   bit pattern of the width of the color channel */
#define MAGICMUL_R (0xFFFF / MAKEBITS(BITS_R))
#define MAGICMUL_G (0xFFFF / MAKEBITS(BITS_G))
#define MAGICMUL_B (0xFFFF / MAKEBITS(BITS_B))

DWORD NEAR PASCAL ColorToBits(colorIn)
COLOR colorIn;
{
	DWORD colorOut;

	colorOut = (DWORD)colorIn.col.r >> 8 - BITS_R << BITS_G + BITS_B;
	colorOut |= (DWORD)colorIn.col.g >> 8 - BITS_G << BITS_B;
	colorOut |= (DWORD)colorIn.col.b >> 8 - BITS_B;
	return colorOut;
}

DWORD NEAR PASCAL BitsToColor(colorIn)
DWORD colorIn;
{
	COLOR colorOut;

	colorOut.col.r = HIBYTE(((WORD)(colorIn >> BITS_G + BITS_B & MAKEBITS(BITS_R)) * MAGICMUL_R));
	colorOut.col.g = HIBYTE(((WORD)(colorIn >> BITS_B & MAKEBITS(BITS_G)) * MAGICMUL_G));
	colorOut.col.b = HIBYTE(((WORD)(colorIn & MAKEBITS(BITS_B)) * MAGICMUL_B));
	colorOut.col.special = 0;
	return colorOut.num;
}

DWORD NEAR PASCAL GetNearestColor(colorIn, secondNearest)
COLOR colorIn;
COLOR FAR *secondNearest;
{
	COLOR nearest;

	nearest.col.r = colorIn.col.r >> 8 - BITS_R;
	nearest.col.g = colorIn.col.g >> 8 - BITS_G;
	nearest.col.b = colorIn.col.b >> 8 - BITS_B;
	nearest.col.special = 0;

	if(secondNearest)
		secondNearest->num = nearest.num;

	nearest.col.r = HIBYTE(((WORD)nearest.col.r * MAGICMUL_R));
	nearest.col.g = HIBYTE(((WORD)nearest.col.g * MAGICMUL_G));
	nearest.col.b = HIBYTE(((WORD)nearest.col.b * MAGICMUL_B));

	if(secondNearest) {
		if(nearest.col.r < colorIn.col.r)
			secondNearest->col.r++;
		else if(nearest.col.r > colorIn.col.r)
			secondNearest->col.r--;

		if(nearest.col.g < colorIn.col.g)
			secondNearest->col.g++;
		else if(nearest.col.g > colorIn.col.g)
			secondNearest->col.g--;

		if(nearest.col.b < colorIn.col.b)
			secondNearest->col.b++;
		else if(nearest.col.b > colorIn.col.b)
			secondNearest->col.b--;

		secondNearest->col.r = HIBYTE(((WORD)secondNearest->col.r * MAGICMUL_R));
		secondNearest->col.g = HIBYTE(((WORD)secondNearest->col.g * MAGICMUL_G));
		secondNearest->col.b = HIBYTE(((WORD)secondNearest->col.b * MAGICMUL_B));
	}

	return nearest.num;
}

/* Grayscale */
#elif (DC_TYPE == 1)

#if(DC_BITS != 1)
DWORD NEAR PASCAL ColorToBits(colorIn)
COLOR colorIn;
{
	DWORD colorOut;

	colorOut = (DWORD)colorIn.col.r >> 8 - DC_BITS;
	return colorOut;
}

DWORD NEAR PASCAL BitsToColor(colorIn)
DWORD colorIn;
{
	COLOR colorOut;

	colorOut.col.r = HIBYTE(((WORD)(colorIn & MAKEBITS(DC_BITS)) * MAGICMUL_T));
	colorOut.col.g = colorOut.col.r;
	colorOut.col.b = colorOut.col.r;
	colorOut.col.special = 0;
	return colorOut.num;
}
#endif

DWORD NEAR PASCAL GetNearestColor(colorIn, secondNearest)
COLOR colorIn;
COLOR FAR *secondNearest;
{
	BYTE colorAvg;
	COLOR nearest;

	colorAvg = ((WORD)colorIn.col.r + colorIn.col.g + colorIn.col.b) / 3;

	nearest.col.r = colorAvg >> 8 - DC_BITS;

	if(secondNearest)
		secondNearest->col.r = nearest.col.r;

	nearest.col.r = HIBYTE(((WORD)nearest.col.r * MAGICMUL_T));
	nearest.col.g = nearest.col.r;
	nearest.col.b = nearest.col.r;
	nearest.col.special = 0;

	if(secondNearest) {
		if(nearest.col.r < colorIn.col.r)
			secondNearest->col.r++;
		else if(nearest.col.r > colorIn.col.r)
			secondNearest->col.r--;

		secondNearest->col.r = HIBYTE(((WORD)secondNearest->col.r * MAGICMUL_T));
		secondNearest->col.g = secondNearest->col.r;
		secondNearest->col.b = secondNearest->col.r;
	}

	return nearest.num;
}

/* Paletted */
#elif (DC_TYPE == 2)

COLOR palette[1 << DC_BITS] = DC_PALETTE;

DWORD NEAR PASCAL GetNearestEntry(colorIn, secondNearest)
COLOR colorIn;
DWORD FAR *secondNearest;
{
	DWORD colorOut;
	DWORD closestColor;
	COLOR currColor;
	WORD currDiff;
	WORD closestDiff;
	WORD secondDiff;
	BOOL getSecond;

	getSecond = secondNearest ? TRUE : FALSE;
	closestDiff = 65535;
	if(getSecond) {
		secondDiff = 65535;
	}
	for(colorOut = 0; colorOut < sizeof(palette) / sizeof(COLOR) && closestDiff != 0; colorOut++) {
		currColor = palette[colorOut];
		currDiff = (WORD)abs(currColor.col.r - colorIn.col.r) +
				(WORD)abs(currColor.col.g - colorIn.col.g) +
				(WORD)abs(currColor.col.b - colorIn.col.b);
		if(currDiff < closestDiff) {
			if(getSecond) {
				secondDiff = closestDiff;
				*secondNearest = closestColor;
			}
			closestDiff = currDiff;
			closestColor = colorOut;
		} else if(getSecond && currDiff < secondDiff) {
			secondDiff = currDiff;
			*secondNearest = colorOut;
		}
	}
	if(getSecond && closestDiff == 0) {
		*secondNearest = closestColor;
	}
	return closestColor;
}

DWORD NEAR PASCAL ColorToBits(colorIn)
COLOR colorIn;
{
	return GetNearestEntry(colorIn, NULL);
}

DWORD NEAR PASCAL BitsToColor(colorIn)
DWORD colorIn;
{
	return palette[colorIn & MAKEBITS(DC_BITS)].num;
}

DWORD NEAR PASCAL GetNearestColor(colorIn, secondNearest)
COLOR colorIn;
COLOR FAR *secondNearest;
{
	colorIn = palette[GetNearestEntry(colorIn, secondNearest)];
	*secondNearest = palette[(*secondNearest).num];
	return colorIn.num;
}

#endif

BYTE ditherArray[] = {0, 32, 8, 40, 2, 34, 10, 42,
		48, 16, 56, 24, 50, 18, 58, 26,
		12, 44, 4, 36, 14, 46, 6, 38,
		60, 28, 52, 20, 62, 30, 54, 22,
		3, 35, 11, 43, 1, 33, 9, 41,
		51, 19, 59, 27, 49, 17, 57, 25,
		15, 47, 7, 39, 13, 45, 5, 37,
		63, 31, 55, 23, 61, 29, 53, 21};
EIGHT_BYTES quarterGray1 = {0x88, 0x22, 0x88, 0x22, 0x88, 0x22, 0x88, 0x22};
EIGHT_BYTES quarterGray2 = {0xDD, 0x77, 0xDD, 0x77, 0xDD, 0x77, 0xDD, 0x77};
void NEAR PASCAL Dither(BR_PATTERN FAR *, COLOR, COLOR, COLOR);
void NEAR PASCAL Dither(outputBits, actualColor, color0, color1)
BR_PATTERN FAR *outputBits;
COLOR actualColor;
COLOR color0;
COLOR color1;
{
	int components;
	int portion1;
	int i;

	components = 0;
	portion1 = 0;
	if(color1.col.r != color0.col.r - 1 && color1.col.r != color0.col.r) {
		if(color1.col.r < color0.col.r) {
			portion1 += 64 - ((int)(actualColor.col.r - color1.col.r - 1) << 6) /
					(int)(color0.col.r - color1.col.r - 1);
		} else {
			portion1 += ((int)(actualColor.col.r - color0.col.r - 1) << 6) /
					(int)(color1.col.r - color0.col.r - 1);
		}
		components++;
	}
	if(color1.col.g != color0.col.g - 1 && color1.col.g != color0.col.g) {
		if(color1.col.g < color0.col.g) {
			portion1 += 64 - ((int)(actualColor.col.g - color1.col.g - 1) << 6) /
					(int)(color0.col.g - color1.col.g - 1);
		} else {
			portion1 += ((int)(actualColor.col.g - color0.col.g - 1) << 6) /
					(int)(color1.col.g - color0.col.g - 1);
		}
		components++;
	}
	if(color1.col.b != color0.col.b - 1 && color1.col.b != color0.col.b) {
		if(color1.col.b < color0.col.b) {
			portion1 += 64 - ((int)(actualColor.col.b - color1.col.b - 1) << 6) /
					(int)(color0.col.b - color1.col.b - 1);
		} else {
			portion1 += ((int)(actualColor.col.b - color0.col.b - 1) << 6) /
					(int)(color1.col.b - color0.col.b - 1);
		}
		components++;
	}

	if(components)
		portion1 /= components;

	switch(portion1) {
		case 15:
		case 16:
		case 17:
			outputBits->cpy = quarterGray1;
			return;
		case 47:
		case 48:
		case 49:
			outputBits->cpy = quarterGray2;
			return;
	}

	if(portion1 < 0) {
		portion1 = 0;
	} else if(portion1 > 64) {
		portion1 = 64;
	}

	for(i = 0; i < 8; i++) {
		outputBits->arr[i] = 0;
	}

	for(i = 0; i < 64; i++) {
		outputBits->arr[i >> 3] <<= 1;
		if(portion1 > ditherArray[i]) {
			outputBits->arr[i >> 3] |= 1;
		}
	}
}


typedef union {
	struct {
		OEM_BRUSH FAR *lpPBrush;
		WORD x;
		WORD y;
		WORD xOrig;
		WORD xLimit;
		char xInc;
		char yInc;
	} xy;
	struct {
		LPSTR offset;
		LPSTR rowOffset;
		WORD x;
		WORD xLimit;
		WORD rowNumBytes;
		BYTE shift;
		BYTE rowShift;
	} off;
} FETCHDATA;
typedef FETCHDATA FAR *LPFETCHDATA;

DWORD NEAR PASCAL FetchNothing(data)
LPFETCHDATA data;
{
	return 0;
}

DWORD NEAR PASCAL FetchDisplay(data)
LPFETCHDATA data;
{
	DWORD output;

	output = dsGetPixel(data->xy.x, data->xy.y);
	data->xy.x += data->xy.xInc;
	if(data->xy.x == data->xy.xLimit) {
		data->xy.x = data->xy.xOrig;
		data->xy.y += data->xy.yInc;
	}

	return output;
}

DWORD NEAR PASCAL FetchMonoBitmap(data)
LPFETCHDATA data;
{
	DWORD output;

	output = *data->off.offset << data->off.shift & 0x80 ? 0xFFFFFF : 0;
	if(++data->off.shift & 8) {
		data->off.offset++;
		data->off.shift = 0;
	}
	if(++data->off.x == data->off.xLimit) {
		data->off.x = 0;
		data->off.rowOffset += data->off.rowNumBytes;
		data->off.offset = data->off.rowOffset;
		data->off.shift = data->off.rowShift;
	}

	return output;
}

#if (DC_BITS != 1)
DWORD NEAR PASCAL FetchColorBitmap(data)
LPFETCHDATA data;
{
	DWORD output;

	output = BitsToColor(*(DWORD FAR *)data->off.offset >> data->off.shift);
	data->off.shift += DC_BITS;
	if(data->off.shift >= 8) {
		data->off.offset += data->off.shift >> 3;
		data->off.shift &= 7;
	}
	if(++data->off.x == data->off.xLimit) {
		data->off.x = 0;
		data->off.rowOffset += data->off.rowNumBytes;
		data->off.offset = data->off.rowOffset;
		data->off.shift = data->off.rowShift;
	}

	return output;
}
#endif

DWORD NEAR PASCAL FetchPattern(data)
LPFETCHDATA data;
{
	COLOR output;

	output = data->xy.lpPBrush->colors[data->xy.lpPBrush->pat.arr[data->xy.y & 7] >>
			7 - (data->xy.x & 7) & 1];
	data->xy.x += data->xy.xInc;
	if(data->xy.x == data->xy.xLimit) {
		data->xy.x = data->xy.xOrig;
		data->xy.y += data->xy.yInc;
	}

	return output.num;
}

void NEAR PASCAL PutDisplay(data, color)
LPFETCHDATA data;
COLOR color;
{
	dsSetPixel(data->xy.x, data->xy.y, color.num);
	data->xy.x += data->xy.xInc;
	if(data->xy.x == data->xy.xLimit) {
		data->xy.x = data->xy.xOrig;
		data->xy.y += data->xy.yInc;
	}
}

void NEAR PASCAL PutMonoBitmap(data, color)
LPFETCHDATA data;
COLOR color;
{
	*data->off.offset &= ~(0x80 >> data->off.shift);
	if((WORD)color.col.r + color.col.g + color.col.b & 0x200) {
		*data->off.offset |= 0x80 >> data->off.shift;
	}
	if(++data->off.shift & 8) {
		data->off.offset++;
		data->off.shift = 0;
	}
	if(++data->off.x == data->off.xLimit) {
		data->off.x = 0;
		data->off.rowOffset += data->off.rowNumBytes;
		data->off.offset = data->off.rowOffset;
		data->off.shift = data->off.rowShift;
	}
}

#if (DC_BITS != 1)
void NEAR PASCAL PutColorBitmap(data, color)
LPFETCHDATA data;
COLOR color;
{
	*(DWORD FAR *)data->off.offset &= ~((DWORD)MAKEBITS(DC_BITS) << data->off.shift);
	*(DWORD FAR *)data->off.offset |= ColorToBits(color.num) << data->off.shift;
	data->off.shift += DC_BITS;
	if(data->off.shift >= 8) {
		data->off.offset += data->off.shift >> 3;
		data->off.shift &= 7;
	}
	if(++data->off.x == data->off.xLimit) {
		data->off.x = 0;
		data->off.rowOffset += data->off.rowNumBytes;
		data->off.offset = data->off.rowOffset;
		data->off.shift = data->off.rowShift;
	}
}
#endif

#define SOURCE_PRESENT 0x01
#define PATTERN_PRESENT 0x02
#define DISP_REVERSE_X 0x04
#define DISP_REVERSE_Y 0x08
#define EXCLUDE_CALLED 0x10
#if (DC_HAS_DSPATCOPY == 1)
OEM_BRUSH patAllZero = {{0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, 0};
#endif
WORD FAR PASCAL BitBlt(lpDestDev, DestxOrg, DestyOrg, lpSrcDev, SrcxOrg, SrcyOrg, xExt, yExt, Rop,
#if (OS >= W1_BETA)
		lpPBrush, lpDrawMode)
#else
		lpPBrush)
#endif
DEVBITMAP FAR *lpDestDev;
WORD DestxOrg;
WORD DestyOrg;
DEVBITMAP FAR *lpSrcDev;
WORD SrcxOrg;
WORD SrcyOrg;
WORD xExt;
WORD yExt;
DWORD Rop;
OEM_BRUSH FAR *lpPBrush;
#if (OS >= W1_BETA)
DRAWMODE FAR *lpDrawMode;
#endif
{
	FETCHDATA srcData;
	FETCHDATA destData;
	FETCHDATA destPutData;
	FETCHDATA patData;
	DWORD (NEAR PASCAL *srcFetch)(LPFETCHDATA);
	DWORD (NEAR PASCAL *destFetch)(LPFETCHDATA);
	DWORD (NEAR PASCAL *patFetch)(LPFETCHDATA);
	void (NEAR PASCAL *destPut)(LPFETCHDATA, COLOR);
	COLOR currSrcPixel;
	COLOR currDestPixel;
	COLOR currPatPixel;
	BYTE flags;
	DWORD i;
	DWORD iEnd;

#if (DC_HAS_DSPATCOPY == 1)
	if(lpDestDev->bm.bmType == PHYS_DEV_ID) {
		Exclude(DestxOrg, DestyOrg, xExt, yExt);
		switch(Rop >> 16) {
			case 0x00:
				patAllZero.colors[0].num = 0;
				dsPatCopy(DestxOrg, DestyOrg, xExt, yExt, &patAllZero);
				Unexclude();
				return 1;
			case 0x0F:
				lpPBrush->colors[0].num = ~lpPBrush->colors[0].num;
				lpPBrush->colors[1].num = ~lpPBrush->colors[1].num;
				dsPatCopy(DestxOrg, DestyOrg, xExt, yExt, lpPBrush);
				lpPBrush->colors[0].num = ~lpPBrush->colors[0].num;
				lpPBrush->colors[1].num = ~lpPBrush->colors[1].num;
				Unexclude();
				return 1;
			case 0xF0:
				dsPatCopy(DestxOrg, DestyOrg, xExt, yExt, lpPBrush);
				Unexclude();
				return 1;
			case 0xFF:
				patAllZero.colors[0].num = -1l;
				dsPatCopy(DestxOrg, DestyOrg, xExt, yExt, &patAllZero);
				Unexclude();
				return 1;
		}
	}
#endif

	flags = CompileRop(Rop >> 16);

	if(!(flags & SOURCE_PRESENT)) {
		srcFetch = FetchNothing;
	} else if(lpSrcDev->bm.bmType == PHYS_DEV_ID) {
		Exclude(SrcxOrg, SrcyOrg, xExt, yExt);
		flags |= EXCLUDE_CALLED;
		srcFetch = FetchDisplay;
		if(lpDestDev->bm.bmType == PHYS_DEV_ID && DestxOrg > SrcxOrg) {
			srcData.xy.xInc = -1;
			srcData.xy.x = SrcxOrg + xExt - 1;
			srcData.xy.xLimit = SrcxOrg - 1;
			flags |= DISP_REVERSE_X;
		} else {
			srcData.xy.xInc = 1;
			srcData.xy.x = SrcxOrg;
			srcData.xy.xLimit = SrcxOrg + xExt;
		}
		srcData.xy.xOrig = srcData.xy.x;
		if(lpDestDev->bm.bmType == PHYS_DEV_ID && DestyOrg > SrcyOrg) {
			srcData.xy.yInc = -1;
			srcData.xy.y = SrcyOrg + yExt - 1;
			flags |= DISP_REVERSE_Y;
		} else {
			srcData.xy.yInc = 1;
			srcData.xy.y = SrcyOrg;
		}
	} else {
		switch(lpSrcDev->bm.bmBitsPixel) {
			case 1:
				srcFetch = FetchMonoBitmap;
				srcData.off.offset = lpSrcDev->bm.bmBits + SrcyOrg * lpSrcDev->bm.bmWidthBytes +
						(SrcxOrg >> 3);
				srcData.off.shift = SrcxOrg & 7;
				break;
#if (DC_BITS != 1)
			case DC_BITS:
				srcFetch = FetchColorBitmap;
				srcData.off.offset = lpSrcDev->bm.bmBits + SrcyOrg * lpSrcDev->bm.bmWidthBytes +
						(SrcxOrg * DC_BITS >> 3);
				srcData.off.shift = SrcxOrg * DC_BITS & 7;
				break;
#endif
			default:
				return 0;
		}
		srcData.off.rowOffset = srcData.off.offset;
		srcData.off.x = 0;
		srcData.off.xLimit = xExt;
		srcData.off.rowNumBytes = lpSrcDev->bm.bmWidthBytes;
		srcData.off.rowShift = srcData.off.shift;
	}

	if(lpDestDev->bm.bmType == PHYS_DEV_ID) {
#if (DC_HAS_DSPATCOPY != 1)
		Exclude(DestxOrg, DestyOrg, xExt, yExt);
#endif
		flags |= EXCLUDE_CALLED;
		destFetch = FetchDisplay;
		destPut = PutDisplay;
		if(flags & DISP_REVERSE_X) {
			destData.xy.xInc = -1;
			destData.xy.x = DestxOrg + xExt - 1;
			destData.xy.xLimit = DestxOrg - 1;
		} else {
			destData.xy.xInc = 1;
			destData.xy.x = DestxOrg;
			destData.xy.xLimit = DestxOrg + xExt;
		}
		destData.xy.xOrig = destData.xy.x;
		if(flags & DISP_REVERSE_Y) {
			destData.xy.yInc = -1;
			destData.xy.y = DestyOrg + yExt - 1;
		} else {
			destData.xy.yInc = 1;
			destData.xy.y = DestyOrg;
		}
	} else {
		switch(lpDestDev->bm.bmBitsPixel) {
			case 1:
				destFetch = FetchMonoBitmap;
				destPut = PutMonoBitmap;
				destData.off.offset = lpDestDev->bm.bmBits + DestyOrg * lpDestDev->bm.bmWidthBytes +
						(DestxOrg >> 3);
				destData.off.shift = DestxOrg & 7;
				break;
#if (DC_BITS != 1)
			case DC_BITS:
				destFetch = FetchColorBitmap;
				destPut = PutColorBitmap;
				destData.off.offset = lpDestDev->bm.bmBits + DestyOrg * lpDestDev->bm.bmWidthBytes +
						(DestxOrg * DC_BITS >> 3);
				destData.off.shift = DestxOrg * DC_BITS & 7;
				break;
#endif
			default:
				return 0;
		}
		destData.off.rowOffset = destData.off.offset;
		destData.off.x = 0;
		destData.off.xLimit = xExt;
		destData.off.rowNumBytes = lpDestDev->bm.bmWidthBytes;
		destData.off.rowShift = destData.off.shift;
	}
	destPutData = destData;

	if(flags & PATTERN_PRESENT) {
		patFetch = FetchPattern;
		if(lpDestDev->bm.bmType == PHYS_DEV_ID) {
			patData = destData;
		} else {
			patData.xy.xInc = 1;
			patData.xy.x = DestxOrg;
			patData.xy.xOrig = patData.xy.x;
			patData.xy.xLimit = DestxOrg + xExt;
			patData.xy.yInc = 1;
			patData.xy.y = DestyOrg;
		}
		patData.xy.lpPBrush = lpPBrush;
	} else {
		patFetch = FetchNothing;
	}

	iEnd = (DWORD)xExt * yExt;
	for(i = 0; i < iEnd; i++) {
		currSrcPixel.num = (*srcFetch)(&srcData);
		currDestPixel.num = (*destFetch)(&destData);
		currPatPixel.num = (*patFetch)(&patData);
		ExecuteRop(&currDestPixel.col.r, currSrcPixel.col.r, currPatPixel.col.r);
		ExecuteRop(&currDestPixel.col.g, currSrcPixel.col.g, currPatPixel.col.g);
		ExecuteRop(&currDestPixel.col.b, currSrcPixel.col.b, currPatPixel.col.b);
		(*destPut)(&destPutData, currDestPixel.num);
	}

	if(flags & EXCLUDE_CALLED) {
		Unexclude();
	}

	return 1;
}

DWORD FAR PASCAL ColorInfo(lp_device, color_in, lp_phys_bits)
DEVBITMAP FAR *lp_device;
COLOR color_in;
COLOR FAR *lp_phys_bits;
{
	color_in.num = GetNearestColor(color_in, (COLOR FAR *)NULL);
	if(lp_phys_bits)
		*lp_phys_bits = color_in;
	if(lp_device->bm.bmType == PHYS_DEV_ID) {
		return color_in.num;
	}
	return (WORD)color_in.col.r + color_in.col.g + color_in.col.b & 0x200 ? 0xFFFFFF : 0;
}

WORD FAR PASCAL Control(lp_device, function, lp_in_data, lp_out_data)
DWORD lp_device;
WORD function;
DWORD lp_in_data;
DWORD lp_out_data;
{
	return 0;
}

WORD FAR PASCAL Disable(lp_device)
DWORD lp_device;
{
	dsResetMode();
	return -1;
}

WORD FAR PASCAL Enable(lp_device, style, lp_device_type, lp_output_file, lp_stuff)
LPSTR lp_device;
WORD style;
LPSTR lp_device_type;
LPSTR lp_output_file;
LPSTR lp_stuff;
{
	char buf[11];

#if (DC_USES_SAVE_AREA == 1)
	if(!saveAddr) {
#if (OS <= W1_DR5)
		saveAddr = GetSaveAddress();
#else
		saveAddr = (LPSTR)temp_buffer;
#endif
	}
#endif

#if (DC_MODIFIABLE_RES == 1)
	if(!enableCalled) {
		GetProfileRes(devRes);
		if(dsParseRes(devRes)) {
			SetProfileRes(devRes);
		}
		if(devRes[DR_WIDTH] && devRes[DR_HEIGHT]) {
			gdiInfo.dpVertSize = (WORD)((DWORD)WIDTH_MM * devRes[DR_HEIGHT] / devRes[DR_WIDTH]);
			gdiInfo.dpHorzRes = devRes[DR_WIDTH];
			gdiInfo.dpVertRes = devRes[DR_HEIGHT];
			deviceInfo.bm.bmWidth = devRes[DR_WIDTH];
			deviceInfo.bm.bmHeight = devRes[DR_HEIGHT];
			deviceInfo.bm.bmWidthBytes = (devRes[DR_WIDTH] * DC_BITS) / 8;
		}
		enableCalled = TRUE;
	}
#endif

	if(style == 0x01) {
		*(GDIINFO FAR *)lp_device = gdiInfo;
		return sizeof(GDIINFO);
	} else {
		*(DEVBITMAP FAR *)lp_device = deviceInfo;
		return dsSetMode();
	}
}

WORD FAR PASCAL EnumDFonts(lp_device, lp_face_name, lp_callback_func, lp_client_data)
DWORD lp_device;
DWORD lp_face_name;
DWORD lp_callback_func;
DWORD lp_client_data;
{
	return 1;
}

WORD FAR PASCAL EnumObj(lp_device, style, lp_callback_func, lp_client_data)
DWORD lp_device;
WORD style;
DWORD lp_callback_func;
DWORD lp_client_data;
{
	return 1;
}

/* r2Mod: a modified version of Rop2 that is more readable bit-wise.

If r2Mod == 0, then do nothing special, just put down the pattern color
If r2Mod != 0, then it follows the bit format r2Mod = ABCD, where:
	A=NOT destination color before primary op
	B=NOT result after primary op
	CD=Primary op (00=Destination byte only, 01=XOR, 10=AND, 11=OR)
If the pattern color is NOTted beforehand, this is already done and won't be specified in r2Mod
*/
#define R2M_NONE			0
#define R2M_PRIM_OP			3
#define R2M_DESTONLY		0
#define R2M_XOR				1
#define R2M_AND				2
#define R2M_OR				3
#define R2M_TRAILING_NOT	4
#define R2M_DEST_NOT		8
void NEAR PASCAL Line(xFrom, yFrom, xTo, yTo, lineColor, r2Mod)
int xFrom;
int yFrom;
int xTo;
int yTo;
COLOR lineColor;
int r2Mod;
{
	BOOL xSwap;
	BOOL ySwap;
	int tempVar;
	BOOL lesserX;
	int dL;
	int dG;
	int lesserN;
	int lesserD;
	int lesser;
	int greater;
	int gMin;
	int x;
	int y;
	COLOR destColor;

	if(xTo < xFrom) {
		tempVar = xTo;
		xTo = xFrom;
		xFrom = tempVar;
		xSwap = TRUE;
	} else {
		xSwap = FALSE;
	}
#if (DC_HAS_DSPATCOPY == 1)
	if(!r2Mod && yFrom == yTo) {
		patAllZero.colors[0].num = lineColor.num;
		dsPatCopy(xFrom, yFrom, xTo - xFrom, 1, &patAllZero);
		return;
	}
#endif
	if(yTo < yFrom) {
		tempVar = yTo;
		yTo = yFrom;
		yFrom = tempVar;
		ySwap = TRUE;
	} else {
		ySwap = FALSE;
	}
	lesserX = xTo - xFrom < yTo - yFrom;
	dL = lesserX ? xTo - xFrom : yTo - yFrom;
	dG = lesserX ? yTo - yFrom : xTo - xFrom;
	lesserN = dG;
	lesserD = dG << 1;
	lesser = lesserX ? xFrom : yFrom;
	gMin = lesserX ? yFrom : xFrom;
	destColor.num = lineColor.num; /* Time saver in most cases */
	for(greater = gMin; greater < gMin + dG; greater++) {
		if(lesserX) {
			x = xSwap ? xTo - lesser + xFrom : lesser;
			y = ySwap ? yTo - greater + yFrom : greater;
		} else {
			x = xSwap ? xTo - greater + xFrom : greater;
			y = ySwap ? yTo - lesser + yFrom : lesser;
		}
		if(r2Mod) {
			destColor.num = dsGetPixel(x, y);
			if(r2Mod & R2M_DEST_NOT) {
				destColor.num = ~destColor.num;
			}
			switch(r2Mod & R2M_PRIM_OP) {
				case R2M_XOR:
					destColor.num ^= lineColor.num;
					break;
				case R2M_AND:
					destColor.num &= lineColor.num;
					break;
				case R2M_OR:
					destColor.num |= lineColor.num;
					/* break; */
				/* case R2M_DESTONLY: */
				/* default: */
			}
			if(r2Mod & R2M_TRAILING_NOT) {
				destColor.num = ~destColor.num;
			}
		}
		dsSetPixel(x, y, destColor);
		lesserN += dL << 1;
		if(lesserN >= lesserD) {
			lesserN -= lesserD;
			lesser++;
		}
	}
}

void NEAR PASCAL Scanline(xFrom, xTo, y, color0, color1, pattern, r2Mod)
int xFrom;
int xTo;
int y;
COLOR color0;
COLOR color1;
BYTE pattern;
int r2Mod;
{
	int x;
	int tempVar;
	COLOR destColor;
	COLOR lineColor;

	if(xTo < xFrom) {
		tempVar = xFrom;
		xFrom = xTo;
		xTo = tempVar;
	}
	for(x = xFrom; x < xTo; x++) {
		lineColor.num = pattern >> (7 - (x & 7)) & 1 ? color1.num : color0.num;
		if(r2Mod) {
			destColor.num = dsGetPixel(x, y);
			if(r2Mod & R2M_DEST_NOT) {
				destColor.num = ~destColor.num;
			}
			switch(r2Mod & R2M_PRIM_OP) {
				case R2M_XOR:
					destColor.num ^= lineColor.num;
					break;
				case R2M_AND:
					destColor.num &= lineColor.num;
					break;
				case R2M_OR:
					destColor.num |= lineColor.num;
					/* break; */
				/* case R2M_DESTONLY: */
				/* default: */
			}
			if(r2Mod & R2M_TRAILING_NOT) {
				destColor.num = ~destColor.num;
			}
		} else {
			destColor.num = lineColor.num;
		}
		dsSetPixel(x, y, destColor);
	}
}

#define LS_SOLID 0
#define LS_DASHED 1
#define LS_DOTTED 2
#define LS_DOTDASHED 3
#if (OS <= W1_ALPHA)
#define LS_NOLINE 4
#else
#define LS_DASHDOTDOT 4
#define LS_NOLINE 5
#endif
#define OS_SCANLINES 4
#define OS_POLYLINE 18
WORD FAR PASCAL Output(lp_dst_dev, style, count, lp_points, lp_phys_pen, lp_phys_brush,
		lp_draw_mode, lp_clip_rect)
DEVBITMAP FAR *lp_dst_dev;
WORD style;
WORD count;
LPPOINT lp_points;
OEM_PEN FAR *lp_phys_pen;
OEM_BRUSH FAR *lp_phys_brush;
DRAWMODE FAR *lp_draw_mode;
LPRECT lp_clip_rect;
{
	int i;
	BOOL doLoop;
	COLOR inColor;
	COLOR inColor2;
#if (DC_HAS_DSPATCOPY == 1)
	COLOR tempColor;
	COLOR tempColor2;
#endif
	int r2Mod;

	if(style == OS_POLYLINE || style == OS_SCANLINES) {
		if(lp_dst_dev->bm.bmType == PHYS_DEV_ID && (lp_phys_brush ||
				lp_phys_pen->style != LS_NOLINE)) {
			if(style == OS_SCANLINES && lp_phys_brush) {
				inColor.num = lp_phys_brush->colors[0].num;
				inColor2.num = lp_phys_brush->colors[1].num;
			} else {
				inColor.num = lp_phys_pen->color.num;
			}
			doLoop = TRUE; /* This will be TRUE for almost all cases, so save some space here */
			switch(lp_draw_mode->Rop2) {
				case R2_BLACK:
					inColor.num = 0;
					inColor2.num = 0;
					r2Mod = R2M_NONE;
					break;
				case R2_NOTMERGEPEN:
					r2Mod = R2M_OR | R2M_TRAILING_NOT;
					break;
				case R2_MASKNOTPEN:
					inColor.num = ~inColor.num;
					inColor2.num = ~inColor2.num;
					r2Mod = R2M_AND;
					break;
				case R2_NOTCOPYPEN:
					inColor.num = ~inColor.num;
					inColor2.num = ~inColor2.num;
					r2Mod = R2M_NONE;
					break;
				case R2_MASKPENNOT:
					r2Mod = R2M_AND | R2M_DEST_NOT;
					break;
				case R2_NOT:
					r2Mod = R2M_DESTONLY | R2M_DEST_NOT;
					break;
				case R2_XORPEN:
					r2Mod = R2M_XOR;
					break;
				case R2_NOTMASKPEN:
					r2Mod = R2M_AND | R2M_TRAILING_NOT;
					break;
				case R2_MASKPEN:
					r2Mod = R2M_AND;
					break;
				case R2_NOTXORPEN:
					r2Mod = R2M_XOR | R2M_TRAILING_NOT;
					break;
				case R2_MERGENOTPEN:
					inColor.num = ~inColor.num;
					inColor2.num = ~inColor2.num;
					r2Mod = R2M_OR;
					break;
				case R2_COPYPEN:
					r2Mod = R2M_NONE;
					break;
				case R2_MERGEPENNOT:
					r2Mod = R2M_OR | R2M_DEST_NOT;
					break;
				case R2_MERGEPEN:
					r2Mod = R2M_OR;
					break;
				case R2_WHITE:
					inColor.num = 0xFFFFFF;
					inColor2.num = 0xFFFFFF;
					r2Mod = R2M_NONE;
					break;
				/* case R2_NOP: */
				default:
					doLoop = FALSE;
			}
		} else {
			doLoop = FALSE;
		}
	} else {
		return 0;
	}
	if(doLoop) {
#if (DC_MODIFIABLE_RES == 1)
		Exclude(0, 0, devRes[DR_WIDTH], devRes[DR_HEIGHT]);
#else
		Exclude(0, 0, DC_WIDTH, DC_HEIGHT);
#endif
		switch(style) {
			case OS_POLYLINE:
				for(i = 1; i < count; i++) {
					Line(lp_points[i - 1].x, lp_points[i - 1].y, lp_points[i].x, lp_points[i].y,
							inColor, r2Mod);
				}
				break;
			case OS_SCANLINES:
				if(lp_phys_brush) {
#if (DC_HAS_DSPATCOPY == 1)
					if(!r2Mod) {
						tempColor.num = lp_phys_brush->colors[0].num;
						tempColor2.num = lp_phys_brush->colors[1].num;
						lp_phys_brush->colors[0].num = inColor.num;
						lp_phys_brush->colors[1].num = inColor2.num;
					}
#endif
					for(i = 1; i < count; i++) {
#if (DC_HAS_DSPATCOPY == 1)
						if(!r2Mod) {
							dsPatCopy(lp_points[i].x, lp_points[0].y, lp_points[i].y -
									lp_points[i].x, 1, lp_phys_brush);
							continue;
						}
#endif
						Scanline(lp_points[i].x, lp_points[i].y, lp_points[0].y, inColor, inColor2,
								lp_phys_brush->pat.arr[lp_points[0].y & 7], r2Mod);
					}
#if (DC_HAS_DSPATCOPY == 1)
					if(!r2Mod) {
						lp_phys_brush->colors[0].num = tempColor.num;
						lp_phys_brush->colors[1].num = tempColor2.num;
					}
#endif
				} else {
					for(i = 1; i < count; i++) {
						Line(lp_points[i].x, lp_points[0].y, lp_points[i].y, lp_points[0].y,
								inColor, r2Mod);
					}
				}
				/* break; */
		}
		Unexclude();
	}
	return 1;
}

DWORD FAR PASCAL Pixel(lp_device, x, y, p_color, lp_draw_mode)
DWORD lp_device;
WORD x;
WORD y;
DWORD p_color;
DWORD lp_draw_mode;
{
	return 0x80000000;
}

EIGHT_BYTES hatches[] = {
	{    /* (-) Horizontal hatch */
		0x00, /* 0b00000000 */
		0x00, /* 0b00000000 */
		0x00, /* 0b00000000 */
		0x00, /* 0b00000000 */
		0xFF, /* 0b11111111 */
		0x00, /* 0b00000000 */
		0x00, /* 0b00000000 */
		0x00  /* 0b00000000 */
	}, { /* (|) Vertical hatch */
		0x08, /* 0b00001000 */
		0x08, /* 0b00001000 */
		0x08, /* 0b00001000 */
		0x08, /* 0b00001000 */
		0x08, /* 0b00001000 */
		0x08, /* 0b00001000 */
		0x08, /* 0b00001000 */
		0x08  /* 0b00001000 */
	}, { /* (\) Diagonal hatch */
		0x80, /* 0b10000000 */
		0x40, /* 0b01000000 */
		0x20, /* 0b00100000 */
		0x10, /* 0b00010000 */
		0x08, /* 0b00001000 */
		0x04, /* 0b00000100 */
		0x02, /* 0b00000010 */
		0x01  /* 0b00000001 */
	}, { /* (/) Diagonal hatch */
		0x01, /* 0b00000001 */
		0x02, /* 0b00000010 */
		0x04, /* 0b00000100 */
		0x08, /* 0b00001000 */
		0x10, /* 0b00010000 */
		0x20, /* 0b00100000 */
		0x40, /* 0b01000000 */
		0x80  /* 0b10000000 */
	}, { /* (+) Cross hatch */
		0x08, /* 0b00001000 */
		0x08, /* 0b00001000 */
		0x08, /* 0b00001000 */
		0x08, /* 0b00001000 */
		0xFF, /* 0b11111111 */
		0x08, /* 0b00001000 */
		0x08, /* 0b00001000 */
		0x08  /* 0b00001000 */
	}, { /* (X) Diagonal cross hatch */
		0x81, /* 0b10000001 */
		0x42, /* 0b01000010 */
		0x24, /* 0b00100100 */
		0x18, /* 0b00011000 */
		0x18, /* 0b00011000 */
		0x24, /* 0b00100100 */
		0x42, /* 0b01000010 */
		0x81  /* 0b10000001 */
	}
};

#if (OS >= W1_BETA)
#define OBJ_PEN 1
#else
#define OBJ_PEN 2
#endif
#define OBJ_BRUSH (OBJ_PEN + 1)
WORD FAR PASCAL RealizeObject(lp_device, style, lp_in_obj, lp_out_obj, lp_text_xform)
DWORD lp_device;
WORD style;
union {
	LPLOGPEN pen;
	LPLOGBRUSH brush;
} lp_in_obj;
union {
	OEM_PEN FAR *pen;
	OEM_BRUSH FAR *brush;
} lp_out_obj;
DWORD lp_text_xform;
{
	if(style & 0x8000) {
		return 1;
	}

	if(lp_out_obj.pen) {
		switch(style) {
			case OBJ_PEN:
				if(lp_in_obj.pen->lopnStyle != LS_NOLINE)
					lp_out_obj.pen->color.num = GetNearestColor(lp_in_obj.pen->lopnColor,
							(COLOR FAR *)NULL);
				lp_out_obj.pen->style = lp_in_obj.pen->lopnStyle <= LS_NOLINE
						? lp_in_obj.pen->lopnStyle : LS_SOLID;
				break;
			case OBJ_BRUSH:
				lp_out_obj.brush->style = lp_in_obj.brush->lbStyle;
				switch(lp_in_obj.brush->lbStyle) {
					case BS_HOLLOW:
						/* We're done if it's hollow */
						break;
					case BS_PATTERN:
						break;
					case BS_HATCHED:
						/* We're given a hatch style to use here */
						*(LPSTR)(0x9000FFFF) = 1;
						*(LPSTR)(0x9000FFFF) = 0;
						if(lp_in_obj.brush->lbColor.num != lp_in_obj.brush->lbBkColor.num &&
								lp_in_obj.brush->lbHatch <= HS_DIAGCROSS) {
							lp_out_obj.brush->colors[0].num = GetNearestColor(
									lp_in_obj.brush->lbBkColor, NULL);
							lp_out_obj.brush->colors[1].num = GetNearestColor(
									lp_in_obj.brush->lbColor, NULL);
							lp_out_obj.brush->pat.cpy = hatches[lp_in_obj.brush->lbHatch];
							break;
						}
					default:
						/* Assume solid brush otherwise */
						lp_out_obj.brush->colors[0].num = GetNearestColor(lp_in_obj.brush->lbColor,
								&lp_out_obj.brush->colors[1]);
						Dither(&lp_out_obj.brush->pat, lp_in_obj.brush->lbColor.num,
								lp_out_obj.brush->colors[0].num, lp_out_obj.brush->colors[1].num);
				}
		}
	}

	switch(style) {
		case OBJ_PEN:
			return sizeof(OEM_PEN);
		case OBJ_BRUSH:
			return sizeof(OEM_BRUSH);
		default:
			return 0;
	}
}

#define BM_OPAQUE 2
DWORD FAR PASCAL Strblt(lp_device, x, y, lp_clip_rect, lp_string, count, lp_font, lp_draw_mode,
		lp_xform)
DEVBITMAP FAR *lp_device;
WORD x;
WORD y;
LPRECT lp_clip_rect;
LPSTR lp_string;
WORD count;
FONTSEG FAR *lp_font;
DRAWMODE FAR *lp_draw_mode;
TEXTXFORM FAR *lp_xform;
{
	BOOL extentOnly;
#if (DC_HAS_DSTEXTOUT == 1)
	BOOL clipped = FALSE;
#endif
	BOOL proportional;
	WORD xMin;
	WORD xMax;
	WORD yMin;
	WORD yMax;
	BYTE currChar;
	BYTE currWidth;
	WORD charRowOffset;
	LPSTR bitsPointer;
	WORD i;
	WORD j;
	WORD k;

	if(count & 0x8000) {
		count = -count;
		extentOnly = TRUE;
	} else {
		extentOnly = FALSE;
	}

	proportional = lp_font->fsPixWidth == 0;

	xMin = x;
	yMin = y;
#if (DC_HAS_DSTEXTOUT == 0)
	if(proportional) {
		xMax = 0;
		for(i = 0; i < count; i++) {
			currChar = lp_string[i] >= lp_font->fsFirstChar && lp_string[i] <= lp_font->fsLastChar ?
				lp_string[i] - lp_font->fsFirstChar : lp_font->fsDefaultChar;
			xMax += lp_font->propOffsets[currChar + 1] - lp_font->propOffsets[currChar];
		}
		xMax += x;
	} else {
		xMax = x + lp_font->fsPixWidth * count;
	}
	yMax = y + lp_font->fsPixHeight;
#else
	xMax = x + DC_LOGICAL_WIDTH * count;
	yMax = y + DC_LOGICAL_HEIGHT;
#endif
#if (DC_MODIFIABLE_RES == 1)
	xMax = min(xMax, devRes[DR_WIDTH]);
	yMax = min(yMax, devRes[DR_HEIGHT]);
#else
	xMax = min(xMax, DC_WIDTH);
	yMax = min(yMax, DC_HEIGHT);
#endif
	if(lp_clip_rect) {
		xMin = max(xMin, lp_clip_rect->left);
		yMin = max(yMin, lp_clip_rect->top);
		xMax = min(xMax, lp_clip_rect->right);
		yMax = min(yMax, lp_clip_rect->bottom);
	}
	if(xMax <= xMin || yMax <= yMin) {
		return 0;
	}
	if(extentOnly) {
		return ((DWORD)yMax - yMin << 16) + (xMax - xMin);
	}

	bitsPointer = (LPSTR)(((DWORD)lp_font >> 16 << 16) + lp_font->fsBitsOffset);

	Exclude(xMin, yMin, xMax - xMin, yMax - yMin);
	for(i = 0; i < count; i++) {
		currChar = lp_string[i] >= lp_font->fsFirstChar && lp_string[i] <= lp_font->fsLastChar ?
				lp_string[i] - lp_font->fsFirstChar : lp_font->fsDefaultChar;
		if(proportional) {
			charRowOffset = lp_font->propOffsets[currChar];
			currWidth = lp_font->propOffsets[currChar + 1] - charRowOffset;
		} else {
			charRowOffset = currChar * lp_font->fsPixWidth;
			currWidth = lp_font->fsPixWidth;
		}
		for(j = 0; j < lp_font->fsPixHeight; j++) {
			for(k = 0; k < currWidth; k++) {
				if(x >= xMin && x < xMax && y >= yMin && y < yMax) {
					if(bitsPointer[charRowOffset >> 3] << (charRowOffset & 7) & 0x80) {
						dsSetPixel(x, y, lp_draw_mode->TextColor.num);
					} else if(lp_draw_mode->bkMode == BM_OPAQUE) {
						dsSetPixel(x, y, lp_draw_mode->bkColor.num);
					}
				}
#if (DC_HAS_DSTEXTOUT == 1)
				else if(x < lp_clip_rect->left || x >= lp_clip_rect->right ||
						y < lp_clip_rect->top || y >= lp_clip_rect->bottom) {
					clipped = TRUE;
				}
#endif
				x++;
				charRowOffset++;
			}
			x -= currWidth;
			charRowOffset -= currWidth;
			y++;
			charRowOffset += lp_font->fsWidthBytes << 3;
		}
		x += currWidth;
		y -= lp_font->fsPixHeight;
	}

#if (DC_HAS_DSTEXTOUT == 1)
	if(TRUE) {
		dsTextOut(xMin / DC_LOGICAL_WIDTH, yMin / DC_LOGICAL_HEIGHT, count, lp_string);
	}
#endif

	Unexclude();

	return 0;
}

WORD FAR PASCAL ScanLR(lp_device, x, y, color, dir_style)
DWORD lp_device;
WORD x;
WORD y;
DWORD color;
WORD dir_style;
{
	return -1;
}

void FAR PASCAL DeviceMode()
{
}

#if (OS <= W1_DR5)
void FAR PASCAL _dsSetPixel(x, y, color)
WORD x;
WORD y;
COLOR color;
{
	dsSetPixel(x, y, color);
}

DWORD FAR PASCAL _dsGetPixel(x, y)
WORD x;
WORD y;
{
	return dsGetPixel(x, y);
}

#if (DC_MODIFIABLE_RES == 1)
LPINT FAR PASCAL GetDimensions()
{
	return (LPINT)devRes;
}
#endif
#endif
