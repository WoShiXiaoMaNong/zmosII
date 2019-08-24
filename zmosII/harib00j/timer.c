#include "bootpack.h"
#include <stdio.h>
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
	
	int i;
	for(i = 0 ; i < MAX_TIMER ; i++){
		timerctl.timer0[i].flag = TIMER_NOT_USED;
	}
	
	struct TIMER *t = timer_alloc();
	t->timeout = 0xffffffff;
	t->flag = TIMER_USED;
	t->next = NULL;
	timerctl.head = t;
	
	return;
}

void inthandler20(int *esp)
{
	io_out8(PIC0_OCW2,0x60 + 0); //通知 主PIC IRQ-0 中断处理完毕。
	timerctl.count++;
	
	struct TIMER *current = timerctl.head;
	while(current != NULL){
		if( current->timeout <= timerctl.count ){
			fifo32_put(current->fifo,current->data + timerdata0);
			timerctl.head = current->next;
			current->next = NULL;
			current = timerctl.head->next;
		}else{
			break;
		}
	}
	return;
}

/*
struct TIMER{
	struct TIMER *next;
	unsigned int timeout,flag;
	struct FIFO8 *fifo;
	unsigned char data;
};

struct TIMERCTL{
	unsigned int count;
	struct TIMER timer0[MAX_TIMER];
	struct TIMER *head;
};
*/

struct TIMER *timer_alloc(void)
{
	int i;
	for(i = 0 ; i < MAX_TIMER ; i++){
		if (timerctl.timer0[i].flag == TIMER_NOT_USED){
			timerctl.timer0[i].flag = TIMER_USED;
			timerctl.timer0[i].fifo = timerfifo;
			timerctl.timer0[i].next = NULL;
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
	

	struct TIMER *current = timerctl.head;
	
	if( timerctl.head->next == NULL || current->timeout >= timer->timeout){
			timerctl.head = timer;
			timer->next = current;
			return;
	}
	
	struct TIMER *next = current->next;
	
	while(next != NULL){
		if(next->timeout >= timer->timeout){
			break;
		}
		current = next;
		next = next->next;
	}
	
	current->next = timer;
	timer->next = next;
	
	
	
	return;
}





