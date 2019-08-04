#include <stdio.h>
#include "bootpack.h"



void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADDR_BOOTINFO;
	struct MOUSE_DESC mdec;
	char s[10];
	unsigned char keybuff[36],mousebuff[36];
	char cursor[16][16];
	
	init_gdtidt();
	init_pic();
	io_sti();
		
	fifo8_init(&keyfifo,keybuff,36);
	fifo8_init(&mousefifo,mousebuff,36);
	
	/*由于 init_pic的时候 禁用了所有IRQ，这里需要手动开放需要的IRQ */
	io_out8(PIC0_IMR, 0xf9); /* PIC0(主PIC) 开放IRQ-1(键盘) IRQ-2(链接 从PIC) (11111001) */
	io_out8(PIC1_IMR, 0xef); /* PIC1(从PIC) 开放IRQ-12(鼠标) (11101111) */
	init_keyboard();
	enable_mouse(&mdec);
	init_palette();
	init_screen(binfo);
	
	/* init mouse start */
	int mx,my;
	mx = binfo->scrnx / 2 - 16;
	my = binfo->scrny / 2 - 16;
	
	init_mouse_cursor8(cursor,COL8_008484);
	putblock8_8(binfo->vram,binfo->scrnx,16,16,mx,my,cursor,16);
	/* init mouse end */
	
	unsigned data;
	int keyBufDataIndex;
	unsigned char mouse_data_buf[3], mouse_phase;
	mouse_phase = 0;
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
				
				if( mouse_decode(&mdec, data) == 1){
					sprintf(s,"[lcr %4d %4d]",mdec.x,mdec.y);
					
					if( (mdec.btn & 0x01) != 0){
						s[1] = 'L';
					}
					if( (mdec.btn & 0x02) != 0){
						s[3] = 'R';
					}
					if( (mdec.btn & 0x04) != 0){
						s[2] = 'C';
					}
					
					boxfill8(binfo->vram, binfo->scrnx, COL8_000000, 32, 16, 32 + 15* 8 -1,31);
					putfont8_string(binfo->vram,binfo->scrnx,32,16,COL8_FFFFFF,s );
					
					/*repaint mouse ---> Mouse move!!!*/
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484,  mx,         my,          mx + 15, my + 15);
					mx += mdec.x;
					my += mdec.y;
					putblock8_8(binfo->vram,binfo->scrnx,16,16,mx,my,cursor,16);
					
					
				}
			}
		}
	}
}

