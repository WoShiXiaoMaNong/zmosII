; naskfunc
; TAB=4

[FORMAT "WCOFF"]				; �I�u�W�F�N�g�t�@�C������郂�[�h	
[INSTRSET "i486p"]				; 486�̖��߂܂Ŏg�������Ƃ����L�q
[BITS 32]						; 32�r�b�g���[�h�p�̋@�B�����点��

; �I�u�W�F�N�g�t�@�C���̂��߂̏��

[FILE "naskfunc.nas"]			; ������

		GLOBAL	_io_hlt			; c?�����I�������C�ݔ������O���㉺??�B
		GLOBAL  _write_mem8

; �ȉ��͎��ۂ̊֐�

[SECTION .text]	

_io_hlt:	; void io_hlt(void);
		HLT
		RET
		
		
_write_mem8:		;write_mem8(int addr,int data)
	mov ecx,[esp + 4] ;��꘢�Q���C��addr
	mov al, [esp + 8] ;��񘢎Q���C��data
	mov [ecx],al    ;
	RET