/* Driver description */
#define DC_DRIVERDESC VBE True Color

/* Whether the screen resolution can be changed */
#define DC_MODIFIABLE_RES 1

/* Reported screen width in pixels */
#define DC_WIDTH 640

/* Reported screen height in pixels */
#define DC_HEIGHT 480

/* Display type:
	0 = RGB Color
	1 = Grayscale
	2 = Palette */
#define DC_TYPE 0

/* Bits per pixel */
#define DC_BITS 24

/* Palette if DC_TYPE is 2 */
/* Sample 16-color palette:
#define DC_PALETTE { \
	{0, 0, 0}, \
	{0, 0, 170}, \
	{0, 170, 0}, \
	{0, 170, 170}, \
	{170, 0, 0}, \
	{170, 0, 170}, \
	{170, 85, 0}, \
	{170, 170, 170}, \
	{85, 85, 85}, \
	{85, 85, 255}, \
	{85, 255, 85}, \
	{85, 255, 255}, \
	{255, 85, 85}, \
	{255, 85, 255}, \
	{255, 255, 85}, \
	{255, 255, 255} \
}
*/

/* Does the set-mode code use the cursor save area?
   (e.g. To store the SVGA mode information) */
#define DC_USES_SAVE_AREA 1

/* Accelerator functions
	0 if not implemented
	1 if implemented */
#define DC_HAS_DSPATCOPY 0
