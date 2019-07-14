#include "bootpack.h"


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
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADDR_BOOTINFO;
	unsigned char data, s[4];
	
	io_out8(PIC0_OCW2,0x60 + 1); //通知PIC IRQ-1 中断处理完毕。
	data = io_in8(PORT_KEYDAT);
	boxfill8(binfo->vram, binfo->scrnx, COL8_000000, 0, 16, 15,31);
	
	sprintf(s,"%02X",data);
	putfont8_string(binfo->vram,binfo->scrnx,0,16,COL8_FFFF00,s );
	
}

void inthandler2c(int *esp)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADDR_BOOTINFO;
	boxfill8(binfo->vram, binfo->scrnx, COL8_000000, 10, 10, 32 * 8 - 1, 15);
	putfont8_string(binfo->vram,binfo->scrnx,10,10,COL8_FFFF00,"INT 2c (IRQ-12) : Mouse" );
}