#include "bootpack.h"

struct FIFO32 *keyfifo;
int keydata0;
char keytable[0x54] = {
					0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0,   0,
					'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', 0,   0,   'A', 'S',
					'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0,   0,   ']', 'Z', 'X', 'C', 'V',
					'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
					0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
					'2', '3', '0', '.'};



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


void init_keyboard(struct FIFO32 *fifo, int data0)
{
	keydata0 = data0;
	keyfifo = fifo;
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD,KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT,KBC_MODE);
	
	return;
}


//Keyboard
void inthandler21(int *esp)
{
	int data;
	io_out8(PIC0_OCW2,0x60 + 1); //通知 主PIC IRQ-1 中断处理完毕。
	data = io_in8(PORT_KEYDAT);
	
	data = data & 0xff; //只截取低8位
	
	fifo32_put(keyfifo,data + keydata0);
	
	return;
}