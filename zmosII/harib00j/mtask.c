#include "bootpack.h"
#include <stdio.h>

struct TIMER *mt_timer;
struct TASK_CTL *taskctl;

void mt_init(struct MEMMAN *man )
{
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADDR_GDT;
	taskctl = (struct TASK_CTL*)memman_alloc_4k(man, sizeof(struct TASK_CTL));
	
	
	struct TASK *task;
	int i;
	for(i = 0 ; i < MAX_TASK ;i ++){
		taskctl->task0[i].status = TASK_STATUS_FREE;
		taskctl->task0[i].segment = (TASK_GDT0 + i) * 8;
		set_segmdesc(gdt + TASK_GDT0 + i, 103, (int) &taskctl->task0[i].tss, AR_TSS32);
		
		
	}
	taskctl->taskcount = 0;
	taskctl->now = 1;
	task = task_alloc();
	task->priority = 2;
	task_run(task);
	load_tr(task->segment);
	mt_timer = timer_alloc();
	settime(mt_timer, task->priority);
	
	return;
}


struct TASK* task_alloc(void)
{
	int i;
	struct TASK *task;
	for(i = 0; i < MAX_TASK; i++){
		if(taskctl->task0[i].status == TASK_STATUS_FREE){
			task = &taskctl->task0[i];
			task->status = TASK_STATUS_ALLOCATED;
			task->tss.ldtr = 0;
			task->tss.iomap =  0x40000000;
			task->tss.eflags = 0x00000202; /* IF = 1; */
		
			task->tss.eip =0;
			task->tss.eax = 0;
			task->tss.ecx = 0;
			task->tss.edx = 0;
			task->tss.ebx = 0;
			task->tss.esp = 0;
			task->tss.ebp = 0;
			task->tss.esi = 0;
			task->tss.edi = 0;
			task->tss.es = 0;
			task->tss.cs = 0;
			task->tss.ss = 0;
			task->tss.ds = 0;
			task->tss.fs = 0;
			task->tss.gs = 0;
			return task;
		}
	}
	
	return 0;
	
}


void task_sleep(struct SHEET* sheet,struct TASK* task)
{
	if (task == NULL ){
		return;
	}
	int ts = 0; /* ts:task_switch  1-> need switch task, 0-> no need*/
		
	if(task->status == TASK_STATUS_RUNNING){
		task->status = TASK_STATUS_SLEEP;
		if( task == taskctl->tasks[taskctl->now]){
			ts = 1;
		}
		
		int i;
		
		/*find out the task's index*/
		for(i = 0; i < taskctl->taskcount; i ++){
			if(taskctl->tasks[i] == task){
				break;
			}
		}


		if(i < taskctl->now){
			taskctl->now --;
		}
		
		/*Remove the task from taskctl->tasks*/
		taskctl->taskcount --;
		for(;i < taskctl->taskcount;i ++){
			taskctl->tasks[i] = taskctl->tasks[i+ 1];
		}
		
		if(taskctl->now >= taskctl->taskcount){
			taskctl->now = 0;
		}
		
		if(ts != 0 ){
			//farjmp(0,taskctl->tasks[taskctl->now]->segment);
		}
	}
	return;
}

void task_run(struct TASK* task)
{
	taskctl->tasks[taskctl->taskcount] = task;
	task->status = TASK_STATUS_RUNNING;
	taskctl->taskcount ++;
	return;
}


void mt_tastswitch(void)
{
	struct TASK *nextTask = taskctl->tasks[taskctl->now];
	taskctl->now ++;
	
	if(taskctl->now >= taskctl->taskcount){
		taskctl->now = 0;
	}

	settime(mt_timer, 2);
	if(nextTask != 0){
		farjmp(0,nextTask->segment);
	}
	
	return;
}










