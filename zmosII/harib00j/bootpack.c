#define COL8_000000		0
#define COL8_FF0000		1
#define COL8_00FF00		2
#define COL8_FFFF00		3
#define COL8_0000FF		4
#define COL8_FF00FF		5
#define COL8_00FFFF		6
#define COL8_FFFFFF		7
#define COL8_C6C6C6		8
#define COL8_840000		9
#define COL8_008400		10
#define COL8_848400		11
#define COL8_000084		12
#define COL8_840084		13
#define COL8_008484		14
#define COL8_848484		15


/*; BOOT_INFO
CYLS	EQU		0x0ff0			; 启动扇区
LEDS	EQU		0x0ff1			; 键盘上的led信息
VMODE	EQU		0x0ff2			; 颜色模式（8位，16位等等）
SCRNX	EQU		0x0ff4			; 分辨率的x
SCRNY	EQU		0x0ff6			; 分辨率的
VRAM	EQU		0x0ff8			; 显存缓存的起始地址
*/
struct BOOTINFO{
	char cyls,leds,vmode,reverse;
	short scrnx,scrny;
	char *vram;
};

struct SEGMENT_DESCRIPTOR{
	short limit_low,base_low;
	char base_mid,access_right;
	char limit_high,base_high;
};

struct GATE_DESCRIPTOR{
	short offset_low,selector;
	char dw_count,access_right;
	short offset_high;
};



//nas functions
void io_hlt(void);
int io_load_eflags(void);
void io_cli(void);
void io_store_eflags(int eflags);
void io_out8(int port,int value);

//normal functions
void init_palette(void);
void set_palette(int color_num_start, int color_num_end, unsigned char *rgb);

/*fill a box start at (x0,y0) ,end at(x1,y1) .
 *The length of x and y is xsize
 *The start address is vram
 *The position of(x,y):
 *       vram + x + y * 320;
*/
void boxfill8(unsigned char *vram,int xsize, unsigned char color,int x0,int y0,int x1,int y1);
void init_screen(struct BOOTINFO * binfo);
void init_mouse(unsigned char *vram,int vxsize,int x,int y,unsigned char back_ground_color);

void init_mouse_cursor8(char *mouse,char back_ground_color);

void init_gdtidt(void);
void set_segmdesc(struct SEGMENT_DESCRIPTOR* gdt,unsigned int limit,int base,int access);
void set_gatedesc(struct GATE_DESCRIPTOR *idt,int offset, int selector,int access);

void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);


void putfont8(unsigned char *vram,int xsize,int x, int y,unsigned char color,char *font);
void putfont8_ascii(unsigned char *vram,int xsize,int x, int y,unsigned char color,char c_ascii);
void putfont8_string(unsigned char *vram,int xsize,int x, int y,unsigned char color,unsigned char *msg);
void putblock8_8(unsigned char *vram,int vxsize,int block_x_size,int block_y_size,int px0,int py0, char *blockbuf,int bxsize);


void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) 0x0ff0;
	
	init_gdtidt();
	init_palette();
	init_screen(binfo);
	init_mouse(binfo->vram,binfo->scrnx,100,100,COL8_008484);
	
	
	char s[10];
	int y = 4;
	//print variable test
	int test = 1;
	y = 20;
	while(test < 10){
		sprintf(s,"Param = %d",test);
		putfont8_string(binfo->vram,binfo->scrnx,20,y,COL8_FFFF00,s);
		test ++;
		y += 16;
	}
	//print variable test end
	
fin:
	io_hlt();
	goto fin;
}

void init_gdtidt(void)
{
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) 0x00270000;
	struct GATE_DESCRIPTOR *idt = (struct GATE_DESCRIPTOR *) 0x0026f800;
	int i;
	/*init gdt 
	  gdt 最大数量为8192个。
	  段寄存器为16位，低3位有特殊用途，13位 = 8192（0 ~ 8191）
	*/
	for(i = 0 ; i<8192 ; i++){
		set_segmdesc(gdt + i,0,0,0);
	}
	set_segmdesc(gdt + 1,0xffffffff,0x00000000,0x4092);//1# 即 1_000b 0~4G内存
	set_segmdesc(gdt + 2,0x0007ffff,0x00280000,0x4092);//2# 0x00280000 ~ 0x0007ffff:asmhead.nas预留空间
	
	load_gdtr(0xffff, 0x00270000);/*limit = 0xffff (8192(gdt count) * 8(gdt size) ),base addr = 0x0027000*/
	
	/*init idt,idt 最大数量为256,idt（中断表） 暂时全部留空*/
	for(i = 0 ; i < 256; i++){
		set_gatedesc(idt + i,0,0,0);
	}
	
	load_idtr(0x7ff, 0x0026f800);/*limit = 0x7ff (256(idt count) * 8(gdt size) ),base addr = 0x0026f800*/
	
}

