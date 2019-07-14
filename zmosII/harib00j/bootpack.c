#include <stdio.h>
#include "bootpack.h"




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
	int y = 4;
	//print variable test
	int test = 1;
	y = 20;
	while(test < 10){
		sprintf(s,"Para = %d",test);
		putfont8_string(binfo->vram,binfo->scrnx,20,y,COL8_FFFF00,s);
		test ++;
		y += 16;
	}
	//print variable test end
	
	io_out8(PIC0_IMR, 0xf9); /* PIC0(主PIC) 开放IRQ-1 IRQ-2 (11111001) */
	io_out8(PIC1_IMR, 0xef); /* PIC1(从PIC) 开放IRQ-12 (11101111) */
fin:
	io_hlt();
	goto fin;
}

