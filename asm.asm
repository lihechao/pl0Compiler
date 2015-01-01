.386
.model flat,stdcall
option casemap:none
include masm32\include\io32.inc
include masm32\include\masm32.inc
includelib masm32\lib\masm32.lib
.data
szPause	byte	"Press Enter to continue",0
.code
start:
;ENTER			main
@@main:
	push	ebp
	mov	ebp,esp
	push	ebx
	push	esi
	push	edi
	sub	esp,16
;MOV	#0		y
	mov	eax,0
;WRITE		y
	WriteSDecDword	eax
	WriteCrlf
;JNE	x		  x		lab1
	mov	ecx,[ebp-16]
	cmp	ecx,ecx
	mov	[ebp-20],eax
	jne	@@lab1
;MOV	#10		y
	mov	eax,10
;SETLAB				lab1
	mov	[ebp-20],eax
@@lab1:
;WRITE		y
	mov	eax,[ebp-20]
	WriteSDecDword	eax
	WriteCrlf
;LEAVE			main
	mov	ebx,[ebp-4]
	mov	esi,[ebp-8]
	mov	edi,[ebp-12]
	mov	esp,ebp
	pop	ebp
	WriteString	szPause
	invoke StdIn, addr szPause,0
	ret


end start