void set_segmdesc(struct SEGMENT_DESCRIPTOR* gdt,unsigned int limit,int base,int access)
{
	if (limit > 0xfffff) {
		access |= 0x8000; /* G_bit = 1 */
		limit /= 0x1000;
	}
	gdt->limit_low    = limit & 0xffff;
	gdt->base_low     = base & 0xffff;
	gdt->base_mid     = (base >> 16) & 0xff;
	gdt->access_right = access & 0xff;
	gdt->limit_high   = ((limit >> 16) & 0x0f) | ((access >> 8) & 0xf0);
	gdt->base_high    = (base >> 24) & 0xff;
	return;
}

void set_gatedesc(struct GATE_DESCRIPTOR *idt,int offset, int selector,int access)
{
	idt->offset_low   = offset & 0xffff;
	idt->selector     = selector;
	idt->dw_count     = (access >> 8) & 0xff;
	idt->access_right = access & 0xff;
	idt->offset_high  = (offset >> 16) & 0xffff;
	return;
}

void init_mouse(unsigned char *vram,int vxsize,int x,int y,unsigned char back_ground_color)
{
	char cursor[16][16];
	init_mouse_cursor8(cursor,back_ground_color);
	putblock8_8(vram,vxsize,16,16,x,y,cursor,16);
}


void putblock8_8(unsigned char *vram,int vxsize,int block_x_size,int block_y_size,int px0,int py0, char *blockbuf,int bxsize)
{
	int x,y;
	for(y = 0; y < block_y_size; y++){
		for(x= 0 ; x< block_x_size; x++){
			vram[(py0 + y) * vxsize + px0 + x] = blockbuf[y * bxsize + x];
		}
	}
}

void init_mouse_cursor8(char *mouse,char back_ground_color)
{
	// * = CLO8_000000
	// O = CLO8_FFFFFF
	// . = back_groudn_color
	//cursor size = 16 * 16
	static char cursor[16][16] = {
		"**************..",
		"*ooooooooooo*...",
		"*oooooooooo*....",
		"*ooooooooo*.....",
		"*oooooooo*......",
		"*ooooooo*.......",
		"*ooooooo*.......",
		"*oooooooo*......",
		"*oooo**ooo*.....",
		"*ooo*..*ooo*....",
		"*oo*....*ooo*...",
		"*o*......*ooo*..",
		"**........*ooo..",
		"*..........*ooo*",
		"............*oo*",
		".............***"
		
	};
	int x,y;
	for(y = 0 ; y < 16; y++){
		for(x = 0 ; x < 16 ; x++){
			if(cursor[y][x] == '*'){
				mouse[y * 16 + x] = COL8_000000;
			}
			if(cursor[y][x] == 'o'){
				mouse[y * 16 + x] = COL8_FFFFFF;
			}
			if(cursor[y][x] == '.'){
				mouse[y * 16 + x] = back_ground_color;
			}
		}
	}	
	
}


void putfont8_string(unsigned char *vram,int xsize,int x, int y,unsigned char color,unsigned char *msg)
{
	int x_temp = x;
	while (* msg != '\0'){
		putfont8_ascii(vram,xsize,x_temp, y,color,*msg);
		x_temp += 8;
		msg ++;
	}
}

void putfont8_ascii(unsigned char *vram,int xsize,int x, int y,unsigned char color,char c_ascii)
{
	extern char hankaku[4096];
	char *font = hankaku + c_ascii * 16;
	putfont8(vram,xsize,x, y,color,font);
}
   
