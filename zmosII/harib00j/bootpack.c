#include <stdio.h>
#include "bootpack.h"

extern struct FIFO8 keyfifo;
extern struct FIFO8 mousefifo;

void wait_KBC_sendready(void)
{
	for(;;){
		/*io_in8(PORT)KEYSTA) will return 0x10 If keyboard is not reay*/
		if( (io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0){
			break;
		}
	}
	return;
}


void init_keyboard(void)
{
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD,KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT,KBC_MODE);
	return;
}

void enable_mouse(void)
{
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD,KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT,MOUSECMD_ENABLE);
	return;
}


void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADDR_BOOTINFO;
	char s[10];
	unsigned char keybuff[36],mousebuff[36];
	
	init_gdtidt();
	init_pic();
	
	init_keyboard();
	enable_mouse();
	io_sti();
	init_palette();
	init_screen(binfo);
	init_mouse(binfo->vram,binfo->scrnx,100,100,COL8_008484);
	fifo8_init(&keyfifo,keybuff,36);
	fifo8_init(&mousefifo,mousebuff,36);
	
	
	
	
	io_out8(PIC0_IMR, 0xf9); /* PIC0(主PIC) 开放IRQ-1 IRQ-2 (11111001) */
	io_out8(PIC1_IMR, 0xef); /* PIC1(从PIC) 开放IRQ-12 (11101111) */
	
	unsigned data;
	int keyBufDataIndex;
	while(1){
		io_cli();
		if( (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) )== 0){
			io_stihlt();
		}else{
			if( fifo8_status(&keyfifo) != 0){
				data = fifo8_get(&keyfifo);
				io_sti();
			
				boxfill8(binfo->vram, binfo->scrnx, COL8_000000, 0, 16, 15,31);
				sprintf(s,"%02X",data);
				putfont8_string(binfo->vram,binfo->scrnx,0,16,COL8_FFFFFF,s );
			}else if( fifo8_status(&mousefifo) != 0){
				data = fifo8_get(&mousefifo);
				io_sti();
			
				boxfill8(binfo->vram, binfo->scrnx, COL8_000000, 32, 16, 47,31);
				sprintf(s,"%02X",data);
				putfont8_string(binfo->vram,binfo->scrnx,32,16,COL8_FFFFFF,s );
			}
		}
	}
}

