include cmacros.inc


sBegin Data
assumes cs,Data

.xlist
	include ROPDEFS.BLT
	include ROPTABLE.BLT
.list

sEnd Data


sBegin Code
assumes cs,Code
assumes ds,Data

; CompileRop: Compiles the ROP code into RopBuffer
;
; Inputs:  rop = ROP table index
; Outputs: AL = SOURCE_PRESENT (0x01) | PATTERN_PRESENT (0x02)
cProc CompileRop,<PUBLIC,PASCAL>,<bx,cx,si,di,es>
	parmB rop
cBegin
	mov bl,rop
	xor bh,bh
	or bl,bl
	jns ERNo1Comp

	not bl
	shl bl,1
	mov ax,roptable[bx]
	xor ah,80h
	jmp short ERN1CCont

ERNo1Comp:
	shl bl,1
	mov ax,roptable[bx]

ERN1CCont:
	push cs
	pop es
	assumes es,Code
	mov di,offset RopBuffer
	mov si,ax
	and si,03FFh
	jz ERSkipCopy

	add si,offset roptable
	mov bx,ax
	and bx,1C00h
	mov cl,10
	shr bx,cl
	mov cl,[bx+offset roptable+100h]
	xor ch,ch
	rep movsb

ERSkipCopy:
	mov al,0EBh
	mov cl,ah

	or ah,ah
	mov ah,00h
	js ERNegate

	mov ah,02h

ERNegate:
	mov bx,di
	sub bx,offset RopEndCode-2
	neg bl
	add ah,bl
	stosw
	mov al,cl
	mov cl,5
	shr al,cl
	and al,03h

cEnd

assumes es,nothing

; ExecuteRop: Executes the code placed into RopBuffer
;
; Inputs:  lpDest = Pointer to destination byte
;          src = Source byte
;          pat = Pattern byte
; Outputs: *lpDest = Output destination byte
cProc ExecuteRop,<PUBLIC,PASCAL>,<dx,di,es>
	parmD lpDest
	parmB src
	parmB pat
cBegin
	les di,lpDest
	mov al,src
	mov dh,pat
RopBuffer:
	db 13+2 dup(0) ; 13 is max. code size + 2 bytes for short jump
RopEndCode:
	not al
errnz <$-RopEndCode-2>
	stosb
cEnd

sEnd Code

END