void putfont8(unsigned char *vram,int xsize,int x, int y,unsigned char color,char *font)
{
	int i,j;
	char *vram_temp, font_temp;
	for(i = 0; i < 16 ; i++){
		vram_temp = &(vram[(y + i) * xsize + x]);
		font_temp = font[i];
		j = 0x80;
		while(j > 0){
			if((font_temp & j) != 0){
				*vram_temp = color;
			}	 
			j = j >> 1;
			vram_temp++;
		}		
	}

}	
   
   
void init_palette(void)
{
	static unsigned char table_rgb[16 * 3] = {
		0x00, 0x00, 0x00,	/*  0:黒 */
		0xff, 0x00, 0x00,	/*  1:亮红 */
		0x00, 0xff, 0x00,	/*  2:亮緑 */
		0xff, 0xff, 0x00,	/*  3:亮黄色 */
		0x00, 0x00, 0xff,	/*  4:亮青 */
		0xff, 0x00, 0xff,	/*  5:亮紫 */
		0x00, 0xff, 0xff,	/*  6:浅蓝色  */
		0xff, 0xff, 0xff,	/*  7:白 */
		0xc6, 0xc6, 0xc6,	/*  8:浅灰色 */
		0x84, 0x00, 0x00,	/*  9:深红 */
		0x00, 0x84, 0x00,	/* 10:深緑 */
		0x84, 0x84, 0x00,	/* 11:深黄色 */
		0x00, 0x00, 0x84,	/* 12:深青 */
		0x84, 0x00, 0x84,	/* 13:深紫 */
		0x00, 0x84, 0x84,	/* 14:深蓝色 */
		0x84, 0x84, 0x84	/* 15:深灰色 */
	};
	set_palette(0, 15, table_rgb);
	return;

	/* static char = DB:0x00,0x00,0x00..... */
}

void init_screen(struct BOOTINFO * binfo)
{
	int xsize = binfo->scrnx;
	int ysize = binfo->scrny;
	char *vram = binfo->vram;
   
	boxfill8(vram, xsize, COL8_008484,  0,         0,          xsize -  1, ysize - 29);
	boxfill8(vram, xsize, COL8_C6C6C6,  0,         ysize - 28, xsize -  1, ysize - 28);
	boxfill8(vram, xsize, COL8_FFFFFF,  0,         ysize - 27, xsize -  1, ysize - 27);
	boxfill8(vram, xsize, COL8_C6C6C6,  0,         ysize - 26, xsize -  1, ysize -  1);
	
	boxfill8(vram, xsize, COL8_FFFFFF,  3,         ysize - 24, 59,         ysize - 24);
	boxfill8(vram, xsize, COL8_FFFFFF,  2,         ysize - 24,  2,         ysize -  4);
	boxfill8(vram, xsize, COL8_848484,  3,         ysize -  4, 59,         ysize -  4);
	boxfill8(vram, xsize, COL8_848484, 59,         ysize - 23, 59,         ysize -  5);
	boxfill8(vram, xsize, COL8_000000,  2,         ysize -  3, 59,         ysize -  3);
	boxfill8(vram, xsize, COL8_000000, 60,         ysize - 24, 60,         ysize -  3);
	
	boxfill8(vram, xsize, COL8_848484, xsize - 47, ysize - 24, xsize -  4, ysize - 24);
	boxfill8(vram, xsize, COL8_848484, xsize - 47, ysize - 23, xsize - 47, ysize -  4);
	boxfill8(vram, xsize, COL8_FFFFFF, xsize - 47, ysize -  3, xsize -  4, ysize -  3);
	boxfill8(vram, xsize, COL8_FFFFFF, xsize -  3, ysize - 24, xsize -  3, ysize -  3);
	
}


void set_palette(int color_num_start, int color_num_end, unsigned char *rgb)
{
	int i,eflags;
	eflags = io_load_eflags();
	io_cli();
	io_out8(0x03c8,color_num_start);			//调色板号码，只需要开始写入一次，只有就可以连续写入其他调色板的颜色配置。
	for(i = color_num_start;i <= color_num_end; i++){
		io_out8(0x03c9,rgb[0] /4);		//  除以4 提高明亮度
		io_out8(0x03c9,rgb[1] /4);
		io_out8(0x03c9,rgb[2] /4);
		rgb += 3;
	}
	io_store_eflags(eflags);
	return;
}


//vram + x + y * 320;
void boxfill8(unsigned char *vram,int xsize, unsigned char color,int x0,int y0,int x1,int y1)
{
	int x, y;
	for (y = y0; y <= y1; y++) {
		for (x = x0; x <= x1; x++)
			vram[y * xsize + x] = color;
	}
	return;
}

