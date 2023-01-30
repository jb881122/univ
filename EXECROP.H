extern LPSTR roptable;
extern LPSTR RopBuffer;
extern LPSTR RopEndCode;
WORD NEAR PASCAL CompileRop(BYTE);
void NEAR PASCAL ExecuteRop(char FAR *, BYTE, BYTE);
