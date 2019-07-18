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

#define ADDR_BOOTINFO 0x0ff0
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
void io_stihlt(void);
int io_load_eflags(void);
void io_cli(void);
void io_sti(void);
void io_store_eflags(int eflags);
void io_out8(int port,int value);
char io_in8(int port);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);
void asm_inthandler21(void);
void asm_inthandler2c(void);
//graphic.c
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
void putfont8(unsigned char *vram,int xsize,int x, int y,unsigned char color,char *font);
void putfont8_ascii(unsigned char *vram,int xsize,int x, int y,unsigned char color,char c_ascii);
void putfont8_string(unsigned char *vram,int xsize,int x, int y,unsigned char color,unsigned char *msg);
void putblock8_8(unsigned char *vram,int vxsize,int block_x_size,int block_y_size,int px0,int py0, char *blockbuf,int bxsize);

//dsctbl.c
void init_gdtidt(void);
void set_segmdesc(struct SEGMENT_DESCRIPTOR* gdt,unsigned int limit,int base,int access);
void set_gatedesc(struct GATE_DESCRIPTOR *idt,int offset, int selector,int access);
#define ADDR_GDT		0x00270000
#define LIMIT_GDT		0xffff
#define ADDR_IDT		0x0026f800
#define LIMIT_IDT		0x7ff
#define LIMIT_BOOTPACK	0x0007ffff
#define	ADDR_BOOTPACK	0x00280000
#define AR_DATA32_RW	0x4092
#define AR_CODE32_ER	0x409a
#define AR_INTGATE32	0x008e


/* int.c */
struct KEYBUF{
	unsigned char data[32];
	int next;
};
void init_pic(void);
void inthandler21(int *esp);
void inthandler2c(int *esp);

#define PIC0_ICW1		0x0020
#define PIC0_OCW2		0x0020
#define PIC0_IMR		0x0021
#define PIC0_ICW2		0x0021
#define PIC0_ICW3		0x0021
#define PIC0_ICW4		0x0021
#define PIC1_ICW1		0x00a0
#define PIC1_OCW2		0x00a0
#define PIC1_IMR		0x00a1
#define PIC1_ICW2		0x00a1
#define PIC1_ICW3		0x00a1
#define PIC1_ICW4		0x00a1
