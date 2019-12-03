#include "bootpack.h"
#include <stdio.h>

struct TIMER *mt_timer;
struct TASK_CTL *taskctl;


void task_idle(void)
{
	while(1){
		io_hlt();
	}
}



struct TASK *mt_init(struct MEMMAN *man,struct FIFO32 *fifo32 )
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
	
	for(i = 0 ; i < MAX_TASK_LEV ;i ++){
		taskctl->task_levels[i].now = 0;
		taskctl->task_levels[i].taskcount = 0;
	}
	
	
	taskctl->current_level = 0;
	task = task_alloc(fifo32);
	task->priority = 1;
	task->level = 0;
	task->status = TASK_STATUS_RUNNING;
	task_add(task);
	
	mt_tastswitchsub();
	
	load_tr(task->segment);
	mt_timer = timer_alloc();
	settime(mt_timer, task->priority);
	
	
	struct TASK *ideltask = task_alloc(0);
	ideltask->tss.eip = (int)&task_idle;
	ideltask->tss.esp = memman_alloc_4k(man, 64 * 1026) + 64 * 1024;
	ideltask->tss.es = 1 * 8;
	ideltask->tss.cs = 2 * 8;
	ideltask->tss.ss = 1 * 8;
	ideltask->tss.ds = 1 * 8;
	ideltask->tss.fs = 1 * 8;
	ideltask->tss.gs = 1 * 8;
	task_run(ideltask,MAX_TASK_LEV - 1,2);
	return task;
}


struct TASK* task_alloc(struct FIFO32 *fifo32)
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
			task->fifo32 = fifo32;
			return task;
		}
	}
	
	return 0;
	
}


void task_sleep(struct TASK* task)
{
	if (task == 0 ){
		return;
	}
	struct TASK* now;
	if(task->status == TASK_STATUS_RUNNING){
		now =  task_now();
		task_remove(task);
		
		if(task == now){
			mt_tastswitchsub();
			now =  task_now();
			farjmp(0,now->segment);
		}
	}
	return;
}

void task_run(struct TASK* task,int level, int priority)
{
	if(level < 0 ){
		level = task->level;
	}
	
	if(priority > 0){
		task->priority = priority;
	}
	
	if(task->status == TASK_STATUS_RUNNING && task->level != level){
		task_remove(task);
	}
	
	if(task->status != TASK_STATUS_RUNNING){
		task->level = level;
		task_add(task);
	}
	
	taskctl->need_change_level = 1;
	
	return;
}


void task_add(struct TASK* task)
{
	struct TASK_LEVEL *task_level = &taskctl->task_levels[task->level];
	task_level->tasks[task_level->taskcount] = task;
	task_level->taskcount ++;
	task->status = TASK_STATUS_RUNNING;
	return;
}



void task_remove(struct TASK* task)
{
	if (task == 0 ){
		return;
	}
	
	struct TASK_LEVEL *tasklev = &(taskctl->task_levels[task->level]);
	
	
	
	if(task->status == TASK_STATUS_RUNNING){
		
		int i;
		
		/*find out the task's index*/
		for(i = 0; i < tasklev->taskcount; i ++){
			if(tasklev->tasks[i] == task){
				break;
			}
		}


		if(i < tasklev->now){
			tasklev->now --;
		}
		
		/*Remove the task from taskctl->tasks*/
		tasklev->taskcount --;
		for(;i < tasklev->taskcount;i ++){
			tasklev->tasks[i] = tasklev->tasks[i+ 1];
		}
		
		if(tasklev->now >= tasklev->taskcount){
			tasklev->now = 0;
		}
		task->status = TASK_STATUS_SLEEP;
		
	}
	return;
	
}

struct TASK* task_now(void)
{
	struct TASK_LEVEL task_level_now = taskctl->task_levels[taskctl->current_level];
	return task_level_now.tasks[task_level_now.now];	
}

void mt_tastswitch(void)
{
	
	struct TASK_LEVEL *task_level_now = &(taskctl->task_levels[taskctl->current_level]);
	struct TASK *nextTask, *nowTask = task_now();
	task_level_now->now ++;
	
	if(task_level_now->now >= task_level_now->taskcount){
		task_level_now->now = 0;
	}
	
	if(taskctl->need_change_level){
		mt_tastswitchsub();
		task_level_now = &(taskctl->task_levels[taskctl->current_level]);
	}
	
	nextTask = task_level_now->tasks[task_level_now->now];
	settime(mt_timer, nextTask->priority);
	if(nextTask != 0 && nextTask != nowTask){
		farjmp(0,nextTask->segment);
	}
	
	return;
}

void mt_tastswitchsub(void)
{
	int i;
	for(i = 0; i < MAX_TASK_LEV ;i ++){
		if(taskctl->task_levels[i].taskcount > 0){
			break;
		}
	}
	
	taskctl->current_level = i;
	taskctl->need_change_level = 0;
}








