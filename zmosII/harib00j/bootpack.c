#include <stdio.h>
#include "bootpack.h"




void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADDR_BOOTINFO;
	struct MEMMAN *man = (struct MEMMAN *) MEMMAN_ADDR;
	struct MOUSE_DESC mdec;
	char s[30];
	unsigned char keybuff[36],mousebuff[36];
	
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
	
	
	/*初始化内存管理器*/
	unsigned int mem_total;
	mem_total = memtest(0x00400000,0xbfffffff);
	memman_init(man);
	memman_free(man,0x00001000,0x009e000);
	memman_free(man,0x00400000,mem_total - 0x00400000);
	
	/*初始化图层管理器，以及背景图层和鼠标图层*/
	struct STCTL *sheetctl = shtctl_init(man, binfo->vram, binfo->scrnx, binfo->scrny);	
	struct SHEET *sheet_back = sheet_alloc(sheetctl);
	struct SHEET *sheet_mouse = sheet_alloc(sheetctl);
	unsigned char *back_buf, mouse_buf[256];
	back_buf = memman_alloc_4k(man, binfo->scrnx * binfo->scrny);
	sheet_setbuf(sheet_back, back_buf, binfo->scrnx, binfo->scrny, -1);
	sheet_setbuf(sheet_mouse, mouse_buf, 16,16, 99);/*设置透明色号为99*/
	sheet_updown(sheetctl, sheet_back,0);
	sheet_updown(sheetctl, sheet_mouse,1);
	
	init_screen(back_buf, binfo->scrnx, binfo->scrny);
	/* init mouse start */
	int mx,my;
	mx = binfo->scrnx / 2 - 16;
	my = binfo->scrny / 2 - 16;
	init_mouse_cursor8(mouse_buf,99);/*设置透明色号为99*/
	sheet_slide(sheetctl, sheet_mouse, mx,my);
	/* init mouse end */
	
	
	/*输出内存使用信息*/
	sprintf(s,"Free Memory : %dKB",memman_total(man)/ 1024);
	putfont8_string(back_buf,binfo->scrnx,0,30,COL8_FF0000,s );	
	int memsize = memtest(0x00400000,0xbfffffff) / 1024 / 1024;
	sprintf(s,"Total Memory : %dMB",memsize);
	putfont8_string(back_buf,binfo->scrnx,0,50,COL8_FFFFFF,s );
	sheet_slide(sheetctl, sheet_back, 0,0);
	
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
			
				boxfill8(back_buf, binfo->scrnx, COL8_008484, 0, 16, 15,31);
				sprintf(s,"%02X",data);
				putfont8_string(back_buf,binfo->scrnx,0,16,COL8_FFFFFF,s );
				sheet_refresh(sheetctl);
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
					
					boxfill8(back_buf, binfo->scrnx, COL8_008484, 32, 16, 32 + 15* 8 -1,31);
					putfont8_string(back_buf,binfo->scrnx,32,16,COL8_FFFFFF,s );
				
					mx += mdec.x;
					my += mdec.y;
					sheet_slide(sheetctl, sheet_mouse, mx,my);/*移动图层，并且重新绘制*/
					
				}
			}
		}
	}
}

