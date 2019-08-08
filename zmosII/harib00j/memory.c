#include "bootpack.h"


void memman_init(struct MEMMAN *man){
	man->frees = 0;
	man->maxfrees = 0;
	man->lostsize = 0;
	man->losts = 0;
	return;
}


int memman_total(struct MEMMAN *man){
	int total = 0;
	int i;
	for(i = 0; i < man->frees ; i++){
		total += man->free[i].size;
	}
	return total;
}

unsigned int memman_alloc(struct MEMMAN *man, unsigned int size){
	int i,j;
	unsigned addr;
	for(i = 0 ; i < man->frees; i ++){
		if(man->free[i].size >= size){
			addr = man->free[i].addr;
			man->free[i].addr += size;
			man->free[i].size -=size;
			
			/*当前free 分配完的时候，清除它*/
			if(man->free[i].size == 0){
				man->frees --;
				for(j = i ; i < man->frees; i ++){
					man->free[i] = man->free[i + 1];
				}
			}
			
			return addr; 
		}
	}
	return 0;
	
}

int memman_free(struct MEMMAN *man ,unsigned int addr,unsigned int size)
{
	int i,j;
	for(i = 0 ; i < man->frees; i++){
		if(man->free[i].addr > addr){
			break;
		}
	}
	
	/*前面有free的内存*/
	if(i > 0){
		
		/*可与前面的free内存合并*/
		if(man->free[i-1].addr + size == addr){
			man->free[i-1].size += size;
			
			/*后面有free的内存*/
			if(i < man->frees){
				/*可与后面的free内存合并*/
				if(man->free[i].addr == addr + size){
					man->free[i-1].size += man->free[i].size;
					man->frees --;
					for(;i< man->frees ; i++){
						man->free[i] = man->free[i+1];
					}
				}
			}
			return 0;
		}
	}
	
	if(i < man->frees){
		if(man->free[i].addr == addr + size){
			man->free[i].addr -= size;
			man->free[i].size += size;
			return 0;
		}
	}
	
	if(man->frees < MEMMAN_FREES){
		for(j = man->free; j > i ;j --){
			man->free[j] = man->free[j - 1];
		}
		
		man->free[i].addr = addr;
		man->free[i].size = size;
		man->frees ++;
		return 0;
	}
	
	man->losts++;
	man->lostsize += size;
	return -1;
}


unsigned int memtest(unsigned int start, unsigned int end)
{
	char flg486 = 0;
	unsigned int eflags,cr0,memory_size;
	
	eflags = io_load_eflags();
	eflags =  eflags | EFLAGS_AC_BIT;
	io_store_eflags(eflags);
	eflags = io_load_eflags();
	
	if( (eflags & EFLAGS_AC_BIT) != 0 ){
		flg486 = 1;
	}
	eflags = eflags & ~EFLAGS_AC_BIT;
	io_store_eflags(eflags);
	
	if(flg486 != 0){
		cr0 = load_cr0();
		cr0 |= CR0_CACHE_DISABLE;
		store_cr0(cr0);
	}
	
	memory_size = memtest_sub(start, end);
	
	if(flg486 != 0){
		cr0 = load_cr0();
		cr0 &=  ~CR0_CACHE_DISABLE;
		store_cr0(cr0);
	}
	
	return memory_size;
}


/* roundup 4kb : 0x1000*/
unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size)
{
	size = (size + 0xfff) & 0xfffff000;
	return memman_alloc(man,size);
}

int memman_free_4k(struct MEMMAN *man ,unsigned int addr,unsigned int size)
{
	size = (size + 0xfff) & 0xfffff000;
	return memman_free(man,addr,size);
	
}


