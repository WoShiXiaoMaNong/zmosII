; naskfunc
; TAB=4

[FORMAT "WCOFF"]				; �I�u�W�F�N�g�t�@�C������郂�[�h	
[INSTRSET "i486p"]				; 486�̖��߂܂Ŏg�������Ƃ����L�q
[BITS 32]						; 32�r�b�g���[�h�p�̋@�B�����点��

; �I�u�W�F�N�g�t�@�C���̂��߂̏��

[FILE "naskfunc.nas"]			; ������

		GLOBAL	_io_hlt,_io_out8,_io_in8, _io_load_eflags,_io_cli,_io_sti, _io_store_eflags			; c���I�������C�ݔ������O���㉺��??�B
		GLOBAL  _io_stihlt
		GLOBAL  _load_gdtr, _load_idtr, _load_cr0, _store_cr0
		GLOBAL  _memtest_sub
		
		
		;���f����
		GLOBAL _asm_inthandler21, _asm_inthandler2c

		EXTERN _inthandler21,_inthandler2c
		
; �ȉ��͎��ۂ̊֐�

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
		pop eax  ;eax �����ԉ�?
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

	
_load_gdtr: ;void load_gdtr(int limit, int addr);
	mov ax,[esp + 4]  ;limit
	mov [esp + 6],ax  ;��꘢�Q��?esp + 4,��񘢎Q��? esp + 8
	lgdt [esp + 6]    ;limit ?2��?,base addr?4��?,��limit mov��base addr�iesp + 8�j�O�ʓI?����?�iesp + 6�j?��lgdt�����v�I�i��
	ret
	
_load_idtr: ;void load_idtr(lint limit,int addr);
	mov ax,[esp + 4] ;limit
	mov [esp + 6],ax ;��load_gdtr
	lidt [esp + 6]
	ret

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
		MOV EAX,[ESP + 4*3 + 4]  ;�R���O�ʗL�O��push�C���Lesp��?�� 4 * 3 ����?�B
		
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

		