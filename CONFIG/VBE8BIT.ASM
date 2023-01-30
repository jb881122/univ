include cmacros.inc


sBegin Data

dsOldMode db 03h
dsModeNumber dw 0000h
dsWidth dw 0000h
dsHeight dw 0000h
dsGranularity dw 0000h
dsFail db 01h
dsCurrentBank dw 0FFFFh
dsPattern db 64 dup(0)
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
	cmp byte ptr es:[di+19h],08h ; 8-bit color?
	jne dsPRModeLoop
	mov ax,word ptr es:[di+12h] ; Exact width match?
	cmp ax,devWidth
	jne dsPRResNoMatch
	mov ax,word ptr es:[di+14h] ; Exact height match?
	cmp ax,devHeight
	je dsPRFoundMode
	dsPRResNoMatch:
	mov ax,word ptr es:[di+12h] ; Width under device's max?
	cmp ax,devWidth
	ja dsPRModeLoop
	mov ax,word ptr es:[di+14h] ; Height biggest without going over?
	cmp ax,devHeight
	ja dsPRModeLoop
	cmp ax,topHeight
	jna dsPRModeLoop
	mov topMode,cx
	mov topHeight,ax
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
	; Offset: x + dsWidth * y
	; DX=Carry (64K bank #)
	mov ax,y
	mov bx,dsWidth
	mul bx
	add ax,x
	adc dx,0
	mov bx,ax
	call dsBankSwitch

	; Place pixel
	mov es:[bx],ch

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
	; Offset: x + dsWidth * y
	; DX=Carry (64K bank #)
	mov ax,y
	mov bx,dsWidth
	mul bx
	add ax,x
	adc dx,0
	mov bx,ax
	call dsBankSwitch

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

	popf
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
	; Can't have the cursor switching banks on us
	pushf
	cli

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
	; Offset: x + dsWidth * y
	; DX=Carry (64K bank #)
	; mov di,xOrg ; Already there
	; mov ax,yOrg ; Already there
	mov dx,dsWidth
	mul dx
	add di,ax
	adc dx,0
	call dsBankSwitch

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
	call dsCheckedRepMovsb
	jmp short dsPCYContLoop

	; Place first few pixels if line doesn't start on octet boundary
	dsPCYPlaceFirstPixels:
	test pixelsBefore,0FFh
	jz dsPCYCopyFullOctets
	mov cl,pixelsBefore
	xor ch,ch
	call dsCheckedRepMovsb
	sub si,8

	; Place all pixels that are within full octets
	dsPCYCopyFullOctets:
	test fullOctets,0FFFFh
	jz dsPCYPlaceLastPixels
	mov cx,fullOctets
	dsPCYCFOLoop:
	push cx
	mov cx,8
	call dsCheckedRepMovsb
	sub si,8
	pop cx
	loop dsPCYCFOLoop

	; Place last few pixels if line doesn't end on octet boundary
	dsPCYPlaceLastPixels:
	test pixelsAfter,0FFh
	jz dsPCYContLoop
	mov cl,pixelsAfter
	xor ch,ch
	call dsCheckedRepMovsb

	; Set up registers and variables for next iteration
	dsPCYContLoop:
	pop si
	add si,8
	cmp si,offset dsPattern + 64
	jb dsPCYNoPatOverflow
	sub si,64
	dsPCYNoPatOverflow:
	mov bx,di
	pop di
	add di,dsWidth
	cmp di,bx
	jnb dsPCYNoBankSwitchBeforeLoop
	inc dx
	call dsBankSwitch
	dsPCYNoBankSwitchBeforeLoop:
	pop cx
	loop dsPCYLoop

	popf
cEnd

; dsBankSwitch: Switches the bank of memory visible
;
; Inputs:  DX = 64K Bank to switch to
; Outputs: None
dsBankSwitch proc near
	cmp dsCurrentBank,dx
	je dsBSRet
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
	dsBSRet:
	ret
dsBankSwitch endp

; dsCheckedRepMovsb: Behaves like rep movsb, but checks for a bank switch
;
; Inputs:  CX = Number of bytes to move
;          DX = Current 64K Bank
;          DS:SI -> Source bytes
;          ES:DI -> Destination bytes (DI could overflow at any time)
; Outputs: None
dsCheckedRepMovsb proc near
	; Will a bank switch be needed?
	add di,cx
	jnc dsCRMNoBankSwitch

	; Yes, split the rep movsb into two parts
	sub di,cx
	push ax
	mov ax,di
	add cx,ax
	neg ax
	xchg ax,cx
	rep movsb
	inc dx
	call dsBankSwitch
	xchg ax,cx
	rep movsb
	pop ax
	ret

	; No bank switch needed, do single rep movsb
	dsCRMNoBankSwitch:
	sub di,cx
	rep movsb
	ret
dsCheckedRepMovsb endp

sEnd Code

END
