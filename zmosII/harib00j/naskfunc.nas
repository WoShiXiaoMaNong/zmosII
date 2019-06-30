; naskfunc
; TAB=4

[FORMAT "WCOFF"]				; オブジェクトファイルを作るモード	
[INSTRSET "i486p"]				; 486の命令まで使いたいという記述
[BITS 32]						; 32ビットモード用の機械語を作らせる

; オブジェクトファイルのための情報

[FILE "naskfunc.nas"]			; 文件名

		GLOBAL	_io_hlt,_io_out8, _io_load_eflags,_io_cli, _io_store_eflag			; c中的函数名，在函数名前加上下??。

; 以下は実際の関数

[SECTION .text]	

_io_hlt:	; void io_hlt(void);
		HLT
		RET
		
_io_out8:	;void io_out8(int port,int value);
		mov edx,[esp + 4]
		mov eax,[esp + 8]
		out dx,al
		ret
_io_load_eflags:  ;int io_load_eflags(void);
		pushfd
		pop eax  ;eax 存放返回?
		ret

_io_cli:		;void io_cli
		cli
		ret

_io_store_eflag:	;void io_store_eflag(int eflags)
		mov eax,[esp + 4]
		push eax
		popfd
		ret