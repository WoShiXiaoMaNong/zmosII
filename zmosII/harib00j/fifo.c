#include "bootpack.h"

void fifo8_init(struct FIFO8 *fifo8,unsigned char *buf, int size)
{
	fifo8->buf = buf;
	fifo8->size = size;
	fifo8->free = size;
	fifo8->flags = 0;
	fifo8->p = 0;  //Next_w
	fifo8->q = 0;  //Next_r
	
}

int fifo8_put(struct FIFO8 *fifo8,unsigned char data)
{
	if(fifo8->free == 0){
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

int fifo8_get(struct FIFO8 *fifo8)
{
	if(fifo8->free == fifo8->size){
		return -1;
	}
	int data = fifo8->buf[fifo8->q];
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