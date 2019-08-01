#include <stdio.h>
#include "bootpack.h"

extern struct FIFO8 keyfifo;
extern struct FIFO8 mousefifo;


struct MOUSE_DESC{
	unsigned char buf[3],phase;
	int x,y,btn;
};

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

void enable_mouse(struct MOUSE_DESC *mdec)
{
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD,KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT,MOUSECMD_ENABLE);
	mdec->phase = 0;
	return;
}


int mouse_decode(struct MOUSE_DESC *mdec, unsigned char data)
{
		if(mdec->phase == 0){
			if(data == 0xfa){
				mdec->phase = 1;
				return 0;
			}
		}else if(mdec->phase == 1){
			if( (data & 0xc8) == 0x08){
				mdec->buf[0] = data;
				mdec->phase = 2;
			}
			return 0;
	    }else if(mdec->phase == 2){
	    	mdec->buf[1] = data;
	    	mdec->phase = 3;
			return 0;
	    }else if(mdec->phase == 3){
	    	mdec->buf[2] = data;
	    	mdec->phase = 1;
			/*开始解析鼠标的信息*/
			mdec->btn = mdec->buf[0] & 0x07; //0x07 = 0000_0111B,鼠标状态被存放在buf[0]的低三位。
			mdec->x = mdec->buf[1];
			mdec->y = mdec->buf[2];
			
			if( (mdec->buf[0] & 0x10) != 0){
				mdec->x |= 0xffffff00;
			}
			if( (mdec->buf[0] & 0x20) != 0){
				mdec->y |= 0xffffff00;
			}
			
			mdec->y = -mdec->y;
			return 1;
		}
	return -1;
}


void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADDR_BOOTINFO;
	struct MOUSE_DESC mdec;
	char s[10];
	unsigned char keybuff[36],mousebuff[36];
	char cursor[16][16];
	
	init_gdtidt();
	init_pic();
	
	init_keyboard();
	enable_mouse(&mdec);
	io_sti();
	init_palette();
	init_screen(binfo);
	
	
	/* init mouse start */
	int mx,my;
	mx = binfo->scrnx / 2 - 16;
	my = binfo->scrny / 2 - 16;
	
	init_mouse_cursor8(cursor,COL8_008484);
	putblock8_8(binfo->vram,binfo->scrnx,16,16,mx,my,cursor,16);
	/* init mouse end */
	
	
	fifo8_init(&keyfifo,keybuff,36);
	fifo8_init(&mousefifo,mousebuff,36);
	
	
	
	
	io_out8(PIC0_IMR, 0xf9); /* PIC0(主PIC) 开放IRQ-1 IRQ-2 (11111001) */
	io_out8(PIC1_IMR, 0xef); /* PIC1(从PIC) 开放IRQ-12 (11101111) */
	
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
					
					
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484,  mx,         my,          mx + 15, my + 15);
					mx += mdec.x;
					my += mdec.y;
					putblock8_8(binfo->vram,binfo->scrnx,16,16,mx,my,cursor,16);
					
					
				}
			}
		}
	}
}

