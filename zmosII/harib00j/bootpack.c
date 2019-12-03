#include <stdio.h>
#include "bootpack.h"


#define BUF_LENGTH 36
extern struct TIMERCTL timerctl;
extern struct TASK_CTL *taskctl;
extern char keytable[];





void task_console(struct SHEET* sheet,struct FIFO32 *buff_fifo)
{
	
	int cursor_x,cursor_y,cursor_h = 16;
	int cursor_color = COL8_000000;
	int init_cursor_x = 10;
	cursor_x = 10;
	cursor_y = 32;
	
	struct TIMER *timer_print = timer_alloc();
	timer_init(timer_print,buff_fifo,2);
	
	settime(timer_print,2);
	int data;
	int count = 0;
	char s[20];

	while(1)
	{
		io_cli();
		count ++;
		if(count <=0 ){
			count = 0;
		}
		if( fifo32_status(buff_fifo) == 0)  {
			io_sti();
		}else{
			
			data = fifo32_get(buff_fifo);
			io_sti();
			
			
			if(data == 2){
				boxfill8(sheet->buf,sheet->bxsize, cursor_color,cursor_x,cursor_y,cursor_x,cursor_y + cursor_h);
				settime(timer_print,50);
				timer_init(timer_print,buff_fifo,3);
				
			}else if(data == 3){
				boxfill8(sheet->buf,sheet->bxsize, COL8_FFFFFF,cursor_x,cursor_y,cursor_x,cursor_y + cursor_h);
				settime(timer_print,50);
				timer_init(timer_print,buff_fifo,2);
			}else if( data >= 256 && data <512 ){  /* 键盘输入*/
				//字符输入测试 >>>>开始<<<<
				data = data - 256;
				if(data < 0x54){
					if(data == 0x1c){  // 0x1c : Enter
						putfont8_string_sht(sheet,cursor_x, cursor_y,COL8_FFFFFF ,COL8_000000, " ",1);
						cursor_y += 16;
						cursor_x = init_cursor_x;
						continue;
					}else if(keytable[data] != 0){
						s[0] = keytable[data];
						s[1] = 0;
						putfont8_string_sht(sheet,cursor_x, cursor_y,COL8_FFFFFF ,COL8_000000, s,1);
						cursor_x += 8;
					}else if(data == 0x0e){
						cursor_x -= 8;
						putfont8_string_sht(sheet,cursor_x, cursor_y,COL8_FFFFFF ,COL8_000000, " ",1);
					}
				}
				boxfill8(sheet->buf,sheet->bxsize, cursor_color,cursor_x,cursor_y,cursor_x,cursor_y + cursor_h);
				//字符输入测试 >>>>结束<<<<
				
			};
			sheet_refresh(sheet, cursor_x,cursor_y,cursor_x,cursor_y + cursor_h);
			
			
		}
	}
}


void task_b_main(struct SHEET* sheet)
{
	
	struct FIFO32 buff_fifo;
	int buff[BUF_LENGTH];

	fifo32_init(&buff_fifo,buff,BUF_LENGTH,0);
	
	
	struct TIMER *timer_print = timer_alloc();
	timer_init(timer_print,&buff_fifo,1);
	
	
	
	settime(timer_print,1);
	int data;
	int count = 0;
	

	char s[17];

	while(1)
	{
		io_cli();
		count ++;
		if(count <=0 ){
			count = 0;
		}
		if( fifo32_status(&buff_fifo) == 0)  {
			io_sti();
		}else{
			
			data = fifo32_get(&buff_fifo);
			io_sti();
			
			if(data == 1){
				if(count % 5 == 0 ){
					sprintf(s,"CountA :%11d",count/100000);
				}
				putfont8_string_sht(sheet,5, 28,COL8_000000,COL8_C6C6C6 , s,19);
				settime(timer_print,1);
			}
		}
	}
}


