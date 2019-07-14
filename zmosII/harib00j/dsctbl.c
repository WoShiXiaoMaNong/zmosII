#include "bootpack.h"

void init_gdtidt(void)
{
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) 0x00270000;
	struct GATE_DESCRIPTOR *idt = (struct GATE_DESCRIPTOR *) 0x0026f800;
	int i;
	/*init gdt 
	  gdt 最大数量为8192个。
	  段寄存器为16位，低3位有特殊用途，13位 = 8192（0 ~ 8191）
	*/
	for(i = 0 ; i<8192 ; i++){
		set_segmdesc(gdt + i,0,0,0);
	}
	set_segmdesc(gdt + 1,0xffffffff,0x00000000,0x4092);//1# 即 1_000b 0~4G内存
	set_segmdesc(gdt + 2,0x0007ffff,0x00280000,0x4092);//2# 0x00280000 ~ 0x0007ffff:asmhead.nas预留空间
	
	load_gdtr(0xffff, 0x00270000);/*limit = 0xffff (8192(gdt count) * 8(gdt size) ),base addr = 0x0027000*/
	
	/*init idt,idt 最大数量为256,idt（中断表） 暂时全部留空*/
	for(i = 0 ; i < 256; i++){
		set_gatedesc(idt + i,0,0,0);
	}
	
	load_idtr(0x7ff, 0x0026f800);/*limit = 0x7ff (256(idt count) * 8(gdt size) ),base addr = 0x0026f800*/
	
}

void set_segmdesc(struct SEGMENT_DESCRIPTOR* gdt,unsigned int limit,int base,int access)
{
	if (limit > 0xfffff) {
		access |= 0x8000; /* G_bit = 1 */
		limit /= 0x1000;
	}
	gdt->limit_low    = limit & 0xffff;
	gdt->base_low     = base & 0xffff;
	gdt->base_mid     = (base >> 16) & 0xff;
	gdt->access_right = access & 0xff;
	gdt->limit_high   = ((limit >> 16) & 0x0f) | ((access >> 8) & 0xf0);
	gdt->base_high    = (base >> 24) & 0xff;
	return;
}

void set_gatedesc(struct GATE_DESCRIPTOR *idt,int offset, int selector,int access)
{
	idt->offset_low   = offset & 0xffff;
	idt->selector     = selector;
	idt->dw_count     = (access >> 8) & 0xff;
	idt->access_right = access & 0xff;
	idt->offset_high  = (offset >> 16) & 0xffff;
	return;
}

