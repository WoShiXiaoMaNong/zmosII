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
	int old,i;
	

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


void sheet_refresh(struct STCTL *ctl, int vx0, int vy0, int vx1, int vy1)
{
	int h;
	struct SHEET *sht;
	for(h = 0 ; h <= ctl->top ; h++){
		sht = ctl->sheets[h];
		if(sht->flags == SHEET_USED){
			sheet_refresh_sub(ctl,sht,vx0,vy0,vx1,vy1);
		}
	}
}

void sheet_refresh_sub(struct STCTL *ctl,struct SHEET *sht, int vx0, int vy0, int vx1, int vy1)
{
	int by,bx, vy,vx,notOverSize;
	unsigned char *buf,c,*vram = ctl->vram;
	
	buf = sht->buf;
	for(by = 0 ;by < sht->bysize ; by++){
		vy = sht->vy0 + by;
		for(bx = 0 ; bx < sht->bxsize ; bx ++){
			vx = sht->vx0 + bx;
			c = buf[by * sht->bxsize + bx];
			notOverSize = (vx >=0 && vy >= 0 && vx < (ctl->xsize) && vy < (ctl->ysize));
			if(vx >= vx0 && vx <= vx1 && vy >= vy0 && vy <= vy1 && notOverSize){
				if(c != sht->col_inv){
					vram[vy * ctl->xsize + vx] = c;
				}
			}
		}
	}
	
}

void sheet_slide(struct STCTL *ctl, struct SHEET *sht, int vx0, int vy0)
{
	int oldx = sht->vx0,oldy = sht->vy0;
	sht->vx0 = vx0;
	sht->vy0 = vy0;
	if(sht->flags >= 0){
		sheet_refresh(ctl,oldx, oldy,oldx + sht->bxsize, oldy + sht->bysize);
		sheet_refresh(ctl,vx0, vy0,vx0 + sht->bxsize, vy0 + sht->bysize);
	}
	return;
}

void sheet_free(struct STCTL *ctl,struct SHEET *sht)
{
	if(sht->height >= 0){
		sheet_updown(ctl,sht,-1);
	}
	sht->flags = 0;
	return;
}