void HariMain(void)
{
	
	int key_to = 0;
	
	int cursor_x = 12 ,cursor_y = 48,cursor_h = 16;
	int cursor_color = COL8_000000;
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADDR_BOOTINFO;
	struct MEMMAN *man = (struct MEMMAN *) MEMMAN_ADDR;
	struct MOUSE_DESC mdec;
	char s[256];
	
	init_gdtidt();
	init_pic();
	io_sti();
	
	/*初始化内存管理器*/
	unsigned int mem_total;
	mem_total = memtest(0x00400000,0xbfffffff);
	memman_init(man);
	memman_free(man,0x00001000,0x009e000);
	memman_free(man,0x00400000,mem_total - 0x00400000);
	
	
	init_pit(); /*初始化定时器芯片，每秒100次*/
	/*由于 init_pic的时候 禁用了所有IRQ，这里需要手动开放需要的IRQ */
	io_out8(PIC0_IMR, 0xf8); /* PIC0(主PIC) 开放IRQ-0(定时器) IRQ-1(键盘) IRQ-2(链接 从PIC) (1111_1000) */
	io_out8(PIC1_IMR, 0xef); /* PIC1(从PIC) 开放IRQ-12(鼠标) (1110_1111) */
	
	
	
	/* 初始化 task 管理器 start*/
	struct FIFO32 buff_mai;
	struct FIFO32 *buff_main = &buff_mai;
	int buff[BUF_LENGTH];
	fifo32_init(buff_main,buff,BUF_LENGTH,0);
	
	struct TASK *task_main = mt_init(man,buff_main); 
	buff_main->task = task_main;
	/* 初始化 task 管理器 end*/
	
	
	
	init_keyboard(buff_main,256);
	enable_mouse(&mdec,buff_main,512);
	init_palette();
	
		
	/*设置定时器*/
	//void timer_init(truct TIMER *timer,struct FIFO32 *fifo,int data)
	struct TIMER *timer = timer_alloc();
	struct TIMER *timer2 = timer_alloc();
	

	
	timer_init(timer,buff_main,1);
	timer_init(timer2,buff_main,2);

	settime(timer,130);
	settime(timer2,200);

	
	/*初始化图层管理器，以及背景图层和鼠标图层*/
	struct STCTL *sheetctl = shtctl_init(man, binfo->vram, binfo->scrnx, binfo->scrny);	
	struct SHEET *sheet_back = sheet_alloc(sheetctl);
	struct SHEET *sheet_mouse = sheet_alloc(sheetctl);
	struct SHEET *sheet_windows = sheet_alloc(sheetctl);
	unsigned char *back_buf, mouse_buf[256],*windows_buf;
	back_buf = (unsigned char*)memman_alloc_4k(man, binfo->scrnx * binfo->scrny);
	
	
	
	windows_buf = (unsigned char*)memman_alloc_4k(man, 160 * 80);
	create_windows8(windows_buf,160,80,"test window",1);

	
	sheet_setbuf(sheet_back, back_buf, binfo->scrnx, binfo->scrny, -1);
	sheet_setbuf(sheet_mouse, mouse_buf, 16,16, 99);/*设置透明色号为99*/
	sheet_setbuf(sheet_windows,windows_buf,160,80,-1);
	
	/*Text input in Test window*/
	make_textbox8(sheet_windows, 8,48,144,16,COL8_FFFFFF);

	

	init_screen(back_buf, binfo->scrnx, binfo->scrny);
	/* init mouse start */
	int mx,my;
	mx = binfo->scrnx / 2 - 16;
	my = binfo->scrny / 2 - 16;
	init_mouse_cursor8(mouse_buf,99);/*设置透明色号为99*/
	
	/* init mouse end */
	
	
	/*输出内存使用信息*/
	sprintf(s,"Free Memory : %dKB",memman_total(man)/ 1024);
	putfont8_string_sht(sheet_back,0, 32,COL8_FF0000,COL8_008484 , s,20);
	putfont8_string_sht(sheet_back,0, 16,COL8_FF0000,COL8_008484 , "testset",20);
	//putfont8_string(back_buf,binfo->scrnx,0,32,COL8_FF0000,s );	
	int memsize = memtest(0x00400000,0xbfffffff) / 1024 / 1024;
	sprintf(s,"Total Memory : %dMB",memsize);
	
	//putfont8_string(back_buf,binfo->scrnx,0,50,COL8_FFFFFF,s );
	putfont8_string_sht(sheet_back,0, 50,COL8_FFFFFF,COL8_008484 , s,20);
	int data;
	
	
	
		
	/* 命令行窗口 开始 */
	struct SHEET *sheet_cons;
	sheet_cons = sheet_alloc(sheetctl);
	unsigned char *cons_buf = (unsigned char*)memman_alloc_4k(man,180 * 320);
	create_windows8(cons_buf,320,180,"console",0);
	sheet_setbuf(sheet_cons,cons_buf,320,180,99);
	
	make_textbox8(sheet_cons, 8,30,300,140,COL8_000000);
	
	
	/* 命令行窗口 结束 */
	
	
	
	/* 多任务测试 开始 */
	
	
	/*创建子窗口用于多任务测试 */
	
	struct FIFO32 *buff_fifo_cons;
	int buff_cons[BUF_LENGTH];
	fifo32_init(buff_fifo_cons,buff_cons,BUF_LENGTH,0);
	
	
	struct SHEET *sheet_windowsb;
	sheet_windowsb = sheet_alloc(sheetctl);
	unsigned char *windowsb_buf;
	windowsb_buf = (unsigned char*)memman_alloc_4k(man, 160 * 80);
	create_windows8(windowsb_buf,160,80,"test window B",0);
	sheet_setbuf(sheet_windowsb,windowsb_buf,160,80,-1);
	
	
	struct TASK *task_cons = task_alloc(buff_fifo_cons);
	int task_b_esp = memman_alloc_4k(man, 64 * 1024) + 64 * 1024; 
	
	*((int*)(task_b_esp + 4)) = (int)sheet_cons;
	*((int*)(task_b_esp + 8)) = (int)buff_fifo_cons;
	 
	task_cons->tss.eip = (int) &task_console;
	task_cons->tss.eflags = 0x00000202; /* IF = 1; */
	task_cons->tss.esp = task_b_esp;
	task_cons->tss.es = 1 * 8;
	task_cons->tss.cs = 2 * 8;
	task_cons->tss.ss = 1 * 8;
	task_cons->tss.ds = 1 * 8;
	task_cons->tss.fs = 1 * 8;
	task_cons->tss.gs = 1 * 8;
	task_cons->priority = 1;
	task_cons->fifo32 = buff_fifo_cons;
	task_run(task_cons,1,0);
	
	
	struct TASK *task2 = task_alloc(0);
	int task_c_esp;
	
	task_c_esp = memman_alloc_4k(man, 64 * 1024) + 64 * 1024;
	task2->tss.eip = (int) &task_b_main;
	task2->tss.eflags = 0x00000202; /* IF = 1; */
	task2->tss.esp = task_c_esp;
	task2->tss.es = 1 * 8;
	task2->tss.cs = 2 * 8;
	task2->tss.ss = 1 * 8;
	task2->tss.ds = 1 * 8;
	task2->tss.fs = 1 * 8;
	task2->tss.gs = 1 * 8;
	task2->priority = 1;
	
	*((int *)(task2->tss.esp + 4)) = (int) sheet_windowsb;
	task_run(task2,1,0);
	
	

	
	/* 多任务测试 结束 */
	

	
	sheet_updown(sheet_back,0);
	sheet_updown(sheet_windows,1);
	sheet_updown(sheet_cons,2);
	sheet_updown(sheet_windowsb,3);
	sheet_updown(sheet_mouse,4);
	
	sheet_slide(sheet_back, 0,0);
	sheet_slide(sheet_windows, 80,70);
	sheet_slide(sheet_windowsb, 80 ,70 + 80 + 10);
	sheet_slide(sheet_cons, 500,60);
	sheet_slide(sheet_mouse, mx,my);
	
	while(1){
		sprintf(s,"Time :%05ds %02d ms",timerctl.count / 100 , timerctl.count % 100);
		putfont8_string_sht(sheet_windows,5,28,COL8_000000,COL8_C6C6C6 , s,18);
		
		io_cli();
		if( fifo32_status(buff_main) == 0){
			task_sleep(task_main);
			io_sti();
		}else{
			data = fifo32_get(buff_main);
			io_sti();
			sprintf(s,"d:%04X",data);
			putfont8_string_sht(sheet_back,220, 150,COL8_FFFF00,COL8_008484 , s,10);
			
			if( data < 20){  /*各种计时器*/
				sprintf(s,"timere:%02X",data);
				
				if(data == 1 || data == 0){
					if(data == 1){
					putfont8_string_sht(sheet_back,20, 150,COL8_FFFF00,COL8_008484 , s,9);
					settime(timer,50);
					timer_init(timer,buff_main,0);
					}else{
						putfont8_string_sht(sheet_back,20, 150,COL8_FFFF00,COL8_008484 , "",9);
						settime(timer,50);
						timer_init(timer,buff_main,1);
						
					};
					
				}else if ( data == 2 || data == 3){  //光标控制
					if(data == 2){
						boxfill8(sheet_windows->buf,sheet_windows->bxsize, cursor_color,cursor_x,cursor_y,cursor_x,cursor_y + cursor_h);
						settime(timer2,50);
						timer_init(timer2,buff_main,3);
						
					}else{
						boxfill8(sheet_windows->buf,sheet_windows->bxsize, COL8_FFFFFF,cursor_x,cursor_y,cursor_x,cursor_y + cursor_h);
						settime(timer2,50);
						timer_init(timer2,buff_main,2);
					};
					sheet_refresh(sheet_windows, cursor_x,cursor_y,cursor_x,cursor_y + cursor_h);
				}
				
			}else if( data >= 256 && data <512 ){  /* 键盘输入*/
			
				data = data - 256;
				if(data == 0x0f){ // Tab 键，切换窗口。目前是hard code。
					
					if(key_to == 0){
						key_to = 1;
						create_title_bar(sheet_cons->buf,sheet_cons->bxsize,"console", 1);
						create_title_bar(sheet_windows->buf,sheet_windows->bxsize,"test window", 0);
					}else{
						key_to = 0;
						create_title_bar(sheet_cons->buf,sheet_cons->bxsize,"console", 0);
						create_title_bar(sheet_windows->buf,sheet_windows->bxsize, "test window", 1);
					}
					
					sheet_refresh(sheet_cons, 0,0,sheet_cons->bxsize, sheet_cons->bysize);
					sheet_refresh(sheet_windows, 0,0,sheet_windows->bxsize, sheet_windows->bysize);
				}
				
				
				sprintf(s,"%02X",data);
				putfont8_string_sht(sheet_back,0, 16,COL8_FFFFFF,COL8_008484 , s,2);
				
				
				//字符输入测试 >>>>开始<<<<
				if(key_to == 1){
					fifo32_put(task_cons->fifo32,data + 256);//(struct FIFO32 *fifo32,int data)
				}else{
					if(data < 0x54){
						if(keytable[data] != 0){
							s[0] = keytable[data];
							s[1] = 0;
							putfont8_string_sht(sheet_windows,cursor_x, cursor_y,COL8_000000,COL8_FFFFFF , s,1);
							cursor_x += 8;
						}
						if(data == 0x0e){
							cursor_x -= 8;
							putfont8_string_sht(sheet_windows,cursor_x, cursor_y,COL8_000000,COL8_FFFFFF , " ",1);
						}
					}
					boxfill8(sheet_windows->buf,sheet_windows->bxsize, cursor_color,cursor_x,cursor_y,cursor_x,cursor_y + cursor_h);
					sheet_refresh(sheet_windows, cursor_x,cursor_y,cursor_x,cursor_y + cursor_h);
				
				}
				//字符输入测试 >>>>结束<<<<
				
			}else if(data >= 512 && data <768 ){/* 鼠标输入*/
				data = data - 512;
				
				if( mouse_decode(&mdec, data) == 1){
					sprintf(s,"Mouse action[lcr %4d %4d]",mdec.x,mdec.y);
					
					if( (mdec.btn & 0x01) != 0){
						s[13] = 'L';
						sheet_slide(sheet_windows, mx,my);/*移动图层，并且重新绘制*/
						//sheet_updown(sheet_mouse,sheet_mouse->height -1);
					}
					if( (mdec.btn & 0x02) != 0){
						s[15] = 'R';
						//sheet_updown(sheet_mouse,sheet_mouse->height + 1);
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


