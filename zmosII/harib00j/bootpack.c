#include <stdio.h>
#include "bootpack.h"

#define MEMMAN_FREES		4090
#define MEMMAN_ADDR			0x003c0000


struct FREEINFO{
	unsigned int addr,size;
};

struct MEMMAN{
	int frees,maxfrees, lostsize, losts;
	struct FREEINFO free[MEMMAN_FREES];
};



void memman_init(struct MEMMAN *man){
	man->frees = 0;
	man->maxfrees = 0;
	man->lostsize = 0;
	man->losts = 0;
	return;
}


int memman_total(struct MEMMAN *man){
	int total = 0;
	int i;
	for(i = 0; i < man->frees ; i++){
		total += man->free[i].size;
	}
	return total;
}

unsigned int memman_alloc(struct MEMMAN *man, unsigned int size){
	int i,j;
	unsigned addr;
	for(i = 0 ; i < man->frees; i ++){
		if(man->free[i].size >= size){
			addr = man->free[i].addr;
			man->free[i].addr += size;
			man->free[i].size -=size;
			
			/*当前free 分配完的时候，清除它*/
			if(man->free[i].size == 0){
				man->frees --;
				for(j = i ; i < man->frees; i ++){
					man->free[i] = man->free[i + 1];
				}
			}
			
			return addr; 
		}
	}
	return 0;
	
}

int memman_free(struct MEMMAN *man ,unsigned int addr,unsigned int size)
{
	int i,j;
	for(i = 0 ; i < man->frees; i++){
		if(man->free[i].addr > addr){
			break;
		}
	}
	
	/*前面有free的内存*/
	if(i > 0){
		
		/*可与前面的free内存合并*/
		if(man->free[i-1].addr + size == addr){
			man->free[i-1].size += size;
			
			/*后面有free的内存*/
			if(i < man->frees){
				/*可与后面的free内存合并*/
				if(man->free[i].addr == addr + size){
					man->free[i-1].size += man->free[i].size;
					man->frees --;
					for(;i< man->frees ; i++){
						man->free[i] = man->free[i+1];
					}
				}
			}
			return 0;
		}
	}
	
	if(i < man->frees){
		if(man->free[i].addr == addr + size){
			man->free[i].addr -= size;
			man->free[i].size += size;
			return 0;
		}
	}
	
	if(man->frees < MEMMAN_FREES){
		for(j = man->free; j > i ;j --){
			man->free[j] = man->free[j - 1];
		}
		
		man->free[i].addr = addr;
		man->free[i].size = size;
		man->frees ++;
		return 0;
	}
	
	man->losts++;
	man->lostsize += size;
	return -1;
}


unsigned int memtest(unsigned int start, unsigned int end)
{
	char flg486 = 0;
	unsigned int eflags,cr0,memory_size;
	
	eflags = io_load_eflags();
	eflags =  eflags | EFLAGS_AC_BIT;
	io_store_eflags(eflags);
	eflags = io_load_eflags();
	
	if( (eflags & EFLAGS_AC_BIT) != 0 ){
		flg486 = 1;
	}
	eflags = eflags & ~EFLAGS_AC_BIT;
	io_store_eflags(eflags);
	
	if(flg486 != 0){
		cr0 = load_cr0();
		cr0 |= CR0_CACHE_DISABLE;
		store_cr0(cr0);
	}
	
	memory_size = memtest_sub(start, end);
	
	if(flg486 != 0){
		cr0 = load_cr0();
		cr0 &=  ~CR0_CACHE_DISABLE;
		store_cr0(cr0);
	}
	
	return memory_size;
}




void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADDR_BOOTINFO;
	struct MEMMAN *man = (struct MEMMAN *) MEMMAN_ADDR;
	struct MOUSE_DESC mdec;
	char s[30];
	unsigned char keybuff[36],mousebuff[36];
	char cursor[16][16];
	
	init_gdtidt();
	init_pic();
	io_sti();
	
	fifo8_init(&keyfifo,keybuff,36);
	fifo8_init(&mousefifo,mousebuff,36);
	
	/*由于 init_pic的时候 禁用了所有IRQ，这里需要手动开放需要的IRQ */
	io_out8(PIC0_IMR, 0xf9); /* PIC0(主PIC) 开放IRQ-1(键盘) IRQ-2(链接 从PIC) (11111001) */
	io_out8(PIC1_IMR, 0xef); /* PIC1(从PIC) 开放IRQ-12(鼠标) (11101111) */
	init_keyboard();
	enable_mouse(&mdec);
	init_palette();
	init_screen(binfo);
	
	unsigned int mem_total;
	mem_total = memtest(0x00400000,0xbfffffff);
	putfont8_string(binfo->vram,binfo->scrnx,0,100,COL8_FFFFFF,s );
	memman_init(man);
	
	memman_free(man,0x00001000,0x009e000);
	memman_free(man,0x00400000,mem_total - 0x00400000);
	sprintf(s,"Free Memory : %dKB",memman_total(man)/ 1024);
	putfont8_string(binfo->vram,binfo->scrnx,0,10,COL8_FFFFFF,s );
	
	/* init mouse start */
	int mx,my;
	mx = binfo->scrnx / 2 - 16;
	my = binfo->scrny / 2 - 16;
	
	init_mouse_cursor8(cursor,COL8_008484);
	putblock8_8(binfo->vram,binfo->scrnx,16,16,mx,my,cursor,16);
	/* init mouse end */
	
	int memsize = memtest(0x00400000,0xbfffffff) / 1024 / 1024;
	sprintf(s,"Memory : %dMB",memsize);
	putfont8_string(binfo->vram,binfo->scrnx,0,50,COL8_FFFFFF,s );
	
	unsigned data;
	int keyBufDataIndex;
	unsigned char mouse_data_buf[3], mouse_phase;
	mouse_phase = 0;
	while(1){
		io_cli();
		if( (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) )== 0){
			io_stihlt();
		}else{
			if( fifo8_status(&keyfifo) != 0){
				data = fifo8_get(&keyfifo);
				io_sti();
			
				boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 16, 15,31);
				sprintf(s,"%02X",data);
				putfont8_string(binfo->vram,binfo->scrnx,0,16,COL8_FFFFFF,s );
			}else if( fifo8_status(&mousefifo) != 0){
				data = fifo8_get(&mousefifo);
				io_sti();
				
				if( mouse_decode(&mdec, data) == 1){
					sprintf(s,"[lcr %4d %4d]",mdec.x,mdec.y);
					
					if( (mdec.btn & 0x01) != 0){
						s[1] = 'L';
					}
					if( (mdec.btn & 0x02) != 0){
						s[3] = 'R';
					}
					if( (mdec.btn & 0x04) != 0){
						s[2] = 'C';
					}
					
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 32, 16, 32 + 15* 8 -1,31);
					putfont8_string(binfo->vram,binfo->scrnx,32,16,COL8_FFFFFF,s );
					
					/*repaint mouse ---> Mouse move!!!*/
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484,  mx,         my,          mx + 15, my + 15);
					mx += mdec.x;
					my += mdec.y;
					putblock8_8(binfo->vram,binfo->scrnx,16,16,mx,my,cursor,16);
					
					
				}
			}
		}
	}
}

