#include "bootpack.h"



struct STCTL *shtctl_init(struct MEMMAN *man, char *vram, int xsize, int ysize)
{
	struct STCTL * ctl;
	ctl = (struct STCTL *)memman_alloc_4k(man,sizeof(struct STCTL));
	if(ctl == 0){
		goto err;
	}
	int i;
	ctl->vram = vram;
	ctl->xsize = xsize;
	ctl->ysize = ysize;
	ctl->top = -1;
	for(i = 0 ; i <MAX_SHEETS ; i++){
		ctl->sheet0[i].flags = SHEET_NOT_USED; /*标记成 未使用*/
		ctl->sheet0[i].ctl = ctl;
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
			sht->ctl = ctl;
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

void sheet_updown(struct SHEET *sht,int height)
{
	int old,i;
	struct STCTL *ctl = sht->ctl;

	if(height > ctl->top + 1){
		height = ctl->top + 1;
	}
	
	if(height < -1){
		height = -1;
	}
	old = sht->height;
	sht->height = height;
	
	if(old == height){
		return;
	}
	if(height >= MAX_SHEETS){
		return;
	}
	
	
	/*Sheet up*/
	if(old < height){
		if(old >= 0){
			for(i = old ; i < height ; i ++){
				ctl->sheets[i] = ctl->sheets[i + 1];
				ctl->sheets[i]->height = i;
			}
			ctl->sheets[height] = sht;
		}else{
			for(i = ctl->top; i >= height ; i--){
				ctl->sheets[i + 1] = ctl->sheets[i];
				ctl->sheets[i + 1]->height = i + 1;
			}
			ctl->sheets[height] = sht;
			ctl->top ++;
			
		}
		sheet_refresh(ctl,sht->vx0,sht->vy0,sht->vx0 + sht->bxsize,sht->vy0 + sht->bysize);
	}else if(old > height){	/*Sheet down */
		if(height >= 0 ){
			for(i = old ; i < height ; i --){
				ctl->sheets[i] = ctl->sheets[i + 1];
				ctl->sheets[i]->height = i;
			}
			ctl->sheets[height] = sht;
		}else{/*隐藏当前图层*/
			for(i = old; i < ctl->top ; i++){
				ctl->sheets[i] = ctl->sheets[i + 1];
				ctl->sheets[i] = i;
			}
			ctl->top --;
		}
		sheet_refresh(ctl,sht->vx0,sht->vy0,sht->vx0 + sht->bxsize,sht->vy0 + sht->bysize);
	}
	return;

	
}


void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bx1, int by1)
{
	
	if(sht->height >=0 ){
		sheet_refresh_sub(sht->ctl,bx0 + sht->vx0,by0 + sht->vy0,bx1 + sht->vx0,by1 + sht->vy0);
	}
	
}

void sheet_refresh_sub(struct STCTL *ctl, int vx0, int vy0, int vx1, int vy1)
{
	int h,by0,bx0,by1,bx1,by,bx, vy,vx,notOutOfScreen;
	unsigned char *buf,color,*vram = ctl->vram;
	struct SHEET *sht;
	for(h = 0 ; h <= ctl->top ; h++){
		sht = ctl->sheets[h];
		by0 = vy0 - sht->vy0;
		bx0 = vx0 - sht->vx0;
		by1 = vy1 - sht->vy0;
		bx1 = vx1 - sht->vx0;
		
		if(bx0 < 0) {bx0 = 0; }
		if(by0 < 0) {by0 = 0; }
		if(bx1 > sht->bxsize) {bx1 = sht->bxsize; }
		if(by1 > sht->bysize) {by1 = sht->bysize; }
			
		buf = sht->buf;
		for(by = by0 ;by < by1 ; by++){
			vy = sht->vy0 + by;
			for(bx = bx0 ; bx < bx1 ; bx ++){
				vx = sht->vx0 + bx;
				color = buf[by * sht->bxsize + bx];
				notOutOfScreen = (vx >=0 && vy >= 0 && vx < (ctl->xsize) && vy < (ctl->ysize));
				if(notOutOfScreen){
					if(color != sht->col_inv){
						vram[vy * ctl->xsize + vx] = color;
					}
				}
			}
		}
	}
}

void sheet_slide(struct SHEET *sht, int vx0, int vy0)
{
	int oldx = sht->vx0,oldy = sht->vy0;
	sht->vx0 = vx0;
	sht->vy0 = vy0;
	if(sht->flags >= 0){
		sheet_refresh_sub(sht->ctl,oldx, oldy,oldx + sht->bxsize, oldy + sht->bysize);
		sheet_refresh_sub(sht->ctl,vx0, vy0,vx0 + sht->bxsize, vy0 + sht->bysize);
	}
	return;
}

void sheet_free(struct SHEET *sht)
{
	if(sht->height >= 0){
		sheet_updown(sht,-1);
	}
	sht->flags = 0;
	return;
}



