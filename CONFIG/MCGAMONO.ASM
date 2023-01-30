include cmacros.inc


sBegin Data

dsOldMode db 03h
dsPattern db 8 dup(0)

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
	mov ax,0011h
	int 10h

	; Check that it was set
	mov ah,0Fh
	int 10h
	cmp al,11h

	; Return 0 for failure if it was not
	mov ax,0
	jne dsSMExit

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
cProc dsSetPixel,<PUBLIC,PASCAL>,<ax,bx,cx,dx,es>
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

	; Set ES=Display
	mov ax,0A000h
	mov es,ax

	; Set BX=Offset of pixel
	; Offset: x / 8 + 80 * y
	mov cl,3
	shr bx,cl
	mov ax,y
	mov cl,4
	shl ax,cl
	add bx,ax
	shl ax,1
	shl ax,1
	add bx,ax

	; Place pixel
	and es:[bx],dl
	or es:[bx],ch
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

	; Set ES=Display
	mov ax,0A000h
	mov es,ax

	; Set BX=Offset of pixel
	; Offset: x / 8 + 80 * y
	mov cl,3
	shr bx,cl
	mov ax,y
	mov cl,4
	shl ax,cl
	add bx,ax
	shl ax,1
	shl ax,1
	add bx,ax

	; Place pixel into DH
	mov dh,es:[bx]

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
	xor ah,ah
	lodsb
	mov bx,ax
	lodsb
	add bx,ax
	lodsb
	add bx,ax
	mov cl,9
	shr bx,cl
	inc si
	lodsb
	mov dx,ax
	lodsb
	add dx,ax
	lodsb
	add dx,ax
	and dh,2
	add bl,dh

	; Copy the brush bits so 1 = White, 0 = Black
	mov di,offset dsPattern
	mov cx,4
	inc si
	shl bl,1
	mov bx,cs:[bx+offset dsPCJumpTable]
	jmp bx

	dsPCJumpTable:
	dw offset dsPCBlackBlack
	dw offset dsPCWhiteBlack
	dw offset dsPCBlackWhite
	dw offset dsPCWhiteWhite

	; Both colors black: Just copy black bits
	dsPCBlackBlack:
	xor ax,ax
	rep stosw
	jmp short dsPCColorsCont

	; 0 = White, 1 = Black: NOT the bits as they're copied
	dsPCWhiteBlack:
	lodsw
	not ax
	stosw
	loop dsPCWhiteBlack
	jmp short dsPCColorsCont

	; 1 = White, 0 = Black: Good! Just copy over the bits
	dsPCBlackWhite:
	rep movsw
	jmp short dsPCColorsCont

	; Both colors white: Just copy white bits
	dsPCWhiteWhite:
	mov ax,0FFFFh
	rep stosw
	; jmp short dsPCColorsCont

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
	mov cx,yOrg
	mov ax,cx
	and cx,07h
	add si,cx

	; Set DI=Offset of pixel
	; Offset: x / 8 + 80 * y
	mov di,xOrg
	mov cl,3
	shr di,cl
	; mov ax,yOrg ; Already there
	mov cl,4
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
	lodsb

	; If fullOctets is negative, each row is within a single octet
	test fullOctets,8000h
	jz dsPCYPlaceFirstPixels
	mov bx,0FFFFh
	mov cl,pixelsBefore
	shl bh,cl
	mov cl,pixelsAfter
	shr bl,cl
	or bl,bh
	mov ah,byte ptr es:[di]
	and ah,bl
	not bl
	and al,bl
	or al,ah
	mov byte ptr es:[di],al
	jmp short dsPCYContLoop

	; Place first few pixels if line doesn't start on octet boundary
	dsPCYPlaceFirstPixels:
	test pixelsBefore,0FFh
	jz dsPCYCopyFullOctets
	push ax
	mov ah,0FFh
	mov cl,pixelsBefore
	shl ah,cl
	not ah
	and al,ah
	not ah
	and ah,byte ptr es:[di]
	or al,ah
	mov byte ptr es:[di],al
	inc di
	pop ax

	; Place all pixels that are within full octets
	dsPCYCopyFullOctets:
	mov cx,fullOctets
	rep stosb

	; Place last few pixels if line doesn't end on octet boundary
	test pixelsAfter,0FFh
	jz dsPCYContLoop
	mov ah,0FFh
	mov cl,pixelsAfter
	shr ah,cl
	not ah
	and al,ah
	not ah
	and ah,byte ptr es:[di]
	or al,ah
	mov byte ptr es:[di],al

	; Set up registers and variables for next iteration
	dsPCYContLoop:
	cmp si,offset dsPattern + 8
	jb dsPCYNoPatOverflow
	mov si,offset dsPattern
	dsPCYNoPatOverflow:
	pop di
	add di,80
	pop cx
	loop dsPCYLoop
cEnd

sEnd Code

END
