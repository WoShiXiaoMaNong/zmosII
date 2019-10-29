#include "bootpack.h"


void create_windows8(unsigned char *buf,int xsize,int ysize,char *title)
{
	static char closebtn[14][16] = {
		"ooooooooooooooo@",
		"oQQQQQQQQQQQQQ$@",
		"oQQQQQQQQQQQQQ$@",
		"oQQQ@@QQQQ@@QQ$@",
		"oQQQQ@@QQ@@QQQ$@",
		"oQQQQQ@@@@QQQQ$@",
		"oQQQQQQ@@QQQQQ$@",
		"oQQQQQ@@@@QQQQ$@",
		"oQQQQ@@QQ@@QQQ$@",
		"oQQQ@@QQQQ@@QQ$@",
		"oQQQQQQQQQQQQQ$@",
		"oQQQQQQQQQQQQQ$@",
		"o$$$$$$$$$$$$$$@",
		"@@@@@@@@@@@@@@@@"
	};
	int x,y;
	char color;
	boxfill8(buf, xsize, COL8_C6C6C6, 0,         0,         xsize - 1, 0        );
	boxfill8(buf, xsize, COL8_FFFFFF, 1,         1,         xsize - 2, 1        );
	boxfill8(buf, xsize, COL8_C6C6C6, 0,         0,         0,         ysize - 1);
	boxfill8(buf, xsize, COL8_FFFFFF, 1,         1,         1,         ysize - 2);
	boxfill8(buf, xsize, COL8_848484, xsize - 2, 1,         xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, xsize - 1, 0,         xsize - 1, ysize - 1);
	boxfill8(buf, xsize, COL8_C6C6C6, 2,         2,         xsize - 3, ysize - 3);
	boxfill8(buf, xsize, COL8_000084, 3,         3,         xsize - 4, 20       );
	boxfill8(buf, xsize, COL8_848484, 1,         ysize - 2, xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, 0,         ysize - 1, xsize - 1, ysize - 1);
	putfont8_string(buf, xsize, 24, 4, COL8_C6C6C6, title);
	
	for(y = 0 ; y < 14; y++){
		for(x = 0 ; x < 16 ; x++){
			color = closebtn[y][x];
			if(color == 'Q'){
				color = COL8_C6C6C6;
			}else if(color == '@'){
				color = COL8_000000;
			}else if(color == '$'){
				color = COL8_848484;
			}else{
				color = COL8_FFFFFF;
			}
			/*定位 close btn 到右上角*/
			buf[(y + 5) * xsize + (xsize - 21 +x)] = color;
		}
	}
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


void putfont8_string_sht(struct SHEET *sht,int x, int y,unsigned char color,unsigned char back_ground_color, char *str,int strLength)
{
	boxfill8(sht->buf, sht->bxsize, back_ground_color,x, y, x + 8 * strLength , y + 16);
	putfont8_string(sht->buf,sht->bxsize,x,y,color,str );
	sheet_refresh(sht,x, y, x + 8 * strLength , y + 16);
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
		vram_temp = vram + (y + i) * xsize + x;
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

void init_screen(unsigned char *vram,int xsize,int ysize)
{
   
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

void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c)
{
	int x1 = x0 + sx, y1 = y0 + sy;
	boxfill8(sht->buf, sht->bxsize, COL8_848484, x0 - 2, y0 - 3, x1 + 1, y0 - 3);
	boxfill8(sht->buf, sht->bxsize, COL8_848484, x0 - 3, y0 - 3, x0 - 3, y1 + 1);
	boxfill8(sht->buf, sht->bxsize, COL8_FFFFFF, x0 - 3, y1 + 2, x1 + 1, y1 + 2);
	boxfill8(sht->buf, sht->bxsize, COL8_FFFFFF, x1 + 2, y0 - 3, x1 + 2, y1 + 2);
	boxfill8(sht->buf, sht->bxsize, COL8_000000, x0 - 1, y0 - 2, x1 + 0, y0 - 2);
	boxfill8(sht->buf, sht->bxsize, COL8_000000, x0 - 2, y0 - 2, x0 - 2, y1 + 0);
	boxfill8(sht->buf, sht->bxsize, COL8_C6C6C6, x0 - 2, y1 + 1, x1 + 0, y1 + 1);
	boxfill8(sht->buf, sht->bxsize, COL8_C6C6C6, x1 + 1, y0 - 2, x1 + 1, y1 + 1);
	boxfill8(sht->buf, sht->bxsize, c,           x0 - 1, y0 - 1, x1 + 0, y1 + 0);
	return;
}
