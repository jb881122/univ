include cmacros.inc


sBegin Data

dsOldMode db 03h
dsModeNumber dw 0000h
dsWidth dw 0000h
dsHeight dw 0000h
dsGranularity dw 0000h
dsBytesPerPixel db 00h
dsFail db 01h
dsCurrentBank dw 0FFFFh
externNP _saveAddr

sEnd Data


sBegin Code
assumes cs,Code
assumes ds,Data

; dsParseRes: Interprets the mode to set based on the resolution passed to it
;
; Inputs:  devRes -> Array of WORDs:
;              Offset 00h: Requested width
;              Offset 02h: Requested height
; Outputs: AX = 1 if the resolution in the user profile should be changed
;          AX = 0 if not
cProc dsParseRes,<PUBLIC,PASCAL>,<bx,cx,es>
	parmD devRes
	localW retVal
	localW devWidth
	localW devHeight
	localW topHeight
	localW topMode
	localB topBits
cBegin
	; Yes, this was called
	mov dsFail,0

	; Set dimensions to default if 0
	mov retVal,0
	les bx,devRes
	cmp word ptr es:[bx],0
	jne dsPRWNotZero
	mov word ptr es:[bx],640
	mov retVal,1
	dsPRWNotZero:
	cmp word ptr es:[bx+2],0
	jne dsPRHNotZero
	mov word ptr es:[bx+2],480
	mov retVal,1
	dsPRHNotZero:

	; Retrieve the width and height (to free up ES)
	mov ax,word ptr es:[bx]
	mov devWidth,ax
	mov ax,word ptr es:[bx+2]
	mov devHeight,ax

	; Get SVGA info
	mov ax,4F00h
	lea di,_saveAddr
	les di,[di]
	int 10h

	; Check if the command worked
	or ah,ah
	jnz dsPRFail
	cmp word ptr es:[di],4556h
	jne dsPRFail
	cmp word ptr es:[di+2],4153h
	je dsPRGoodInfo
	dsPRFail:
	mov dsFail,1
	jmp dsPRExit
	dsPRGoodInfo:

	; Prepare for the following loop
	push ds
	lds si,es:[di+0Eh]
	assumes ds,nothing
	add di,256
	mov topHeight,0
	mov topMode,0
	mov topBits,0

	; Loop through each supported mode
	dsPRModeLoop:
	lodsw
	cmp ax,0FFFFh
	je dsPRModesDone
	mov cx,ax
	mov ax,4F01h
	int 10h
	or ah,ah ; Did the command work?
	jz dsPRModeInfoSuccess
	jmp dsPRPopAndFail
	dsPRModeInfoSuccess:
	test byte ptr es:[di],01h ; Mode supported?
	jz dsPRModeLoop
	test byte ptr es:[di],02h ; Extended info available?
	jz dsPRModeLoop
	mov bl,byte ptr es:[di+19h]
	cmp bl,24 ; 24-bit color?
	je dsPRColorMatch
	cmp bl,32 ; 32-bit color?
	jne dsPRModeLoop
	dsPRColorMatch:
	mov ax,word ptr es:[di+12h] ; Exact width match?
	cmp ax,devWidth
	jne dsPRResNoMatch
	mov ax,word ptr es:[di+14h] ; Exact height match?
	cmp ax,devHeight
	jne dsPRResNoMatch
	cmp bl,32 ; 32-bit color? (preferred because it's faster)
	je dsPRFoundMode
	jmp short dsPRResNotColor
	dsPRResNoMatch:
	mov ax,word ptr es:[di+12h] ; Width under device's max?
	cmp ax,devWidth
	ja dsPRModeLoop
	mov ax,word ptr es:[di+14h] ; Height biggest without going over?
	cmp ax,devHeight
	ja dsPRModeLoop
	cmp ax,topHeight
	jb dsPRModeLoop
	ja dsPRResNotColor
	cmp bl,topBits ; Height matches current top resolution, is color depth better?
	jna dsPRModeLoop
	dsPRResNotColor:
	mov topMode,cx
	mov topHeight,ax
	mov topBits,bl
	jmp short dsPRModeLoop

	; Done with the loop, get the best available mode if any
	dsPRModesDone:
	cmp topMode,0
	je dsPRPopAndFail
	mov ax,4F01h
	mov cx,topMode
	int 10h
	or ah,ah
	jnz dsPRPopAndFail

	; Found the correct/best mode, set our driver up to use it
	dsPRFoundMode:
	pop ds
	assumes ds,Data
	mov dsModeNumber,cx
	mov ax,word ptr es:[di+12h]
	mov dsWidth,ax
	mov ax,word ptr es:[di+14h]
	mov dsHeight,ax
	mov al,byte ptr es:[di+19h]
	mov cl,3
	shr al,cl
	mov dsBytesPerPixel,al
	mov ax,word ptr es:[di+4]
	mov dsGranularity,ax

	; And update the resolution that the rest of the driver will see
	les di,devRes
	mov ax,dsWidth
	stosw
	mov ax,dsHeight
	stosw
	jmp short dsPRExit

	dsPRPopAndFail:
	pop ds
	mov dsFail,1
	dsPRExit:
	mov ax,retVal
cEnd

; dsSetMode: Saves the current mode if needed and sets the display mode of the
;            graphics adapter
;
; Inputs:  None
; Outputs: AX = 1 if success
;          AX = 0 if failure
cProc dsSetMode,<PUBLIC,PASCAL>,<bx,cx,dx,di,es>
cBegin
	; Did dsParseRes() fail?
	xor ax,ax
	cmp dsFail,0
	jne dsSMExit

	; Store old mode
	dsSMStoreMode:
	mov ah,0Fh
	int 10h
	mov dsOldMode,al

	; Set SVGA mode
	mov ax,4F02h
	mov bx,dsModeNumber
	int 10h

	; Check that it was set
	xor bx,bx
	mov ax,4F03h
	int 10h
	cmp bx,dsModeNumber

	; Return 0 for failure if it was not
	mov ax,0
	jne dsSMExit

	; Return 1 for success
	mov ax,1
	mov dsCurrentBank,0FFFFh

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
	; Can't have the cursor switching banks on us
	pushf
	cli

	; Set ES=Display
	mov ax,0A000h
	mov es,ax

	; Set BX=Offset of pixel
	; Offset: (3 or 4) * (x + dsWidth * y)
	; DX=Carry (64K bank #)
	mov ax,y
	mov bx,dsWidth
	mul bx
	add ax,x
	adc dx,0
	mov bx,ax
	cmp dsBytesPerPixel,4
	je dsSP4Bytes

	; Three byte per pixel code
	mov cx,dx
	shl bx,1
	rcl dx,1
	add bx,ax
	adc dx,0
	add dx,cx
	cmp dsCurrentBank,dx
	je dsSP3FetchColor

	; Need to switch banks...
	call dsBankSwitch

	; Fetch color
	; AL = Red, AH = Green, CL = Blue
	dsSP3FetchColor:
	mov ax,off_color
	mov cx,seg_color

	; Place pixel
	cmp bx,0FFFEh
	jb dsSP4PlacePixel

	; Pixel divided between banks, check after each byte
	mov byte ptr es:[bx],cl
	inc bx
	jnz dsSP3NoBlueOverflow
	inc dx
	call dsBankSwitch
	dsSP3NoBlueOverflow:
	mov byte ptr es:[bx],ah
	inc bx
	jnz dsSP3NoGreenOverflow
	inc dx
	call dsBankSwitch
	dsSP3NoGreenOverflow:
	mov byte ptr es:[bx],al
	jmp short dsSPEnd

	; Four byte per pixel code
	dsSP4Bytes:
	shl bx,1
	rcl dx,1
	shl bx,1
	rcl dx,1
	cmp dsCurrentBank,dx
	je dsSP4FetchColor

	; Need to switch banks...
	call dsBankSwitch

	; Fetch color
	; AL = Red, AH = Green, CL = Blue
	dsSP4FetchColor:
	mov ax,off_color
	mov cx,seg_color

	; Place pixel
	dsSP4PlacePixel:
	mov byte ptr es:[bx],cl
	mov byte ptr es:[bx+1],ah
	mov byte ptr es:[bx+2],al

	dsSPEnd:
	popf
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
	; Can't have the cursor switching banks on us
	pushf
	cli

	; Set ES=Display
	mov ax,0A000h
	mov es,ax

	; Set BX=Offset of pixel
	; Offset: (3 or 4) * (x + dsWidth * y)
	; DX=Carry (64K bank #)
	mov ax,y
	mov bx,dsWidth
	mul bx
	add ax,x
	adc dx,0
	mov bx,ax
	cmp dsBytesPerPixel,4
	je dsGP4Bytes

	; Three byte per pixel code
	mov cx,dx
	shl bx,1
	rcl dx,1
	add bx,ax
	adc dx,0
	add dx,cx
	cmp dsCurrentBank,dx
	je dsGP3FetchColor

	; Need to switch banks...
	call dsBankSwitch

	; Fetch color
	; AL = Red, AH = Green, DL = Blue
	dsGP3FetchColor:
	cmp bx,0FFFEh
	jb dsGP4FetchColor

	; Pixel divided between banks, check after each byte
	mov cl,byte ptr es:[bx]
	inc bx
	jnz dsGP3NoBlueOverflow
	inc dx
	call dsBankSwitch
	dsGP3NoBlueOverflow:
	mov ah,byte ptr es:[bx]
	inc bx
	jnz dsGP3NoGreenOverflow
	inc dx
	call dsBankSwitch
	dsGP3NoGreenOverflow:
	mov al,byte ptr es:[bx]
	mov dl,cl
	jmp short dsGPEnd

	; Four byte per pixel code
	dsGP4Bytes:
	shl bx,1
	rcl dx,1
	shl bx,1
	rcl dx,1
	cmp dsCurrentBank,dx
	je dsGP4FetchColor

	; Need to switch banks...
	call dsBankSwitch

	; Fetch color
	; AL = Red, AH = Green, DL = Blue
	dsGP4FetchColor:
	mov dl,byte ptr es:[bx]
	mov ah,byte ptr es:[bx+1]
	mov al,byte ptr es:[bx+2]

	; Set DH=0, unused but part of what's returned
	dsGPEnd:
	xor dh,dh
	popf
cEnd

; dsBankSwitch: Switches the bank of memory visible
;
; Inputs:   DX = 64K Bank to switch to
; Outputs:  None
dsBankSwitch proc near
	push ax
	push bx
	push cx
	push dx
	mov dsCurrentBank,dx
	mov ax,dx
	mov dx,64
	mul dx
	mov cx,dsGranularity
	div cx
	mov dx,ax
	xor bx,bx
	mov ax,4F05h
	int 10h
	pop dx
	pop cx
	pop bx
	pop ax
	ret
dsBankSwitch endp

sEnd Code

END
