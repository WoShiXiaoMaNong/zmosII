; haribote-os boot asm
; TAB=4

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

		MOV		AL,0x13			; VGAグラフィックス、320x200x8bitカラー
		MOV		AH,0x00
		INT		0x10
		MOV		BYTE [VMODE],8	; 画面モードをメモする（C言語が参照する）
		MOV		WORD [SCRNX],320
		MOV		WORD [SCRNY],200
		MOV		DWORD [VRAM],0x000a0000

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
		MOV		ECX,512*1024/   ; size
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
