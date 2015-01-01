.NOLIST
;io32.inc


STD_INPUT_HANDLE EQU -10 ;输入设备
STD_OUTPUT_HANDLE EQU -11 ;输出设备

endl EQU <0dh,0ah>		; 
includelib masm32\lib\kernel32.lib

WriteConsole equ <WriteConsoleA>
ReadConsole equ <ReadConsoleA>
 
GetStdHandle PROTO,  :DWORD 
WriteConsoleA PROTO,:DWORD,:PTR BYTE, :DWORD,:PTR DWORD,:DWORD
ExitProcess PROTO, :DWORD		
ReadConsoleA PROTO,:DWORD,:PTR BYTE,:DWORD,:PTR DWORD, :DWORD              
GetConsoleMode PROTO, :DWORD,:PTR DWORD
SetConsoleMode PROTO,:DWORD,:PTR DWORD

include io.inc
includelib masm32\lib\io32.lib

.LIST
