; naskfunc
; TAB=4

[FORMAT "WCOFF"]				; �I�u�W�F�N�g�t�@�C������郂�[�h	
[INSTRSET "i486p"]				; 486�̖��߂܂Ŏg�������Ƃ����L�q
[BITS 32]						; 32�r�b�g���[�h�p�̋@�B�����点��

; �I�u�W�F�N�g�t�@�C���̂��߂̏��

[FILE "naskfunc.nas"]			; ������

		GLOBAL	_io_hlt,_io_out8,_io_in8, _io_load_eflags,_io_cli,_io_sti, _io_store_eflags			; c���I�������C�ݔ������O���㉺��??�B
		GLOBAL  _io_stihlt
		GLOBAL  _load_gdtr, _load_idtr
		
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

		