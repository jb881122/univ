include cmacros.inc


sBegin Data

dsOldMode db 03h
dsSegment dw 0000h
dsCombinations db 020h,0DAh,0BFh,0DCh,0C0h,0DEh,02Fh,0C9h,0D9h,05Ch,0DDh,0BBh,0DFh,0C8h,0CCh,0DBh
dsPixels db 4000 dup(0)

sEnd Data


sBegin Code
assumes cs,Code
assumes ds,Data

; dsSetMode: Saves the current mode if needed and sets the display mode of the
;            graphics adapter
;
; Inputs:  None
; Outputs: AX = 1 if success
;          AX = 0 if failure
cProc dsSetMode,<PUBLIC,PASCAL>,<cx,di,es>
cBegin
	; Store old mode
	mov ah,0Fh
	int 10h
	mov dsOldMode,al

	; Does the mode need to be set?
	cmp al,07h
	je dsSMModeMono
	cmp al,03h
	je dsSMModeColor

	; Set 80x25 text mode
	mov ax,0003h
	int 10h

	; Check that it was set
	mov ah,0Fh
	int 10h
	cmp al,03h

	; Return 0 for failure if it was not
	mov ax,0
	jne dsSMExit

	; Set segment for color text mode
	dsSMModeColor:
	mov ax,0B800h
	jmp short dsSMNoSetMode

	; Set segment for mono text mode
	dsSMModeMono:
	mov ax,0B000h

	dsSMNoSetMode:
	mov dsSegment,ax
	mov es,ax
	xor di,di
	mov ax,0720h
	mov cx,2000
	rep stosw

	; Return 1 for success
	dsSMSuccess:
	mov ax,1

	dsSMExit:
cEnd

; dsResetMode: Restores the old display mode
;
; Inputs:  None
; Outputs: None
cProc dsResetMode,<PUBLIC,PASCAL>,<ax>
cBegin
	; Set old mode
	xor ah,ah
	mov al,dsOldMode
	int 10h
cEnd

; dsSetPixel: Sets the given pixel to the specified color
;
; Inputs:  x = X-coordinate of pixel
;          y = Y-coordinate of pixel
;          color = Color to set pixel
;              off_color = Low word: low byte is R and high byte is G
;              seg_color = High word: low byte is B
; Outputs: None
cProc dsSetPixel,<PUBLIC,PASCAL>,<ax,bx,cx,es>
	parmW x
	parmW y
	parmD color
cBegin
	; Zero here used to zero-extend the channel values
	xor ch,ch

	; Fetch red & green values first
	mov bx,off_color

	; Set AX to the red value
	mov cl,bl
	mov ax,cx

	; Add the green value to AX
	mov cl,bh
	add ax,cx

	; Fetch the blue value
	mov bx,seg_color

	; Add the blue value to AX
	; AX=Sum of channel values
	mov cl,bl
	add ax,cx

	; CH=1 if pixel set (sum >= 0x200), 0 if not
	shr ah,1
	and ah,1
	mov ch,ah

	; Fetch X-value
	mov bx,x

	; Shift pixel to correct bit
	mov cl,bl
	and cl,7
	neg cl
	add cl,7
	shl ch,cl

	; Make mask byte
	mov dl,0FEh
	rol dl,cl

	; Set BX=Offset of pixel
	; Offset: x / 8 + 40 * y
	mov cl,3
	shr bx,cl
	mov ax,y
	shl ax,cl
	add bx,ax
	shl ax,1
	shl ax,1
	add bx,ax

	; Place pixel
	and dsPixels[bx],dl
	or dsPixels[bx],ch

	; Write pixel to screen if applicable
	mov al,byte ptr x
	and al,03h
	or al,al
	jz dsSPExit
	cmp al,03h
	je dsSPExit
	mov al,byte ptr y
	and al,03h
	or al,al
	jz dsSPExit
	cmp al,03h
	je dsSPExit

	; Set ES=Display
	mov ax,dsSegment
	mov es,ax

	; Set BX=Offset of character
	mov ax,y
	and al,0FCh
	shl ax,cl
	mov bx,ax
	shl ax,1
	shl ax,1
	add bx,ax
	mov ax,x
	shr ax,1
	and al,0FEh
	add bx,ax

	; Get character value
	mov al,byte ptr es:[bx]
	xor dh,dh
	or ch,ch
	jz dsSPPixelZero
	mov dh,8
	dsSPPixelZero:
	push bx
	mov bx,0FFFFh
	mov cx,16
	dsSPCombLoop:
	inc bx
	mov ah,dsCombinations[bx]
	cmp ah,al
	loopne dsSPCombLoop

	; Determine new character value
	mov cl,byte ptr x
	shr cl,1
	and cl,1
	mov al,byte ptr y
	and al,2
	or cl,al
	shr dh,cl
	mov dl,0F7h
	sar dl,cl
	and bl,dl
	or bl,dh
	mov al,dsCombinations[bx]

	; Write the new character
	pop bx
	mov byte ptr es:[bx],al

	dsSPExit:
cEnd

; dsGetPixel: Gets the color of the specified pixel
;
; Inputs:  x = X-coordinate of pixel
;          y = Y-coordinate of pixel
; Outputs: AL = Red intensity
;          AH = Green intensity
;          DL = Blue intensity
;          DH = 0
cProc dsGetPixel,<PUBLIC,PASCAL>,<bx,cx,es>
	parmW x
	parmW y
cBegin
	; Get shift amount+1 (for carry bit)
	mov bx,x
	mov ch,bl
	and ch,7
	neg ch
	add ch,8

	; Set BX=Offset of pixel
	; Offset: x / 8 + 40 * y
	mov cl,3
	shr bx,cl
	mov ax,y
	shl ax,cl
	add bx,ax
	shl ax,1
	shl ax,1
	add bx,ax

	; Place pixel into DH
	mov dh,dsPixels[bx]

	; Shift pixel into carry bit
	mov cl,ch
	shr dh,cl

	; AX=0xFFFF if pixel set, 0x0000 otherwise
	sbb ax,ax

	; DX as well
	cwd

	; Set DH=0, unused but part of what's returned
	xor dh,dh
cEnd

; dsTextOut: Outputs the specified text to the screen at the specified coordinates
;
; Inputs:  x = Left x-coordinate of string in characters
;          y = Top y-coordinate of string in characters
;          count = Number of characters to print
;          string -> String of characters to print
; Outputs: None
cProc dsTextOut,<PUBLIC,PASCAL>,<ax,cx,si,di,ds,es>
	parmW x
	parmW y
	parmW count
	parmD string
cBegin
	; Set ES=Display
	mov ax,dsSegment
	mov es,ax

	; Set DI=Offset of first character
	mov ax,y
	mov cl,5
	shl ax,cl
	mov di,ax
	shl ax,1
	shl ax,1
	add di,ax
	mov ax,x
	shl ax,1
	add di,ax

	; Set DS:SI=Pointer to string
	lds si,string
	assumes ds,nothing

	; Display the characters
	mov cx,count
	dsTOLoop:
	movsb
	inc di
	loop dsTOLoop
cEnd
assumes ds,Data

sEnd Code

END
