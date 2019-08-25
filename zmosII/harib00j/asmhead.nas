; haribote-os boot asm
; TAB=4

;========VEBMODE LIST===========
;0x101  ....640 * 480 * 8bit
;0x103  ....800 * 600 * 8bit
;0x105  ....1024 * 768 * 8bit
;0x107  ....1280 * 1024 * 8bit
;===============================
VBEMODE	EQU		0x101			; 制定VBE 分辨率模式 
BOTPAK	EQU		0x00280000		; bootpack的装?地址
DSKCAC	EQU		0x00100000		; ディスクキャッシュの場所
DSKCAC0	EQU		0x00008000		; ディスクキャッシュの場所（リアルモード）

; BOOT_INFO
CYLS	EQU		0x0ff0			; ??扇区
LEDS	EQU		0x0ff1			; ??上的led信息
VMODE	EQU		0x0ff2			; ?色模式（8位，16位等等）
SCRNX	EQU		0x0ff4			; 分辨率的x
SCRNY	EQU		0x0ff6			; 分辨率的
VRAM	EQU		0x0ff8			; ?存?存的起始地址

		ORG		0xc200			; このプログラムがどこに読み込まれるのか

; ?示?定
		;测试VBE是否可用
		;显卡可用的VBE信息会在调用 INT 0x10之后被写入到 ES:DI，共512字节
		MOV		AX,0x9000
		MOV		ES,AX
		MOV 	DI,0
		MOV 	AX,0x4f00
		INT		0x10
		CMP		AX,0x004f	;如果VBE可用，AX的值应该是0x004f
		JNE		scrn320
		
		;检查VBE版本，VBE版本低于2.0，将无法使用高分辨率。具体多少是高分辨，没有去细查。
		MOV 	AX,[ES:DI +4]
		CMP		AX,0x200			;ax 小于 0x200 说明版本低于2.0
		JB		scrn320
		
		;检查VBEMODE 是否可用
		;如果可用，AX将等于0x004f，并且画面模式信息会被写入到ES:DI
		MOV		CX,VBEMODE
		MOV 	AX,0x4f01
		INT 	0x10
		CMP		AX,0x004f
		JNE		scrn320
		
		;画面信息确认
		CMP		BYTE [ES:DI + 0x19],8
		JNE		scrn320
		CMP		BYTE [ES:DI + 0x1b],4
		JNE		scrn320
		MOV		AX,[ES:DI]
		AND		AX,0x0080  ; 0100_0000b  检查第7位是否为1
		JZ		scrn320		;如果第7位是0，放弃分辨率设定，使用默认320
		
		;储存画面信息，用于struct BOOTINFO
		MOV		BX,VBEMODE + 0x4000
		MOV		AX,0x4f02	
		INT		0x10
		
		MOV		BYTE [VMODE],8	; 画面模式，固定是8
		MOV		AX,[ES:DI + 0x12]
		MOV		[SCRNX],AX
		MOV		AX,[ES:DI + 0x14]
		MOV		[SCRNY],AX
		MOV		AX,[ES:DI + 0x28]
		MOV		WORD [VRAM],AX
		MOV		AX,[ES:DI + 0x28 + 2]
		MOV		WORD [VRAM + 2],AX
		
		
		JMP		display_setting_end
;设置默认分辨率为320*200*8bit		
scrn320:
		MOV		AL,0x13			; VGAグラフィックス、320x200x8bitカラー
		MOV		AH,0x00
		INT		0x10
		MOV		BYTE [VMODE],8	; 画面モードをメモする（C言語が参照する）
		MOV		WORD [SCRNX],320
		MOV		WORD [SCRNY],200
		MOV		DWORD [VRAM],0x000a0000
display_setting_end:

; キーボードのLED状態をBIOSに教えてもらう

		MOV		AH,0x02
		INT		0x16 			; keyboard BIOS
		MOV		[LEDS],AL

; ??所有PIC,禁止所有中断
;	根据AT兼容机的?格，如果要初始化PIC，必?在CLI之前?行，否?有?候回挂起
;	随后?行PIC初始化

		MOV		AL,0xff
		OUT		0x21,AL
		NOP						; ???行OUT指令，有些机器会无法正常?行，所以在此插入一条NOP
		OUT		0xa1,AL

		CLI						; 禁止CPU??的中断

; ??A20，以使用1MB以上的内存

		CALL	waitkbdout
		MOV		AL,0xd1
		OUT		0x64,AL
		CALL	waitkbdout
		MOV		AL,0xdf			; enable A20
		OUT		0x60,AL
		CALL	waitkbdout

; プロテクトモード移行

[INSTRSET "i486p"]				; 486の命令まで使いたいという記述

		LGDT	[GDTR0]			; ??GDT
		MOV		EAX,CR0
		AND		EAX,0x7fffffff	; bit31位置0，禁止分?
		OR		EAX,0x00000001	; bit0位置1，切?到保?模式
		MOV		CR0,EAX
		JMP		pipelineflush
pipelineflush:
		MOV		AX,1*8			;  可?写的段 1# ； 1 * 8 = 1 << 3  ：0000_1000b
		MOV		DS,AX
		MOV		ES,AX
		MOV		FS,AX
		MOV		GS,AX
		MOV		SS,AX

; 加?bootpack

		MOV		ESI,bootpack	; 源
		MOV		EDI,BOTPAK		; 目的地
		MOV		ECX,512*1024/4  ; size
		CALL	memcpy

; ついでにディスクデータも本来の位置へ転送

; まずはブートセクタから

		MOV		ESI,0x7c00		; 転送元
		MOV		EDI,DSKCAC		; 転送先
		MOV		ECX,512/4
		CALL	memcpy

; 残り全部

		MOV		ESI,DSKCAC0+512	; 転送元
		MOV		EDI,DSKCAC+512	; 転送先
		MOV		ECX,0
		MOV		CL,BYTE [CYLS]
		IMUL	ECX,512*18*2/4	; シリンダ数からバイト数/4に変換
		SUB		ECX,512/4		; IPLの分だけ差し引く
		CALL	memcpy

; asmhead的工作到此?束
;	接下来交?bootpack

; bootpack??

		MOV		EBX,BOTPAK
		MOV		ECX,[EBX+16]
		ADD		ECX,3			; ECX += 3;
		SHR		ECX,2			; ECX /= 4;
		JZ		skip			; 転送するべきものがない
		MOV		ESI,[EBX+20]	; 転送元
		ADD		ESI,EBX
		MOV		EDI,[EBX+12]	; 転送先
		CALL	memcpy
skip:
		MOV		ESP,[EBX+12]	; スタック初期値
		JMP		DWORD 2*8:0x0000001b

waitkbdout:
		IN		 AL,0x64
		AND		 AL,0x02
		JNZ		waitkbdout		; ANDの結果が0でなければwaitkbdoutへ
		RET

memcpy:
		MOV		EAX,[ESI]
		ADD		ESI,4
		MOV		[EDI],EAX
		ADD		EDI,4
		SUB		ECX,1
		JNZ		memcpy			; 引き算した結果が0でなければmemcpyへ
		RET
; memcpyはアドレスサイズプリフィクスを入れ忘れなければ、ストリング命令でも書ける

		ALIGNB	16
GDT0:
		RESB	8				; ヌルセレクタ
		DW		0xffff,0x0000,0x9200,0x00cf	; 1#段，可?写的
		DW		0xffff,0x0000,0x9a28,0x0047	; 実行可能セグメント32bit（bootpack用）

		DW		0
GDTR0:
		DW		8*3-1
		DD		GDT0

		ALIGNB	16
bootpack:
