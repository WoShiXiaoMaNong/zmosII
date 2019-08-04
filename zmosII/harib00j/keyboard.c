#include "bootpack.h"

struct FIFO8 keyfifo;

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


//Keyboard
void inthandler21(int *esp)
{
	unsigned char data;
	io_out8(PIC0_OCW2,0x60 + 1); //通知 主PIC IRQ-1 中断处理完毕。
	data = io_in8(PORT_KEYDAT);
	fifo8_put(&keyfifo,data);
	return;
}