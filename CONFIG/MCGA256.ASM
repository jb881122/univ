include cmacros.inc


sBegin Data

dsOldMode db 03h
dsPattern db 64 dup(0)

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
cProc dsSetMode,<PUBLIC,PASCAL>,<bx,cx,dx>
cBegin
	; Store old mode
	mov ah,0Fh
	int 10h
	mov dsOldMode,al

	; Set MCGA mode
	mov ax,0013h
	int 10h

	; Check that it was set
	mov ah,0Fh
	int 10h
	cmp al,13h

	; Return 0 for failure if it was not
	mov ax,0
	jne dsSMExit

	; Set the palette to RGB332

	; BH = Red, BL = Green, AH = Blue, DX = Port Number
	mov bx,0707h
	mov ah,03h
	mov dx,03C8h

	; Loop through palette entries
	mov cx,100h
	dsSMPaletteLoop:

	; Set palette index
	mov al,cl
	sub al,1
	out dx,al

	; Change port for color intensities
	inc dx

	; Set red intensity
	; 3-bit conversion: ABC -> ABCABC
	mov al,bh
	shl al,1
	shl al,1
	shl al,1
	or al,bh
	out dx,al

	; Set green intensity
	; 3-bit conversion: ABC -> ABCABC
	mov al,bl
	shl al,1
	shl al,1
	shl al,1
	or al,bl
	out dx,al

	; Set blue intensity
	; 2-bit conversion: AB -> ABABAB
	mov al,ah
	shl al,1
	shl al,1
	or al,ah
	shl al,1
	shl al,1
	or al,ah
	out dx,al

	; Change port for palette index
	dec dx

	; Decrement RGB332 intensities
	sub ah,1
	sbb bx,0
	and ah,03h
	and bx,0707h

	; Loop if we're not done
	loop dsSMPaletteLoop

	; Return 1 for success
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
	; Fetch red & green values first
	mov ax,off_color

	; Set red component
	; 3-bit conversion: ABCxxxxx -> ABCxxxxx
	and al,0E0h
	mov ch,al

	; Set green component
	; 3-bit conversion: ABCxxxxx -> xxxABCxx
	and ah,0E0h
	mov cl,3
	shr ah,cl
	or ch,ah

	; Now fetch blue value
	mov ax,seg_color

	; Set blue component
	; 2-bit conversion: ABxxxxxx -> xxxxxxAB
	and al,0C0h
	mov cl,6
	shr al,cl
	or ch,al

	; Set ES=Display
	mov ax,0A000h
	mov es,ax

	; Set BX=Offset of pixel
	; Offset: x + 320 * y
	mov ax,y
	mov cl,6
	shl ax,cl
	mov bx,ax
	shl ax,1
	shl ax,1
	add bx,ax
	add bx,x

	; Place pixel
	mov es:[bx],ch
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
	; Set ES=Display
	mov ax,0A000h
	mov es,ax

	; Set BX=Offset of pixel
	; Offset: x + 320 * y
	mov ax,y
	mov cl,6
	shl ax,cl
	mov bx,ax
	shl ax,1
	shl ax,1
	add bx,ax
	add bx,x

	; Place pixel into DH
	mov dh,es:[bx]

	; Get red intensity (0-255) in AL
	; 3-bit conversion: ABCxxxxx -> ABCABCAB
	mov ch,dh
	and ch,0E0h
	mov al,ch
	mov cl,3
	shr ch,cl
	or al,ch
	shr ch,cl
	or al,ch

	; Get green intensity (0-255) in AH
	; 3-bit conversion: xxxABCxx -> ABCABCAB
	mov ch,dh
	and ch,01Ch
	mov ah,ch
	mov cl,3
	shl ch,cl
	or ah,ch
	mov cl,6
	shr ch,cl
	or ah,ch

	; Get blue intensity (0-255) in DL
	; 2-bit conversion: xxxxxxAB -> ABABABAB
	mov ch,dh
	and ch,03h
	mov dl,ch
	mov cl,2
	shl ch,cl
	or dl,ch
	shl ch,cl
	or dl,ch
	shl ch,cl
	or dl,ch

	; Set DH=0, unused but part of what's returned
	xor dh,dh
cEnd

; dsPatCopy: Copies the specified pattern to the specified screen area
;
; Inputs:  xOrg = X-coordinate of top-left corner
;          yOrg = Y-coordinate of top-left corner
;          xExt = Width of screen area
;          yExt = Height of screen area
;          brush -> OEM_BRUSH structure to paint
; Outputs: None
cProc dsPatCopy,<PUBLIC,PASCAL>,<ax,bx,cx,dx,si,di,es>
	parmW xOrg
	parmW yOrg
	parmW xExt
	parmW yExt
	parmD brush
	localB pixelsBefore
	localB pixelsAfter
	localW fullOctets
