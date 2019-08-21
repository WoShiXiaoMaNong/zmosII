#include "bootpack.h"

#define PIT_CTL		0x0043
#define	PIT_CNT0	0x0040


struct TIMERCTL timerctl;

struct FIFO32 *timerfifo;
int timerdata0;


void init_pit(void)
{
	/*定时器8254芯片固定设定方法
	 *设置定时器为 每x 毫秒产生一次中断
	 * x = 主频/设定的数值
	 * 这里设定数值为 0x2e9c (即 11932)
	 * 主频为1.19318KHz
	 * 大约每秒100次中断。
	*/
	io_out8(PIT_CTL,0x34);
	io_out8(PIT_CNT0,0x9c);//中断周期低8位
	io_out8(PIT_CNT0,0x2e);//中断周期高8位。
	timerctl.count = 0;
	timerctl.using = 0;
	
	int i;
	for(i = 0 ; i < MAX_TIMER ; i++){
		timerctl.timer0[i].flag = TIMER_NOT_USED;
	}
	
	return;
}
/*
struct TIMER{
	unsigned int timeout,flag;
	struct FIFO8 *fifo;
	unsigned char data;
};

struct TIMERCTL{
	unsigned int count,using;
	struct TIMER timers[MAX_TIMER];
};
*/
void inthandler20(int *esp)
{
	io_out8(PIC0_OCW2,0x60 + 0); //通知 主PIC IRQ-0 中断处理完毕。
	timerctl.count++;
	int i;
	for(i = 0 ; i < timerctl.using ; i++){
		if( timerctl.timers[i]->timeout <= timerctl.count ){
			fifo32_put(timerctl.timers[i]->fifo,timerctl.timers[i]->data + timerdata0);
			timerctl.timers[i] = timerctl.timers[i + 1];
		}else{
			break;
		}
	}
	
	
	timerctl.using -= i;
	
	return;
}

struct TIMER *timer_alloc(void)
{
	int i;
	for(i = 0 ; i < MAX_TIMER ; i++){
		if (timerctl.timer0[i].flag == TIMER_NOT_USED){
			timerctl.timer0[i].flag = TIMER_USED;
			timerctl.timer0[i].fifo = timerfifo;
			return & (timerctl.timer0[i]);
		}
	}
	return 0;
}
void timer_free(struct TIMER *timer)
{
	//tbd
}

void timer_init(struct FIFO32 *fifo,int data0)
{
	timerfifo = fifo;
	timerdata0 = data0;
}

void settime(struct TIMER *timer,unsigned int timeout, int data)
{
	if(timer->flag == TIMER_NOT_USED){
		return;
	}
	
	timer->timeout = timeout + timerctl.count;
	timer->data = data;
	int i,j;
	for(i = 0 ; i < timerctl.using ; i++){
		if( timerctl.timers[i]->timeout >= timer->timeout ){
			break;
		}
	}
	for(j = timerctl.using ; j >= i ;j--){
		timerctl.timers[j] = timerctl.timers[j - 1];
	}
	timerctl.timers[i] = timer;
	timerctl.using ++;
	return;
}





