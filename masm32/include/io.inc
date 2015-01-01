.nolist
;io.inc

;io.inc

.nolist

;declare procedures for inputting and outputting charactor or string
	extern readc:near,readmsg:near
	extern dispc:near,dispmsg:near,dispcrlf:near
;declare procedures for inputting and outputting binary number
	extern readbb:near,readbw:near,readbd:near
	extern dispbb:near,dispbw:near,dispbd:near
;declare procedures for inputting and outputting hexadecimal number
	extern readhb:near,readhw:near,readhd:near
	extern disphb:near,disphw:near,disphd:near
;declare procedures for inputting and outputting unsigned integer number
	extern readuib:near,readuiw:near,readuid:near
	extern dispuib:near,dispuiw:near,dispuid:near
;declare procedures for inputting and outputting signed integer number
	extern readsib:near,readsiw:near,readsid:near
	extern dispsib:near,dispsiw:near,dispsid:near
;declare procedures for outputting registers
	extern disprb:near,disprw:near,disprd:near,disprf:near

ReadSDecDword MACRO data
call readsid
mov data, eax
ENDM
ReadSDecWord MACRO data
call readsiw
mov data, ax
ENDM
ReadSDecByte MACRO data
call readsib
mov data, al
ENDM

WriteSDecDword MACRO data
push eax
mov eax, data
call dispsid
pop eax
endm
WriteSDecWord MACRO data
push eax
mov ax, data
call dispsiw
pop eax
endm
WriteSDecByte MACRO data
push eax
mov al, data
call dispsib
pop eax
endm

ReadUDecDword MACRO data
call readuid
mov data, eax
ENDM
ReadUDecWord MACRO data
call readuiw
mov data, ax
ENDM
ReadUDecByte MACRO data
call readuib
mov data, al
ENDM

WriteUDecDword MACRO data
push eax
mov eax, data
call dispuid
pop eax
endm
WriteUDecWord MACRO data
push eax
mov ax, data
call dispuiw
pop eax
endm
WriteUDecByte MACRO data
push eax
mov al, data
call dispuib
pop eax
endm

ReadHexByte MACRO data
call readhb
mov data, al
ENDM
ReadHexWord MACRO data
call readhw
mov data, ax
ENDM
ReadHexDword MACRO data
call readhd
mov data, eax
ENDM

WriteHexDword MACRO data
push eax
mov eax, data
call disphd
pop eax
endm
WriteHexWord MACRO data
push eax
mov ax, data
call disphw
pop eax
endm
WriteHexByte MACRO data
push eax
mov al, data
call disphb
pop eax
endm

ReadBinByte MACRO data
call readbb
mov data, al
ENDM
ReadBinWord MACRO data
call readbw
mov data, ax
ENDM
ReadBinDword MACRO data
call readbd
mov data, eax
ENDM

WriteBinDword MACRO data
push eax
mov eax, data
call dispbd
pop eax
endm
WriteBinWord MACRO data
push eax
mov ax, data
call dispbw
pop eax
endm
WriteBinByte MACRO data
push eax
mov al, data
call dispbb
pop eax
endm

ReadChar macro data
call readc
mov data, al
endm

WriteChar macro data
push eax
mov  al,data
call dispc
pop eax
endm

ReadString macro string
push eax
lea eax, string
call readmsg
pop eax
endm

WriteString macro string
push eax
lea eax, string
call dispmsg
pop eax
endm

WriteCrlf macro
local string
.data
string byte 0dh,0ah,0
.code
WriteString string
endm

WriteRegDword macro
call disprd
endm

WriteFlags macro
call disprf
endm

.list
