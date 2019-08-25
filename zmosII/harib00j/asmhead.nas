; haribote-os boot asm
; TAB=4


;0x101  ....640 * 480 * 8bit
;0x103  ....800 * 600 * 8bit
;0x105  ....1024 * 768 * 8bit
;0x107  ....1280 * 1024 * 8bit
VBEMODE	EQU		0x101			; ����VBE �������͎� 
BOTPAK	EQU		0x00280000		; bootpack�I��?�n��
DSKCAC	EQU		0x00100000		; �f�B�X�N�L���b�V���̏ꏊ
DSKCAC0	EQU		0x00008000		; �f�B�X�N�L���b�V���̏ꏊ�i���A�����[�h�j

; BOOT_INFO
CYLS	EQU		0x0ff0			; ??���
LEDS	EQU		0x0ff1			; ??��Iled�M��
VMODE	EQU		0x0ff2			; ?�F�͎��i8�ʁC16�ʓ����j
SCRNX	EQU		0x0ff4			; �������Ix
SCRNY	EQU		0x0ff6			; �������I
VRAM	EQU		0x0ff8			; ?��?���I�N�n�n��

		ORG		0xc200			; ���̃v���O�������ǂ��ɓǂݍ��܂��̂�

; ?��?��
		;??VBE���ۉp
		;??�p�IVBE�M�����?�p INT 0x10�V�@��ʓ��� ES:DI�C��512��?
		MOV		AX,0x9000
		MOV		ES,AX
		MOV 	DI,0
		MOV 	AX,0x4f00
		INT		0x10
		CMP		AX,0x004f	;�@��VBE�p�CAX�I???��0x004f
		JNE		scrn320
		
		;??VBE�Ŗ{�CVBE�Ŗ{�ᘰ2.0�C���ٖ@�g�p���������B��̑������������C�v�L��??�B
		MOV 	AX,[ES:DI +4]
		CMP		AX,0x200			;ax ���� 0x200 ?���Ŗ{�ᘰ2.0
		JB		scrn320
		
		;??VBEMODE ���ۉp
		;�@�ʉp�CAX������0x004f�C�󊎉�ʖ͎��M�����ʓ���ES:DI
		MOV		CX,VBEMODE
		MOV 	AX,0x4f01
		INT 	0x10
		CMP		AX,0x004f
		JNE		scrn320
		
		;��ʐM��??
		CMP		BYTE [ES:DI + 0x19],8
		JNE		scrn320
		CMP		BYTE [ES:DI + 0x1b],4
		JNE		scrn320
		MOV		AX,[ES:DI]
		AND		AX,0x0080  ; 0100_0000b  ??��7�ʐ���?1
		JZ		scrn320		;�@�ʑ�7�ʐ�0�C���P������?��C�g�p��?320
		
		;?����ʐM���C�p��struct BOOTINFO
		MOV		BX,VBEMODE + 0x4000
		MOV		AX,0x4f02	
		INT		0x10
		
		MOV		BYTE [VMODE],8	; ��ʖ͎��C�Œ营8
		MOV		AX,[ES:DI + 0x12]
		MOV		[SCRNX],AX
		MOV		AX,[ES:DI + 0x14]
		MOV		[SCRNY],AX
		MOV		AX,[ES:DI + 0x28]
		MOV		WORD [VRAM],AX
		MOV		AX,[ES:DI + 0x28 + 2]
		MOV		WORD [VRAM + 2],AX
		
		
		JMP		display_setting_end
;?�u��?������?320*200*8bit		
scrn320:
		MOV		AL,0x13			; VGA�O���t�B�b�N�X�A320x200x8bit�J���[
		MOV		AH,0x00
		INT		0x10
		MOV		BYTE [VMODE],8	; ��ʃ��[�h����������iC���ꂪ�Q�Ƃ���j
		MOV		WORD [SCRNX],320
		MOV		WORD [SCRNY],200
		MOV		DWORD [VRAM],0x000a0000
display_setting_end:

; �L�[�{�[�h��LED��Ԃ�BIOS�ɋ����Ă��炤

		MOV		AH,0x02
		INT		0x16 			; keyboard BIOS
		MOV		[LEDS],AL

; ??���LPIC,�֎~���L���f
;	����AT���e���I?�i�C�@�ʗv���n��PIC�C�K?��CLI�V�O?�s�C��?�L?���k�N
;	���@?�sPIC���n��

		MOV		AL,0xff
		OUT		0x21,AL
		NOP						; ???�sOUT�w�߁C�L�������ٖ@����?�s�C���ȍݍ��������NOP
		OUT		0xa1,AL

		CLI						; �֎~CPU??�I���f

; ??A20�C�Ȏg�p1MB�ȏ�I����

		CALL	waitkbdout
		MOV		AL,0xd1
		OUT		0x64,AL
		CALL	waitkbdout
		MOV		AL,0xdf			; enable A20
		OUT		0x60,AL
		CALL	waitkbdout

; �v���e�N�g���[�h�ڍs

[INSTRSET "i486p"]				; 486�̖��߂܂Ŏg�������Ƃ����L�q

		LGDT	[GDTR0]			; ??GDT
		MOV		EAX,CR0
		AND		EAX,0x7fffffff	; bit31�ʒu0�C�֎~��?
		OR		EAX,0x00000001	; bit0�ʒu1�C��?����?�͎�
		MOV		CR0,EAX
		JMP		pipelineflush
pipelineflush:
		MOV		AX,1*8			;  ��?�ʓI�i 1# �G 1 * 8 = 1 << 3  �F0000_1000b
		MOV		DS,AX
		MOV		ES,AX
		MOV		FS,AX
		MOV		GS,AX
		MOV		SS,AX

; ��?bootpack

		MOV		ESI,bootpack	; ��
		MOV		EDI,BOTPAK		; �ړI�n
		MOV		ECX,512*1024/4  ; size
		CALL	memcpy

; ���łɃf�B�X�N�f�[�^���{���̈ʒu�֓]��

; �܂��̓u�[�g�Z�N�^����

		MOV		ESI,0x7c00		; �]����
		MOV		EDI,DSKCAC		; �]����
		MOV		ECX,512/4
		CALL	memcpy

; �c��S��

		MOV		ESI,DSKCAC0+512	; �]����
		MOV		EDI,DSKCAC+512	; �]����
		MOV		ECX,0
		MOV		CL,BYTE [CYLS]
		IMUL	ECX,512*18*2/4	; �V�����_������o�C�g��/4�ɕϊ�
		SUB		ECX,512/4		; IPL�̕�������������
		CALL	memcpy

; asmhead�I�H�쓞��?��
;	�ډ�����?bootpack

; bootpack??

		MOV		EBX,BOTPAK
		MOV		ECX,[EBX+16]
		ADD		ECX,3			; ECX += 3;
		SHR		ECX,2			; ECX /= 4;
		JZ		skip			; �]������ׂ����̂��Ȃ�
		MOV		ESI,[EBX+20]	; �]����
		ADD		ESI,EBX
		MOV		EDI,[EBX+12]	; �]����
		CALL	memcpy
skip:
		MOV		ESP,[EBX+12]	; �X�^�b�N�����l
		JMP		DWORD 2*8:0x0000001b

waitkbdout:
		IN		 AL,0x64
		AND		 AL,0x02
		JNZ		waitkbdout		; AND�̌��ʂ�0�łȂ����waitkbdout��
		RET

memcpy:
		MOV		EAX,[ESI]
		ADD		ESI,4
		MOV		[EDI],EAX
		ADD		EDI,4
		SUB		ECX,1
		JNZ		memcpy			; �����Z�������ʂ�0�łȂ����memcpy��
		RET
; memcpy�̓A�h���X�T�C�Y�v���t�B�N�X�����Y��Ȃ���΁A�X�g�����O���߂ł�������

		ALIGNB	16
GDT0:
		RESB	8				; �k���Z���N�^
		DW		0xffff,0x0000,0x9200,0x00cf	; 1#�i�C��?�ʓI
		DW		0xffff,0x0000,0x9a28,0x0047	; ���s�\�Z�O�����g32bit�ibootpack�p�j

		DW		0
GDTR0:
		DW		8*3-1
		DD		GDT0

		ALIGNB	16
bootpack:
