#include "os.h"
#include "winnmeth.h"
#include "devconst.h"
#define IS_CURSOR
#include "devspec.h"

#define MAX_CUR_WIDTH 32
#define MAX_CUR_HEIGHT 32

#if (DC_MODIFIABLE_RES == 1)
#define DR_WIDTH 0
#define DR_HEIGHT 1
#if (OS <= W1_DR5)
LPINT FAR PASCAL GetDimensions(void);
LPINT devRes = NULL;
#else
extern int devRes[];
#endif
#endif

CURSORINFO inquireInfo = {
	/* dpXRate = */ 1,
	/* dpYRate = */ 2
#if (OS <= W1_DR5)
	,
	/* unknown1 = */ -8,
	/* unknown2 = */ -1,
	/* unknown3 = */ 0x10,
	/* unknown4 = */ 0x20,
	/* unknown5 = */ 0x20,
	/* unknown6 = */ 0x20
#endif
};

BOOL cur_enabled = FALSE;
BOOL cur_excluded = FALSE;
BOOL display_busy = FALSE;
WORD cur_old_x = 0;
WORD cur_old_y = 0;
WORD cur_x = 0;
WORD cur_y = 0;
WORD cur_new_x = 0;
WORD cur_new_y = 0;
CURSORSHAPE cur;
COLOR saved_pixels[MAX_CUR_WIDTH * MAX_CUR_HEIGHT];
COLOR temp_buffer[(2 * MAX_CUR_WIDTH - 1) * (2 * MAX_CUR_HEIGHT - 1)];
BYTE and_masks[MAX_CUR_WIDTH * MAX_CUR_HEIGHT];
BYTE xor_masks[MAX_CUR_WIDTH * MAX_CUR_HEIGHT];

/* Cursor drawing routines */

