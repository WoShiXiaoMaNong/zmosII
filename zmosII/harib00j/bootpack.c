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


void putfont8(unsigned char *vram,int xsize,int x, int y,unsigned char color,char *font);
void putfont8_ascii(unsigned char *vram,int xsize,int x, int y,unsigned char color,char c_ascii);
void putfont8_string(unsigned char *vram,int xsize,int x, int y,unsigned char color,unsigned char *msg);



void HariMain(void)
{
	init_palette();
	struct BOOTINFO *binfo = (struct BOOTINFO *) 0x0ff0;
	init_screen(binfo);
	
	putfont8_string(binfo->vram,binfo->scrnx,20,20,COL8_848400,"Hello world!!!!!");
fin:
	io_hlt();
	goto fin;
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

