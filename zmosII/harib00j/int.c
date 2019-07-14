#include "bootpack.h"


/*
 *初始化PIC，固定配置
*/
void init_pic(void)
{
	io_out8(PIC0_IMR,  0xff  );
	io_out8(PIC1_IMR,  0xff  );

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
void inthandler21(int *esp)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADDR_BOOTINFO;
	boxfill8(binfo->vram, binfo->scrnx, COL8_000000, 10, 10, 32 * 8 - 1, 15);
	putfont8_string(binfo->vram,binfo->scrnx,10,10,COL8_FFFF00,"INT 21 (IRQ-1) : PS/2 keyboard");
	for (;;) {
		io_hlt();
	}
}