#include "bootpack.h"


struct TIMER *mt_timer;
int mt_tr;

void mt_init(void)
{
	mt_timer = timer_alloc();
	settime(mt_timer, 2);
	mt_tr = 3 * 8;	
	return;
}


void mt_tastswitch(void)
{
	if(mt_tr == 3 << 3){
		mt_tr = 4 << 3;
	}else{
		mt_tr = 3 << 3;
	}
	settime(mt_timer, 2);
	farjmp(0,mt_tr);
	return;
}