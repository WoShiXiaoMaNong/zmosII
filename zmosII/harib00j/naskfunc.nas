; naskfunc
; TAB=4

[FORMAT "WCOFF"]				; オブジェクトファイルを作るモード	
[INSTRSET "i486p"]				; 486の命令まで使いたいという記述
[BITS 32]						; 32ビットモード用の機械語を作らせる

; オブジェクトファイルのための情報

[FILE "naskfunc.nas"]			; 文件名

		GLOBAL	_io_hlt,_io_out8,_io_in8, _io_load_eflags,_io_cli,_io_sti, _io_store_eflags			; c中的函数名，在函数名前加上下下??。
		GLOBAL  _io_stihlt
		GLOBAL  _load_gdtr, _load_idtr, _load_cr0, _store_cr0, _load_tr
		GLOBAL  _memtest_sub
		GLOBAL  _farjmp
		
		;中断函数
		GLOBAL _asm_inthandler20,_asm_inthandler21, _asm_inthandler2c

		EXTERN _inthandler20,_inthandler21,_inthandler2c
		
; 以下は実際の関数

[SECTION .text]	
	
_io_hlt:	; void io_hlt(void);
		HLT
		RET
		
_io_stihlt:
		STI
		HLT
		RET
		
_io_out8:	;void io_out8(int port,int value);
		mov edx,[esp + 4]
		mov al,[esp + 8]
		out dx,al
		ret
		
_io_in8:	;char io_in8(int port);
		mov dx,[esp + 4]
		in al,dx
		ret
		
_io_load_eflags:  ;int io_load_eflags(void);
		pushfd
		pop eax  ;eax 存放返回?
		ret

_io_cli:		;void io_cli
		cli
		ret
_io_sti:		;void io_sti
		sti
		ret
		
_io_store_eflags:	;void io_store_eflag(int eflags)
		mov eax,[esp + 4]
		push eax
		popfd
		ret
		
_load_cr0:
		mov eax,cr0
		ret

_store_cr0:
		mov eax,[esp + 4]
		mov cr0,eax
		ret

_load_tr: ;void load_tr(int tr)
		ltr [ESP + 4]
		ret
		
_load_gdtr: ;void load_gdtr(int limit, int addr);
	mov ax,[esp + 4]  ;limit
	mov [esp + 6],ax  ;第一个参数?esp + 4,第二个参数? esp + 8
	lgdt [esp + 6]    ;limit ?2字?,base addr?4字?,将limit mov到base addr（esp + 8）前面的?个字?（esp + 6）?成lgdt所需要的格式
	ret
	
_load_idtr: ;void load_idtr(lint limit,int addr);
	mov ax,[esp + 4] ;limit
	mov [esp + 6],ax ;同load_gdtr
	lidt [esp + 6]
	ret

_asm_inthandler20:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler20
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

_asm_inthandler21:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler21
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

_asm_inthandler2c:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler2c
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD		

_memtest_sub: ; int memtest_sub(unsigned start,unsigned end);
		PUSH EDI
		PUSH ESI
		PUSH EBX
		MOV ESI,0xaa55aa55
		MOV EDI,0x55aa55aa
		MOV EAX,[ESP + 4*3 + 4]  ;由于前面有三次push，所有esp��?了 4 * 3 个字?。
		
	mts_loop:
		MOV EBX,EAX
		ADD EBX,0xffc   ;addr
		MOV EDX,[EBX]   ;origin value
		MOV [EBX],ESI   ;WRITE VALUE
		XOR dword [EBX],0xffffffff
		cmp edi,[ebx]
		jnz mts_fin
		xor dword [ebx],0xffffffff
		cmp esi,[ebx]
		jnz mts_fin
		
		mov [ebx],edx
		add eax,0x1000
		cmp eax,[esp + 12 + 8]
		jbe mts_loop
		pop ebx
		pop esi
		pop edi
		ret		
		
	mts_fin:
		mov [ebx],edx
		pop ebx
		pop esi
		pop edi
		ret	
		
_farjmp: 		;void farjmp(int eip, int cs)
		jmp	far [esp + 4]   ; [esp + 4] -> eip,  [esp + 8] -> cs
		ret
		