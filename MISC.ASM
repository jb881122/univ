include cmacros.inc


sBegin Code
assumes cs,Code

; msCreateFile: Uses DOS to create the specified file
;
; Inputs:  lpFileName -> String containing the name of the file to create
; Outputs: AX = A handle to the created file (negative if error)
cProc msCreateFile,<PUBLIC,PASCAL>,<cx,dx,ds>
	parmD lpFileName
cBegin
	mov ah,3Ch
	xor cx,cx
	lds dx,lpFileName
	int 21h
	jnc msCFSuccess
	neg ax
	msCFSuccess:
cEnd

; msOpenFile: Uses DOS to open the specified file
;
; Inputs:  lpFileName -> String containing the name of the file to open
; Outputs: AX = A handle to the opened file (negative if error)
cProc msOpenFile,<PUBLIC,PASCAL>,<dx,ds>
	parmD lpFileName
cBegin
	mov ax,3D00h
	lds dx,lpFileName
	int 21h
	jnc msOFSuccess
	neg ax
	msOFSuccess:
cEnd

; msReadFile: Uses DOS to read the specified file
;
; Inputs:  handle = The handle of the file to read
;          len = The length of the buffer to read to
;          lpBuffer -> Buffer to read to
; Outputs: AX = Number of bytes read
cProc msReadFile,<PUBLIC,PASCAL>,<bx,cx,dx,ds>
	parmW handle
	parmW len
	parmD lpBuffer
cBegin
	mov ah,3Fh
	mov bx,handle
	mov cx,len
	lds dx,lpBuffer
	int 21h
	jnc msRFSuccess
	xor ax,ax
	msRFSuccess:
cEnd

; msWriteFile: Uses DOS to write to the specified file
;
; Inputs:  handle = The handle of the file to write
;          len = The length of the string to write
;          lpString -> String to write
; Outputs: AX = 0 if success, 1 if error
cProc msWriteFile,<PUBLIC,PASCAL>,<bx,cx,dx,ds>
	parmW handle
	parmW len
	parmD lpString
cBegin
	mov ah,40h
	mov bx,handle
	mov cx,len
	lds dx,lpString
	int 21h
	mov ax,0
	jnc msWFSuccess
	inc ax
	msWFSuccess:
cEnd

; msCloseFile: Uses DOS to close the specified file
;
; Inputs:  handle = The handle of the file to close
; Outputs: None
cProc msCloseFile,<PUBLIC,PASCAL>,<ax,bx>
	parmW handle
cBegin
	mov ah,3Eh
	mov bx,handle
	int 21h
cEnd

; msDeleteFile: Uses DOS to delete the specified file
;
; Inputs:  lpFileName -> String containing the name of the file to delete
; Outputs: None
cProc msDeleteFile,<PUBLIC,PASCAL>,<ax,dx,ds>
	parmD lpFileName
cBegin
	mov ah,41h
	lds dx,lpFileName
	int 21h
cEnd

sEnd Code

END
