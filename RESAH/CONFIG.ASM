;    Written by ComputerHunter based on Windows 2.11's CONFIG.ASM
;                   for Windows 1.0 Alpha Release
;
;   Note: This will not work on Windows 1.0 Beta Release or later

;***************************************************************************
;                                                                          *
;   config.asm                                                             *
;   Hardware Dependent Parameters                                          *
;   Windows Alpha Release                                                  *
;   Copyright (C) 1985 by Microsoft Inc.                                   *
;                                                                          *
;***************************************************************************

        title   Hardware Dependent Parameters
        %out    config
        page    ,132

OEM     segment public

;       Machine dependent parameters

        dw      24                      ;Height of vertical thumb
        dw      24                      ;Width of horizontal thumb
        dw      2                       ;Icon horiz compression factor
        dw      2                       ;Icon vert compression factor
        dw      1                       ;Cursor horz compression factor
        dw      1                       ;Cursor vert compression factor
;        dw      0
;        dw      0

OEM     ends
end
