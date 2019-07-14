#include <stdio.h>
#include "bootpack.h"

void io_hlt(void);

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) 0x0ff0;
	
	init_gdtidt();
	init_palette();
	init_screen(binfo);
	init_mouse(binfo->vram,binfo->scrnx,100,100,COL8_008484);
	
	
	char s[10];
	int y = 4;
	//print variable test
	int test = 1;
	y = 20;
	while(test < 10){
		sprintf(s,"Params = %d",test);
		putfont8_string(binfo->vram,binfo->scrnx,20,y,COL8_FFFF00,s);
		test ++;
		y += 16;
	}
	//print variable test end
	
fin:
	io_hlt();
	goto fin;
}

