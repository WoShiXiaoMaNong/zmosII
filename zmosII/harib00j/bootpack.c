#include <stdio.h>
#include "bootpack.h"

extern struct KEYBUF keybuf;

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
	
	unsigned data;
	int keyBufDataIndex;
	while(1){
		io_cli();
		if(keybuf.next == 0){
			io_stihlt();
		}else{
			data = keybuf.data[0];
			keybuf.next --;
			for(keyBufDataIndex = 0 ; keyBufDataIndex < keybuf.next ;keyBufDataIndex ++){
				keybuf.data[keyBufDataIndex] = keybuf.data[keyBufDataIndex + 1];
			}
			io_sti();
			boxfill8(binfo->vram, binfo->scrnx, COL8_000000, 0, 16, 15,31);
	
			sprintf(s,"%02X",data);
			putfont8_string(binfo->vram,binfo->scrnx,0,16,COL8_FFFF00,s );
			
		}
	}
}

