#include "bootpack.h"



struct FIFO8 keyfifo;
struct FIFO8 mousefifo;
/*
 *初始化PIC，固定配置
*/
void init_pic(void)
{
	io_out8(PIC0_IMR,  0xff  ); // 禁止主PIC上所有中断，(11111111)
	io_out8(PIC1_IMR,  0xff  ); // 禁止 从PIC上所有中断  (11111111)

	io_out8(PIC0_ICW1, 0x11  );
	io_out8(PIC0_ICW2, 0x20  );  //唯一可以自定义的设置，这是起始中断号。
	io_out8(PIC0_ICW3, 1 << 2);
	io_out8(PIC0_ICW4, 0x01  );

	io_out8(PIC1_ICW1, 0x11  );
	io_out8(PIC1_ICW2, 0x28  );
	io_out8(PIC1_ICW3, 2     );
	io_out8(PIC1_ICW4, 0x01  );

	io_out8(PIC0_IMR,  0xfb  );
	io_out8(PIC1_IMR,  0xff  );


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

void inthandler2c(int *esp)
{
	unsigned char data;
	io_out8(PIC1_OCW2,0x60 + 4); //通知 从PIC IRQ-12 中断处理完毕。
	io_out8(PIC0_OCW2,0x60 + 2); //通知 主PIC IRQ-2 中断处理完毕。
	
	data = io_in8(PORT_KEYDAT);
	fifo8_put(&mousefifo,data);
	
	return;
}