#ifndef _BOOTPACK_H
#define _BOOTPACK_H

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

#define EFLAGS_AC_BIT 		 0x00040000
#define  CR0_CACHE_DISABLE   0x60000000

/*Keyboard & Mouse start */
#define PORT_KEYDAT     		0x0060
#define PORT_KEYSTA     		0x0064
#define PORT_KEYCMD     		0x0064
#define KEYSTA_SEND_NOTREADY   0x02
#define KEYCMD_WRITE_MODE     	0x60
#define KBC_MODE     			0x47
#define KEYCMD_SENDTO_MOUSE		0xd4
#define MOUSECMD_ENABLE			0xf4
/*Keyboard & Mouse end   */


/*Memory.c config */
#define MEMMAN_FREES		4090
#define MEMMAN_ADDR			0x003c0000

/*sheet.c*/
#define MAX_SHEETS	256
#define SHEET_USED	1
#define SHEET_NOT_USED	0

/*timer.c*/
#define MAX_TIMER	256
#define TIMER_USED	1
#define TIMER_NOT_USED	0


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

struct TSS32{
	int backlink, esp0, ss0, esp1, ss1, esp2, ss2, cr3;
	int eip, eflags, eax, ecx, edx,ebx, esp, ebp, esi, edi;
	int es, cs, ss, ds, fs, gs;
	int ldtr,iomap;
	
};


/*fifo.c*/
struct FIFO8{
	char *buf;
	int p,q,size,free,flags;	
};

struct FIFO32{
	int *buf;
	int p,q,size,free,flags;	
};
/*memory.c*/
struct FREEINFO{
	unsigned int addr,size;
};

struct MEMMAN{
	int frees,maxfrees, lostsize, losts;
	struct FREEINFO free[MEMMAN_FREES];
};

/*sheet.c*/
struct SHEET{
	unsigned char *buf;
	int sid;
	struct STCTL *ctl;
	int bxsize,bysize, vx0,vy0,col_inv,height,flags;
};


struct STCTL{
	unsigned char *vram, *map;
	int xsize,ysize,top;
	struct SHEET *sheets[MAX_SHEETS];
	struct SHEET sheet0[MAX_SHEETS];
};

/*timer.c*/
struct TIMER{
	struct TIMER *next;
	unsigned int timeout,flag;
	struct FIFO32 *fifo;
	unsigned char data;
};

struct TIMERCTL{
	unsigned int count;
	struct TIMER timer0[MAX_TIMER];
	struct TIMER *head;
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
int load_cr0(void);
void store_cr0(int cr0); 
void asm_inthandler20(void);
void asm_inthandler21(void);
void asm_inthandler2c(void);
int memtest_sub(unsigned start,unsigned end);
void load_tr(int tr);
void taskswitch3(void);
void taskswitch4(void);

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
void init_screen(unsigned char *vram,int xsize,int ysize);
void init_mouse_cursor8(char *mouse,char back_ground_color);
void putfont8(unsigned char *vram,int xsize,int x, int y,unsigned char color,char *font);
void putfont8_ascii(unsigned char *vram,int xsize,int x, int y,unsigned char color,char c_ascii);
void putfont8_string(unsigned char *vram,int xsize,int x, int y,unsigned char color,unsigned char *msg);
void putfont8_string_sht(struct SHEET *sht,int x, int y,unsigned char color,unsigned char back_ground_color, char *str,int strLength);
void putblock8_8(unsigned char *vram,int vxsize,int block_x_size,int block_y_size,int px0,int py0, char *blockbuf,int bxsize);
void create_windows8(unsigned char *buf,int xsize,int ysize,char *title);
void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c);

/*sheet.c*/
struct STCTL *shtctl_init(struct MEMMAN *man, char *vram, int xsize, int ysize);
struct SHEET *sheet_alloc(struct STCTL *ctl);
void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv);
void sheet_updown(struct SHEET *sht,int height);
void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bx1, int by1);
void sheet_refresh_sub(struct STCTL *ctl, int vx0, int vy0, int vx1, int vy1,int h0,int h1);
void sheet_refresh_map(struct STCTL *ctl, int vx0, int vy0, int vx1, int vy1,int h0);
void sheet_slide(struct SHEET *sht, int vx0, int vy0);
void sheet_free(struct SHEET *sht);
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
#define AR_TSS32		0x0089

/* int.c */
void init_pic(void);



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



/*FIFO.c */

void fifo8_init(struct FIFO8 *fifo8, char *buf, int size);
int fifo8_put(struct FIFO8 *fifo8, char data);
char fifo8_get(struct FIFO8 *fifo8);
int fifo8_status(struct FIFO8 *fifo8);

void fifo32_init(struct FIFO32 *fifo32,int *buf, int size);
int fifo32_put(struct FIFO32 *fifo32,int data);
int fifo32_get(struct FIFO32 *fifo32);
int fifo32_status(struct FIFO32 *fifo32);

/*timer.c*/
void init_pit(void);
void inthandler20(int *esp);
void settime(struct TIMER *timer,unsigned int timeout);
struct TIMER * timer_alloc(void);
void timer_free(struct TIMER *timer);
void timer_init(struct TIMER *timer,struct FIFO32 *fifo,int data);


/*mouse.c*/
struct MOUSE_DESC{
	int buf[3],phase;
	int x,y,btn;
};
void inthandler2c(int *esp);
int mouse_decode(struct MOUSE_DESC *mdec, int data);
void enable_mouse(struct MOUSE_DESC *mdec,struct FIFO32 *fifo, int data0);

/*keyboard.c*/
void init_keyboard(struct FIFO32 *fifo, int data0);
void wait_KBC_sendready(void);
void inthandler21(int *esp);


/*memory.c*/
void memman_init(struct MEMMAN *man);
int memman_total(struct MEMMAN *man);
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size);
int memman_free(struct MEMMAN *man ,unsigned int addr,unsigned int size);
unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size);
int memman_free_4k(struct MEMMAN *man ,unsigned int addr,unsigned int size);
unsigned int memtest(unsigned int start, unsigned int end);

#endif
