#if !defined(IS_CURSOR)
#if (DC_MODIFIABLE_RES == 1)
BOOL NEAR PASCAL dsParseRes(LPINT);
#endif
WORD NEAR PASCAL dsSetMode(void);
void NEAR PASCAL dsResetMode(void);
#endif

#if (OS <= W1_DR5) && defined(IS_CURSOR)
void FAR PASCAL dsSetPixel(WORD, WORD, COLOR);
DWORD FAR PASCAL dsGetPixel(WORD, WORD);
#else
void NEAR PASCAL dsSetPixel(WORD, WORD, COLOR);
DWORD NEAR PASCAL dsGetPixel(WORD, WORD);
#endif

#if (DC_HAS_DSPATCOPY == 1)
void NEAR PASCAL dsPatCopy(WORD, WORD, WORD, WORD, OEM_BRUSH FAR *);
#endif

#if (DC_HAS_DSTEXTOUT == 1)
void NEAR PASCAL dsTextOut(WORD, WORD, WORD, LPSTR);
#endif
