; naskfunc
; TAB=4

[FORMAT "WCOFF"]				; �I�u�W�F�N�g�t�@�C������郂�[�h	
[INSTRSET "i486p"]				; 486�̖��߂܂Ŏg�������Ƃ����L�q
[BITS 32]						; 32�r�b�g���[�h�p�̋@�B�����点��

; �I�u�W�F�N�g�t�@�C���̂��߂̏��

[FILE "naskfunc.nas"]			; ������

		GLOBAL	_io_hlt,_io_out8, _io_load_eflags,_io_cli, _io_store_eflag			; c���I�������C�ݔ������O���㉺??�B

; �ȉ��͎��ۂ̊֐�

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
		pop eax  ;eax �����ԉ�?
		ret

_io_cli:		;void io_cli
		cli
		ret

_io_store_eflag:	;void io_store_eflag(int eflags)
		mov eax,[esp + 4]
		push eax
		popfd
		ret