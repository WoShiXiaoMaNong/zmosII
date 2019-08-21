#include <stdio.h>
#include "bootpack.h"

#define BUF_LENGTH 36
extern struct TIMERCTL timerctl;


void HariMain(void)
{
	int countForTest;
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADDR_BOOTINFO;
	struct MEMMAN *man = (struct MEMMAN *) MEMMAN_ADDR;
	struct MOUSE_DESC mdec;
	char s[256];
	
	
	struct FIFO32 buff_fifo;
	int buff[BUF_LENGTH];
	fifo32_init(&buff_fifo,buff,BUF_LENGTH);
	
	init_gdtidt();
	init_pic();
	io_sti();
	
	init_pit(); /*初始化定时器芯片，每秒100次*/
	/*由于 init_pic的时候 禁用了所有IRQ，这里需要手动开放需要的IRQ */
	io_out8(PIC0_IMR, 0xf8); /* PIC0(主PIC) 开放IRQ-0(定时器) IRQ-1(键盘) IRQ-2(链接 从PIC) (1111_1000) */
	io_out8(PIC1_IMR, 0xef); /* PIC1(从PIC) 开放IRQ-12(鼠标) (1110_1111) */
	init_keyboard(&buff_fifo,768);
	enable_mouse(&mdec,&buff_fifo,0);
	init_palette();
	
	
	/*初始化内存管理器*/
	unsigned int mem_total;
	mem_total = memtest(0x00400000,0xbfffffff);
	memman_init(man);
	memman_free(man,0x00001000,0x009e000);
	memman_free(man,0x00400000,mem_total - 0x00400000);
	
	/*设置定时器*/
	timer_init(&buff_fifo,512);
	struct TIMER *timer = timer_alloc();
	struct TIMER *timer2 = timer_alloc();
	settime(timer,130,1);
	settime(timer2,200,2);
	
	/*初始化图层管理器，以及背景图层和鼠标图层*/
	struct STCTL *sheetctl = shtctl_init(man, binfo->vram, binfo->scrnx, binfo->scrny);	
	struct SHEET *sheet_back = sheet_alloc(sheetctl);
	struct SHEET *sheet_mouse = sheet_alloc(sheetctl);
	struct SHEET *sheet_windows = sheet_alloc(sheetctl);
	unsigned char *back_buf, mouse_buf[256],*windows_buf;
	back_buf = memman_alloc_4k(man, binfo->scrnx * binfo->scrny);
	windows_buf = memman_alloc_4k(man, 160 * 80);
	create_windows8(windows_buf,160,80,"test window");
	sheet_setbuf(sheet_back, back_buf, binfo->scrnx, binfo->scrny, -1);
	sheet_setbuf(sheet_mouse, mouse_buf, 16,16, 99);/*设置透明色号为99*/
	sheet_setbuf(sheet_windows,windows_buf,160,80,-1);
	sheet_updown(sheet_back,0);
	sheet_updown(sheet_windows,1);
	sheet_updown(sheet_mouse,2);

	sheet_slide(sheet_windows, 80,70);

	init_screen(back_buf, binfo->scrnx, binfo->scrny);
	/* init mouse start */
	int mx,my;
	mx = binfo->scrnx / 2 - 16;
	my = binfo->scrny / 2 - 16;
	init_mouse_cursor8(mouse_buf,99);/*设置透明色号为99*/
	sheet_slide(sheet_mouse, mx,my);
	/* init mouse end */
	
	
	/*输出内存使用信息*/
	sprintf(s,"Free Memory : %dKB",memman_total(man)/ 1024);
	putfont8_string(back_buf,binfo->scrnx,0,32,COL8_FF0000,s );	
	int memsize = memtest(0x00400000,0xbfffffff) / 1024 / 1024;
	sprintf(s,"Total Memory : %dMB",memsize);
	putfont8_string(back_buf,binfo->scrnx,0,50,COL8_FFFFFF,s );
	sheet_slide(sheet_back, 0,0);
	int data;
	while(1){
		sprintf(s,"Time :%05ds %02d ms",timerctl.count / 100 , timerctl.count % 100);
		putfont8_string_sht(sheet_windows,5,28,COL8_FFFFFF,COL8_C6C6C6 , s,18);
		
		io_cli();
		if( fifo32_status(&buff_fifo) == 0){
			io_stihlt();
			//io_sti();
		}else{
			data = fifo32_get(&buff_fifo);
			
			io_sti();
			if( data == -1000){
				data = data - 512;
				sprintf(s,"timere:%02X",data);
				
				if(data == 1 || data == 0){
					if(data == 1){
					putfont8_string_sht(sheet_back,20, 150,COL8_FFFF00,COL8_008484 , s,10);
					settime(timer,50,0);
					}else{
						putfont8_string_sht(sheet_back,20, 150,COL8_FFFF00,COL8_008484 , "",10);
						settime(timer,50,1);
					};
					
				}else if ( data == 2 || data == 3){ 
					if(data == 2){
						//putfont8_string_sht(sheet_back,110,150,COL8_FFFFFF,COL8_008484 , s,10);
						boxfill8(sheet_back->buf,sheet_back->bxsize, COL8_FFFFFF,110,150,111,166);
						settime(timer2,50,3);
					}else{
						boxfill8(sheet_back->buf,sheet_back->bxsize, COL8_008484,110,150,111,166);
						//putfont8_string_sht(sheet_back,110,150,COL8_FFFFFF,COL8_008484 ,"",10);
						settime(timer2,50,2);
					};
					sheet_refresh(sheet_back, 110,150,111,166);
				}
				
			}else if( data == -1000 ){
				data = data - 768;
				sprintf(s,"%02X",data);
				putfont8_string_sht(sheet_back,0, 16,COL8_FFFFFF,COL8_008484 , s,2);
			}else {
				data = data - 100;
				sprintf(s,"data2:%04X",data);
				putfont8_string_sht(sheet_back,220, 150,COL8_FFFF00,COL8_008484 , s,10);
				if( mouse_decode(&mdec, data) == 1){
					sprintf(s,"Mouse action[lcr %4d %4d]",mdec.x,mdec.y);
					
					if( (mdec.btn & 0x01) != 0){
						s[13] = 'L';
						sheet_updown(sheet_mouse,sheet_mouse->height -1);
					}
					if( (mdec.btn & 0x02) != 0){
						s[15] = 'R';
						sheet_updown(sheet_mouse,sheet_mouse->height + 1);
					}
					if( (mdec.btn & 0x04) != 0){
						s[14] = 'C';
					}
					
					putfont8_string_sht(sheet_back,32,17,COL8_FFFFFF,COL8_008484 , s,50);
					mx += mdec.x;
					my += mdec.y;
					sprintf(s,"Mouse position[%4d:%4d],h:%d",mx,my,sheet_mouse->height);
					putfont8_string_sht(sheet_back,0,0,COL8_FFFFFF,COL8_008484 , s,50);
					sheet_slide(sheet_mouse, mx,my);/*移动图层，并且重新绘制*/
					
				}
			}
		}
	}
}

