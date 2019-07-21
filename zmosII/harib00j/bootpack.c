#include <stdio.h>
#include "bootpack.h"

extern struct FIFO8 keyfifo;

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADDR_BOOTINFO;
	
	init_gdtidt();
	init_pic();
	io_sti();
	init_palette();
	init_screen(binfo);
	init_mouse(binfo->vram,binfo->scrnx,100,100,COL8_008484);
	
	
	char s[10];
	unsigned char keybuff[36];
	fifo8_init(&keyfifo,keybuff,36);
	
	io_out8(PIC0_IMR, 0xf9); /* PIC0(主PIC) 开放IRQ-1 IRQ-2 (11111001) */
	io_out8(PIC1_IMR, 0xef); /* PIC1(从PIC) 开放IRQ-12 (11101111) */
	
	unsigned data;
	int keyBufDataIndex;
	while(1){
		io_cli();
		if(fifo8_status(&keyfifo) == 0){
			io_stihlt();
		}else{
			data = fifo8_get(&keyfifo);
			io_sti();
			boxfill8(binfo->vram, binfo->scrnx, COL8_000000, 0, 16, 15,31);
	
			sprintf(s,"%02X",data);
			putfont8_string(binfo->vram,binfo->scrnx,0,16,COL8_FFFF00,s );
			
		}
	}
}

