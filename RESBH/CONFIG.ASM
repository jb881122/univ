;***************************************************************************
;                                                                          *
;   config.asm                                                             *
;   Hardware Dependent Parameters                                          *
;   Windows Beta Release                                                   *
;   Copyright (C) 1985 by Microsoft Inc.                                   *
;                                                                          *
;***************************************************************************

        title   Hardware Dependent Parameters
        %out    config
        page    ,132

OEM     segment public

;       Machine dependent parameters

        dw      18                      ;Height of vertical thumb
        dw      20                      ;Width of horizontal thumb
        dw      2                       ;Icon horiz compression factor
        dw      2                       ;Icon vert compression factor
        dw      1                       ;Cursor horz compression factor
        dw      1                       ;Cursor vert compression factor
        dw      0                       ;Kanji window height
        dw      1                       ;cxBorder (thickness of vertical lines)
        dw      1                       ;cyBorder (thickness of horizontal lines)

;       Default system color values

        db      0C0h,0C0h,0C0h          ;Scroll bar body
        db      0
        db      080h,080h,080h          ;Desktop
        db      0
        db      000h,000h,000h          ;Active caption
        db      0
        db      080h,080h,080h          ;Inactive caption
        db      0
        db      0FFh,0FFh,0FFh          ;Menu background
        db      0
        db      0FFh,0FFh,0FFh          ;Window background
        db      0
        db      000h,000h,000h          ;Caption
        db      0
        db      000h,000h,000h          ;Menu text
        db      0
        db      000h,000h,000h          ;Window text
        db      0
        db      0FFh,0FFh,0FFh          ;Caption text
        db      0

        dw      0                       ;Unused words
        dw      0
        dw      0
        dw      0
        dw      0
        dw      0
        dw      0
        dw      0

OEM     ends
end
