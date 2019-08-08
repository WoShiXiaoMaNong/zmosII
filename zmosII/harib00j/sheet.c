#define MAX_SHEETS	256
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



void shtctl_init(struct MEMMAN *man, unsigned char *vram, int xsize, int ysize)
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
		ctl->sheet0[i].flasg = 0; /*标记成 未使用*/
	}
err:
	return ctl;
}