VOID NEAR PASCAL DrawCursorLine(line)
WORD line;
{
	int column;
	int x;
	int y;
	int y_offset;
	int end_x;
	BYTE mask;
	COLOR curr_pixel;

	y = cur_y - cur.csHotY + line;
#if (DC_MODIFIABLE_RES == 1)
	if(y < 0 || y >= devRes[DR_HEIGHT]) {
#else
	if(y < 0 || y >= DC_HEIGHT) {
#endif
		return;
	}

	x = cur_x - cur.csHotX;
	column = 0;
	if(x < 0) {
		column -= x;
		x = 0;
	}

	end_x = x - column + cur.csWidth;
#if (DC_MODIFIABLE_RES == 1)
	if(end_x > devRes[DR_WIDTH])
		end_x = devRes[DR_WIDTH];
#else
	if(end_x > DC_WIDTH)
		end_x = DC_WIDTH;
#endif

	y_offset = cur.csWidth * line;

	while(x < end_x) {
		curr_pixel.num = dsGetPixel(x, y);
		saved_pixels[column + y_offset] = curr_pixel;

		mask = and_masks[column + y_offset];
		curr_pixel.col.r &= mask;
		curr_pixel.col.g &= mask;
		curr_pixel.col.b &= mask;

		mask = xor_masks[column + y_offset];
		curr_pixel.col.r ^= mask;
		curr_pixel.col.g ^= mask;
		curr_pixel.col.b ^= mask;

		dsSetPixel(x, y, curr_pixel.num);

		x++;
		column++;
	}
}

VOID NEAR PASCAL EraseCursorLine(line)
WORD line;
{
	int column;
	int x;
	int y;
	int y_offset;
	int end_x;
	BYTE mask;
	COLOR curr_pixel;

	y = cur_old_y - cur.csHotY + line;
#if (DC_MODIFIABLE_RES == 1)
	if(y < 0 || y >= devRes[DR_HEIGHT]) {
#else
	if(y < 0 || y >= DC_HEIGHT) {
#endif
		return;
	}

	x = cur_old_x - cur.csHotX;
	column = 0;
	if(x < 0) {
		column -= x;
		x = 0;
	}

	end_x = x - column + cur.csWidth;

#if (DC_MODIFIABLE_RES == 1)
	if(end_x > devRes[DR_WIDTH])
		end_x = devRes[DR_WIDTH];
#else
	if(end_x > DC_WIDTH)
		end_x = DC_WIDTH;
#endif

	y_offset = cur.csWidth * line;

	while(x < end_x)
		dsSetPixel(x++, y, saved_pixels[column++ + y_offset].num);
}

VOID NEAR PASCAL DrawCursor()
{
	WORD i;

	for(i = 0; i < cur.csHeight; i++)
		DrawCursorLine(i);
}

VOID NEAR PASCAL EraseCursor()
{
	WORD i;

	for(i = 0; i < cur.csHeight; i++)
		EraseCursorLine(i);
}

VOID NEAR PASCAL BlockToBuffer(x, y, xExt, yExt, xWidth, xStart, yStart)
WORD x;
WORD y;
WORD xExt;
WORD yExt;
WORD xWidth;
WORD xStart;
WORD yStart;
{
	WORD tempOffset;
	WORD tempOffsetT;
	WORD currX;

	tempOffset = (y - yStart) * xWidth + x - xStart;
	xExt += x;
	yExt += y;

	for(; y < yExt; y++) {
		tempOffsetT = tempOffset;
		for(currX = x; currX < xExt; currX++) {
			temp_buffer[tempOffsetT++].num = dsGetPixel(currX, y);
		}
		tempOffset += xWidth;
	}
}

VOID NEAR PASCAL UpdateCursorShort()
{
	int x_old;
	int y_old;
	int x_new;
	int y_new;
	int xMin;
	int xMax;
	int yMin;
	int yMax;
	WORD width;
	WORD height;
	int x;
	int y;
	WORD sOffset;
	WORD sOffsetT;
	WORD tOffset;
	WORD tOffsetT;

	x_old = cur_old_x - cur.csHotX;
	y_old = cur_old_y - cur.csHotY;
	x_new = cur_x - cur.csHotX;
	y_new = cur_y - cur.csHotY;

	xMin = min(cur_old_x, cur_x) - cur.csHotX;
	xMin = max(xMin, 0);

	xMax = max(cur_old_x, cur_x) - cur.csHotX + cur.csWidth;

	yMin = min(cur_old_y, cur_y) - cur.csHotY;
	yMin = max(yMin, 0);

	yMax = max(cur_old_y, cur_y) - cur.csHotY + cur.csHeight;

#if (DC_MODIFIABLE_RES == 1)
	xMax = min(xMax, devRes[DR_WIDTH]);
	yMax = min(yMax, devRes[DR_HEIGHT]);
#else
	xMax = min(xMax, DC_WIDTH);
	yMax = min(yMax, DC_HEIGHT);
#endif

	width = xMax - xMin;
	height = yMax - yMin;

	BlockToBuffer(xMin, yMin, width, height, width, xMin, yMin);

	sOffset = -(cur.csWidth * min(y_old, 0) + min(x_old, 0));
	tOffset = width * max(y_old - yMin, 0) + max(x_old - xMin, 0);
	for(y = max(y_old, yMin); y < min(y_old + cur.csHeight, yMax); y++) {
		sOffsetT = sOffset;
		tOffsetT = tOffset;
		for(x = max(x_old, xMin); x < min(x_old + cur.csWidth, xMax); x++)
			temp_buffer[tOffsetT++] = saved_pixels[sOffsetT++];
		tOffset += width;
		sOffset += cur.csWidth;
	}

	sOffset = -(cur.csWidth * min(y_new, 0) + min(x_new, 0));
	tOffset = width * max(y_new - yMin, 0) + max(x_new - xMin, 0);
	for(y = max(y_new, yMin); y < min(y_new + cur.csHeight, yMax); y++) {
		sOffsetT = sOffset;
		tOffsetT = tOffset;
		for(x = max(x_new, xMin); x < min(x_new + cur.csWidth, xMax); x++) {
			saved_pixels[sOffsetT] = temp_buffer[tOffsetT];

			temp_buffer[tOffsetT].col.r &= and_masks[sOffsetT];
			temp_buffer[tOffsetT].col.g &= and_masks[sOffsetT];
			temp_buffer[tOffsetT].col.b &= and_masks[sOffsetT];

			temp_buffer[tOffsetT].col.r ^= xor_masks[sOffsetT];
			temp_buffer[tOffsetT].col.g ^= xor_masks[sOffsetT];
			temp_buffer[tOffsetT++].col.b ^= xor_masks[sOffsetT++];
		}
		tOffset += width;
		sOffset += cur.csWidth;
	}

	if(abs(y_new - y_old) >= abs(x_new - x_old)) {
		if(y_new < y_old) {
			tOffsetT = 0;
			for(y = yMin; y < yMax; y++)
				for(x = xMin; x < xMax; x++)
					dsSetPixel(x, y, temp_buffer[tOffsetT++].num);
		} else {
			tOffset = width * (height - 1);
			for(y = yMax - 1; y >= yMin; y--) {
				tOffsetT = tOffset;
				for(x = xMin; x < xMax; x++)
					dsSetPixel(x, y, temp_buffer[tOffsetT++].num);
				tOffset -= width;
			}
		}
	} else {
		if(x_new < x_old) {
			tOffset = 0;
			for(x = xMin; x < xMax; x++) {
				tOffsetT = tOffset++;
				for(y = yMin; y < yMax; y++) {
					dsSetPixel(x, y, temp_buffer[tOffsetT].num);
					tOffsetT += width;
				}
			}
		} else {
			tOffset = width - 1;
			for(x = xMax - 1; x >= xMin; x--) {
				tOffsetT = tOffset--;
				for(y = yMin; y < yMax; y++) {
					dsSetPixel(x, y, temp_buffer[tOffsetT].num);
					tOffsetT += width;
				}
			}
		}
	}
}


#if (OS >= W1_ALPHA)
/* Exclusion functions */

void NEAR PASCAL Exclude(x, y, width, height)
WORD x;
WORD y;
WORD width;
WORD height;
{
	int curXMin;
	int curYMin;
	int curXMax;
	int curYMax;

	display_busy = TRUE;

	curXMin = max(cur_old_x - cur.csHotX, 0);
	curYMin = max(cur_old_y - cur.csHotY, 0);
#if (DC_MODIFIABLE_RES == 1)
	curXMax = min(cur_old_x - cur.csHotX + cur.csWidth, devRes[DR_WIDTH]);
	curYMax = min(cur_old_y - cur.csHotY + cur.csHeight, devRes[DR_HEIGHT]);
#else
	curXMax = min(cur_old_x - cur.csHotX + cur.csWidth, DC_WIDTH);
	curYMax = min(cur_old_y - cur.csHotY + cur.csHeight, DC_HEIGHT);
#endif

	if(x > curXMax || x + width < curXMin || y > curYMax || y + height < curYMin)
		return;

	if(cur_enabled && !cur_excluded)
		EraseCursor();
	cur_excluded = TRUE;
}

void NEAR PASCAL Unexclude()
{
	display_busy = FALSE;
}

#endif


/* API functions */

WORD FAR PASCAL Inquire(lp_cursor_info)
CURSORINFO FAR *lp_cursor_info;
{
	*lp_cursor_info = inquireInfo;
	return sizeof(CURSORINFO);
}

void FAR PASCAL SetCursor(lp_cursor)
LPCURSORSHAPE lp_cursor;
{
	WORD x;
	WORD y;
	WORD i;
	WORD maskOffset;
	BYTE andMasks;
	BYTE xorMasks;

#if (OS <= W1_DR5 && DC_MODIFIABLE_RES == 1)
	if(!devRes) {
		devRes = GetDimensions();
	}
#endif

	if(lp_cursor && (lp_cursor->cur.csWidth > MAX_CUR_WIDTH ||
			lp_cursor->cur.csHeight > MAX_CUR_HEIGHT))
		return;

	display_busy = TRUE;

	if(cur_enabled && !cur_excluded)
		EraseCursor();

	if(!lp_cursor) {
		cur_enabled = FALSE;
		cur_excluded = FALSE;
		display_busy = FALSE;
		return;
	}

	cur = lp_cursor->cur;

	for(y = 0; y < cur.csHeight; y++) {
		for(x = 0; x < cur.csWidthBytes; x++) {
			andMasks = lp_cursor->csBits[x + cur.csWidthBytes * y];
			xorMasks = lp_cursor->csBits[x + cur.csWidthBytes * (y + cur.csHeight)];
			for(i = 0; i < 8 && (x << 3) + i < cur.csWidth; i++) {
#if (OS <= W1_DR5) && (DC_WIDTH / 2 < DC_HEIGHT)
				/* All cursors in older versions are half-height regardless of display driver */
				/* EDIT: Ooh boy maybe not all, added if statements */
				if(cur.csHeight <= 16) {
					maskOffset = (x << 3) + i + cur.csWidth * (y << 1);
					and_masks[maskOffset] = andMasks & 0x80 ? -1 : 0;
					xor_masks[maskOffset] = xorMasks & 0x80 ? -1 : 0;
					maskOffset = (x << 3) + i + cur.csWidth * ((y << 1) + 1);
				} else
#endif
				maskOffset = (x << 3) + i + cur.csWidth * y;
				and_masks[maskOffset] = andMasks & 0x80 ? -1 : 0;
				xor_masks[maskOffset] = xorMasks & 0x80 ? -1 : 0;
				andMasks <<= 1;
				xorMasks <<= 1;
			}
		}
	}

#if (OS <= W1_DR5) && (DC_WIDTH / 2 < DC_HEIGHT)
	if(cur.csHeight <= 16) {
		cur.csHeight <<= 1;
	}
#endif

	cur_enabled = TRUE;
	cur_excluded = TRUE;
	display_busy = FALSE;
}

#if (OS <= W1_DR5)
void NEAR PASCAL MoveCursor(abs_x, abs_y)
#else
void FAR PASCAL MoveCursor(abs_x, abs_y)
#endif
WORD abs_x;
WORD abs_y;
{
	int i;

	cur_new_x = abs_x;
	cur_new_y = abs_y;

	if(cur_enabled && !cur_excluded && !display_busy) {
		display_busy = TRUE;
		cur_x = cur_new_x;
		cur_y = cur_new_y;
		if(cur_y - cur_old_y + cur.csHeight >= cur.csHeight << 1 ||
				cur_x - cur_old_x + cur.csWidth >= cur.csWidth << 1) {
			for(i = 0; i < cur.csHeight; i++) {
				EraseCursorLine(i);
				DrawCursorLine(i);
			}
		} else {
			UpdateCursorShort();
		}
		cur_old_x = cur_x;
		cur_old_y = cur_y;
		display_busy = FALSE;
	}
}

#if (OS <= W1_DR5)

void FAR PASCAL ShowCursor(abs_x, abs_y)
WORD abs_x;
WORD abs_y;
{
	if(cur_enabled && !display_busy) {
		if(cur_excluded) {
			DrawCursor();
			cur_excluded = FALSE;
		}
		if(cur_old_x != abs_x || cur_old_y != abs_y) {
			MoveCursor(abs_x, abs_y);
		}
	}
}

void FAR PASCAL HideCursor()
{
	if(cur_enabled && !cur_excluded && !display_busy) {
		EraseCursor();
		cur_excluded = TRUE;
	}
}

LPSTR FAR PASCAL GetSaveAddress()
{
	return (LPSTR)temp_buffer;
}

#else

void FAR PASCAL CheckCursor()
{
	if(cur_enabled && cur_excluded && !display_busy) {
		cur_x = cur_new_x;
		cur_y = cur_new_y;
		DrawCursor();
		cur_old_x = cur_x;
		cur_old_y = cur_y;
		cur_excluded = FALSE;
	}
}

#endif
