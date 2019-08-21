#include "bootpack.h"


struct FIFO32 *mousefifo;
int mousedata0;



void inthandler2c(int *esp)
{
	unsigned char data;
	io_out8(PIC1_OCW2,0x60 + 4); //通知 从PIC IRQ-12 中断处理完毕。
	io_out8(PIC0_OCW2,0x60 + 2); //通知 主PIC IRQ-2 中断处理完毕。
	
	data = io_in8(PORT_KEYDAT);
	fifo32_put(mousefifo,data + 100);
	
	return;
}

void enable_mouse(struct MOUSE_DESC *mdec,struct FIFO32 *fifo, int data0)
{
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD,KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT,MOUSECMD_ENABLE);
	mdec->phase = 0;
	mousefifo = fifo;
	mousedata0 = data0;
	return;
}


int mouse_decode(struct MOUSE_DESC *mdec, int data)
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

