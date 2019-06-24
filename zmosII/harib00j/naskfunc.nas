; naskfunc
; TAB=4

[FORMAT "WCOFF"]				; オブジェクトファイルを作るモード	
[INSTRSET "i486p"]				; 486の命令まで使いたいという記述
[BITS 32]						; 32ビットモード用の機械語を作らせる

; オブジェクトファイルのための情報

[FILE "naskfunc.nas"]			; 文件名

		GLOBAL	_io_hlt			; c?言中的函数名，在函数名前加上下??。
		GLOBAL  _write_mem8

; 以下は実際の関数

[SECTION .text]	

_io_hlt:	; void io_hlt(void);
		HLT
		RET
		
		
_write_mem8:		;write_mem8(int addr,int data)
	mov ecx,[esp + 4] ;第一个参数，即addr
	mov al, [esp + 8] ;第二个参数，即data
	mov [ecx],al    ;
	RET