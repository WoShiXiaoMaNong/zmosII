#include "bootpack.h"
#define MAX_SHEETS	256
#define SHEET_USED	1
#define SHEET_NOT_USED	0

struct SHEET{
	unsigned char *buf;
	int bxsize,bysize, vx0,vy0,col_inv,height,flags;
};


struct STCTL{
	unsigned char *vram;
	int xsize,ysize,top;
	struct SHEET *sheets[MAX_SHEETS];
	struct SHEET sheet0[MAX_SHEETS];
};



struct STCTL *shtctl_init(struct MEMMAN *man, unsigned char *vram, int xsize, int ysize)
{
	struct STCTL * ctl;
	ctl = (struct STCTL *)memman_alloc_4k(man,sizeof(struct STCTL));
	if(ctl == o){
		goto err;
	}
	int i;
	ctl->vram = vram;
	ctl->xsize = xsize;
	ctl->ysize = ysize;
	ctl->top = -1;
	for(i = 0 ; i <MAX_SHEETS ; i++){
		ctl->sheet0[i].flasg = SHEET_NOT_USED; /*标记成 未使用*/
	}
err:
	return ctl;
}

struct SHEET *sheet_alloc(struct STCTL *ctl)
{
	struct SHEET *sht;
	int i;
	for(i = 0 ; i < MAX_SHEETS ; i++){
		if(ctl->sheet0[i].flags == 0){
			sht = &(ctl->sheet0[i]);
			sht->flags = 1;
			sht->height = -1;
			return sht;
		}
	}
	return 0;
}

//int bxsize,bysize, vx0,vy0,col_inv,height,flags;
void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv)
{
	sht->buf = buf;
	sht->bxsize = xsize;
	sht->bysize = ysize;
	sht->col_inv = col_inv;
	return;
}

void sheet_updown(struct STCTL *ctl, struct SHEET *sht,int height)
{
	int old,h;
	if(height > ctl->top + 1){
		height = ctl->top + 1;
	}
	
	if(height < -1){
		height = -1;
	}
	old = sht->height;
	sht->height = height;
	
	//TBD
	
}


void sheet_refresh(struct STCTL *ctl)
{
	int h,by,bx, vy,vx;
	struct SHEET *sht;
	unsigned char *buf,c,vram = ctl->vram;
	for(h = 0 ; h < ctl->top ; h++){
		sht = ctl->sheets[h];
		buf = sht->buf;
		for(by = 0 ;by < sht->bysize ; by++){
			vy = sht->vy0 + by;
			
			for(bx = 0 ; bx < sht->bxsize ; bx ++){
				vx = sht->vx0 + bx;
				c = buf[by * sht->bxsize + bx];
				if(c != sht->col_inv){
					vram[vy * ctl->xsize + vx] = c;
				}
			}
			
		}
		
	}
}








