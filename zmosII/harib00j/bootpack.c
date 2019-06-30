

void io_hlt(void);
void init_palette(void);
void set_palette(int color_num_start, int color_num_end, unsigned char *rgb);
int io_load_eflags(void);
void io_cli(void);
void io_store_eflags(int eflags);


void HariMain(void)
{
	init_palette();
	int i;
	char* p = (char *)0xa0000;
	for (i = 0;i<= 0xffff; i++){
		*(p + i) = i & 0x0f; // mov byte [0xa0000 + i], i & 0x0f
	}
fin:
	io_hlt();
	goto fin;

}
   
void init_palette(void)
{
	static unsigned char table_rgb[16 * 3] = {
		0x00, 0x00, 0x00,	/*  0:黒 */
		0xff, 0x00, 0x00,	/*  1:明るい赤 */
		0x00, 0xff, 0x00,	/*  2:明るい緑 */
		0xff, 0xff, 0x00,	/*  3:明るい黄色 */
		0x00, 0x00, 0xff,	/*  4:明るい青 */
		0xff, 0x00, 0xff,	/*  5:明るい紫 */
		0x00, 0xff, 0xff,	/*  6:明るい水色 */
		0xff, 0xff, 0xff,	/*  7:白 */
		0xc6, 0xc6, 0xc6,	/*  8:明るい灰色 */
		0x84, 0x00, 0x00,	/*  9:暗い赤 */
		0x00, 0x84, 0x00,	/* 10:暗い緑 */
		0x84, 0x84, 0x00,	/* 11:暗い黄色 */
		0x00, 0x00, 0x84,	/* 12:暗い青 */
		0x84, 0x00, 0x84,	/* 13:暗い紫 */
		0x00, 0x84, 0x84,	/* 14:暗い水色 */
		0x84, 0x84, 0x84	/* 15:暗い灰色 */
	};
	set_palette(0, 15, table_rgb);
	return;

	/* static char = DB:0x00,0x00,0x00..... */
}



void set_palette(int color_num_start, int color_num_end, unsigned char *rgb)
{
	int color_mode_port = 0x03c9;
	int i,eflags;
	eflags = io_load_eflags();
	io_cli();
	io_out8(color_mode_port,color_num_start);			//调色板号码，只需要开始写入一次，只有就可以连续写入其他调色板的颜色配置。
	for(i = color_num_start;i <= color_num_end; i++){
		io_out8(color_mode_port,rgb[0] / 4);
		io_out8(color_mode_port,rgb[1] / 4);
		io_out8(color_mode_port,rgb[2] / 4);
		rgb += 3;
	}
	io_store_eflag(eflags);
}





