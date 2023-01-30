;    Written by ComputerHunter based on Windows 2.11's FONTS.ASM
;                  for Windows 1.0 Alpha Release
;
; Note: This will not work on Windows 1.0 Premiere Edition or later
;          but it should be compatible with Beta Release

;***************************************************************************
;                                                                          *
;   fonts.asm                                                              *
;   High Resolution Font Definitions                                       *
;   Windows Alpha Release                                                  *
;   Copyright (C) 1985 by Microsoft Inc.                                   *
;                                                                          *
;***************************************************************************

        title   High Resolution Font Definitions
        %out    fonts
        page    ,132



;       System font height should be less than or equal to the
;       close box and the size box bitmaps heights.



OEM     segment public


;       OEM Font Description

OEMFixed        dw      12              ;lfheight
                dw      0               ;lfwidth
                dw      0               ;lfescapement
                dw      0               ;character orientation
                dw      0               ;weight
                db      0               ;Italic
                db      0               ;underline
                db      0               ;strikeout
                db      255             ;charset
                db      0               ;output precision
                db      2               ;clip precision
                db      2               ;quality
                db      1               ;pitch
                db      0

;       Ansi Fixed Font Description

AnsiFixed       dw      12              ;lfheight
                dw      0               ;lfwidth
                dw      0               ;lfescapement
                dw      0               ;character orientation
                dw      0               ;weight
                db      0               ;Italic
                db      0               ;underline
                db      0               ;strikeout
                db      0               ;charset
                db      0               ;output precision
                db      2               ;clip precision
                db      2               ;quality
                db      1               ;pitch
                db      0

;	Ansi Variable Pitch Font Definition

AnsiVar         dw      12              ;lfheight
                dw      0               ;lfwidth
                dw      0               ;lfescapement
                dw      0               ;character orientation
                dw      0               ;weight
                db      0               ;Italic
                db      0               ;underline
                db      0               ;strikeout
                db      0               ;charset
                db      0               ;output precision
                db      2               ;clip precision
                db      2               ;quality
                db      2               ;pitch
                db      0

OEM     ends
end