cBegin
	; Determine each color of the brush
	push ds
	pop es
	assumes es,Data
	lds si,brush
	assumes ds,nothing

	; For each color:
	mov cx,2
	dsPCColorLoop:
	push cx

	; Set red component
	; 3-bit conversion: ABCxxxxx -> ABCxxxxx
	lodsw
	and al,0E0h
	mov dh,al

	; Set green component
	; 3-bit conversion: ABCxxxxx -> xxxABCxx
	and ah,0E0h
	mov cl,3
	shr ah,cl
	or dh,ah

	; Set blue component
	; 2-bit conversion: ABxxxxxx -> xxxxxxAB
	lodsw
	and al,0C0h
	mov cl,6
	shr al,cl
	or dh,al

	; Loop back around to get the second color
	; At the end:
	;     DH = 8-bit value of first color
	;     DL = 8-bit value of second color
	xchg dh,dl
	pop cx
	loop dsPCColorLoop

	; Copy the brush bits to a block of pixels
	mov di,offset dsPattern

	; Loop 8 times in Y for an 8-line pattern
	mov cx,8
	dsPCBrushYLoop:
	push cx

	; Update AH with new line of the brush
	lodsb
	mov ah,al

	; Loop 8 times in X for a line width of 8
	mov cx,8
	dsPCBrushXLoop:

	; Set current pattern pixel to the color specified by the brush
	shl ah,1
	jc dsPCBX2ndColor
	mov al,dh
	jmp short dsPCBXCont
	dsPCBX2ndColor:
	mov al,dl
	dsPCBXCont:
	stosb

	loop dsPCBrushXLoop

	pop cx
	loop dsPCBrushYLoop

	; Get pixelsBefore, pixelsAfter, and fullOctets set up
	dsPCColorsCont:
	mov ax,xOrg
	neg ax
	and ax,07h
	mov pixelsBefore,al
	neg ax
	add ax,xExt
	push ax
	and al,07h
	mov pixelsAfter,al
	pop ax
	mov cl,3
	sar ax,cl
	mov fullOctets,ax

	; DS:SI -> pattern pixels
	; ES = display segment
	push es
	pop ds
	assumes ds,Data
	mov ax,0A000h
	mov es,ax
	assumes es,nothing
	mov si,offset dsPattern
	mov bx,yOrg
	mov ax,bx
	and bx,07h
	mov cl,3
	shl bx,cl
	add si,bx
	mov di,xOrg
	mov bx,di
	and bx,07h
	add si,bx

	; Set DI=Offset of pixel
	; Offset: x + 320 * y
	; mov di,xOrg ; Already there
	; mov ax,yOrg ; Already there
	mov cl,6
	shl ax,cl
	add di,ax
	shl ax,1
	shl ax,1
	add di,ax

	; Start Y loop
	mov cx,yExt
	dsPCYLoop:
	push cx
	push di
	push si

	; If fullOctets is negative, each row is within a single octet
	test fullOctets,8000h
	jz dsPCYPlaceFirstPixels
	mov cx,xExt
	rep movsb
	jmp short dsPCYContLoop

	; Place first few pixels if line doesn't start on octet boundary
	dsPCYPlaceFirstPixels:
	test pixelsBefore,0FFh
	jz dsPCYCopyFullOctets
	mov cl,pixelsBefore
	xor ch,ch
	rep movsb
	sub si,8

	; Place all pixels that are within full octets
	dsPCYCopyFullOctets:
	test fullOctets,0FFFFh
	jz dsPCYPlaceLastPixels
	mov cx,fullOctets
	dsPCYCFOLoop:
	push cx
	mov cx,4
	rep movsw
	sub si,8
	pop cx
	loop dsPCYCFOLoop

	; Place last few pixels if line doesn't end on octet boundary
	dsPCYPlaceLastPixels:
	test pixelsAfter,0FFh
	jz dsPCYContLoop
	mov cl,pixelsAfter
	xor ch,ch
	rep movsb

	; Set up registers and variables for next iteration
	dsPCYContLoop:
	pop si
	add si,8
	cmp si,offset dsPattern + 64
	jb dsPCYNoPatOverflow
	sub si,64
	dsPCYNoPatOverflow:
	pop di
	add di,320
	pop cx
	loop dsPCYLoop
cEnd

sEnd Code

END
