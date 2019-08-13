#include "bootpack.h"

#define PIT_CTL		0x0043
#define	PIT_CNT0	0x0040


struct TIMERCTL timerctl;

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
	return;
}

void inthandler20(int *esp)
{
	io_out8(PIC0_OCW2,0x60 + 0); //通知 主PIC IRQ-0 中断处理完毕。
	
	timerctl.count++;
	
	return;
}