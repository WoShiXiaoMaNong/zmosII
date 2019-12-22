#include "bootpack.h"

void fifo8_init(struct FIFO8 *fifo8,char *buf, int size)
{
	fifo8->buf = buf;
	fifo8->size = size;
	fifo8->free = size;
	fifo8->flags = 0;
	fifo8->p = 0;  //Next_w
	fifo8->q = 0;  //Next_r
	
}

int fifo8_put(struct FIFO8 *fifo8,char data)
{
	if(fifo8->free <= 0){
		fifo8->flags = -1;
		return -1;
	}
	
	fifo8->buf[fifo8->p] = data;
	fifo8->p ++;
	fifo8->free --;
	if(fifo8->p == fifo8->size){
		fifo8->p = 0;
	}
	return 0;
	
}

char fifo8_get(struct FIFO8 *fifo8)
{
	if(fifo8->free >= fifo8->size){
		return -1;
	}
	char data = fifo8->buf[fifo8->q];
	fifo8->q ++;
	fifo8->free ++;
	
	if(fifo8->q == fifo8->size){
		fifo8->q = 0;
	}
	return data;
}

int fifo8_status(struct FIFO8 *fifo8)
{
	
	return fifo8->size - fifo8->free;
	
}


void fifo32_init(struct FIFO32 *fifo32,int *buf, int size,struct TASK* task)
{
	fifo32->buf = buf;
	fifo32->size = size;
	fifo32->free = size;
	fifo32->flags = 0;
	fifo32->p = 0;  //Next_w
	fifo32->q = 0;  //Next_r
	fifo32->task = task;
}


int fifo32_put(struct FIFO32 *fifo32,int data)
{
	if(fifo32->free == 0){
		fifo32->flags = -1;
		return -1;
	}
	
	fifo32->buf[fifo32->p] = data;
	fifo32->p ++;
	fifo32->free --;
	if(fifo32->p == fifo32->size){
		fifo32->p = 0;
	}
	
	if(fifo32->task != 0){
		if(fifo32->task->status != TASK_STATUS_RUNNING){
			task_run(fifo32->task,-1,0);
		}
		
	}
	
	
	return 0;
}
int fifo32_get(struct FIFO32 *fifo32)
{
	if(fifo32->free == fifo32->size){
		return -1;
	}
	int data = fifo32->buf[fifo32->q];
	fifo32->q ++;
	fifo32->free ++;
	
	if(fifo32->q == fifo32->size){
		fifo32->q = 0;
	}
	return data;
}
int fifo32_status(struct FIFO32 *fifo32)
{
	return fifo32->size - fifo32->free;
}