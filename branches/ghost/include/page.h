#ifndef _PAGE_H
#define _PAGE_H

#define PAGE_ATTR_PRESENT	(1<<0)	//页面存在
#define PAGE_ATTR_WRITE		(1<<1)	//页面可写
#define PAGE_ATTR_USER		(1<<2)	//页面为用户级
//#define PAGE_ATTR_PWT       (1<<3)  //page write through 写内存时顺便刷新高速缓存,不是所有处理器都支持
//#define PAGE_ATTR_PCD       (1<<4)  //page cache disable.不是所有都支持
#define PAGE_ATTR_ACCESS	(1<<5)	//页面被访问过
//#define PAGE_ATTR_PSE       (1<<7)  //=1时,页面指向的是4M的页

//自定义
//#define PAGE_ATTR_GLOBAL	(1<<8)	//全局页
//#define PAGE_ATTR_SHARE		(1<<9)	//系统定义：共享页
//#define PAGE_ATTR_COPYONWRITE	(1<<11)	//系统定义：写时复制

#define PAGE_ATTR_MASK (PAGE_ATTR_PRESENT | PAGE_ATTR_WRITE | PAGE_ATTR_USER | PAGE_ATTR_ACCESS)
#define PAGE_ACTION_VALID_ATTR(ATTR) (!(ATTR & ~PAGE_ATTR_MASK))

#define PAGE_ACTION_IS_PRESENT(P) (P & PAGE_ATTR_PRESENT)
#define PAGE_ACTION_PRESENT(P) (P | PAGE_ATTR_PRESENT)
#define PAGE_ACTION_UNPRESENT(P) (P & ~PAGE_ATTR_PRESENT)

typedef union PAGE_TABLE{
  DWORD v;
  struct{
  unsigned present:1;
  unsigned write:1;
  unsigned user:1;
  unsigned unused1:2;
  unsigned access:1;
  unsigned unused2:2;
  //unsigned global:1;
  //unsigned share:1;
  //unsigned allocated:1;
  //unsigned copyOnWrite:1;
  unsigned unused3:4;
  unsigned physicalAddress:20;
  }a;
}PAGE_TABLE, PAGE_DIR ;


#define PAGE_L1_ENTRIES   0x400 //1024
#define PAGE_L2_ENTRIES   0x400 //1024

#define PAGE_SIZE 4096
#define PAGE_VALID_MAPPING_ADDR(X) (0 == X % PAGE_SIZE)
#define PAGE_VALID_MAPPING_SIZE(X) (0 == X % PAGE_SIZE)


#define PAGE_LINEAR_TO_PDT(ADDR) ((ADDR >> 22) & 0x3ff)   //高10位是页目录索引
#define PAGE_LINEAR_TO_PET(ADDR) ((ADDR >> 12) & 0x3ff)   //高11-21位是页目录索引



#define SetCr3(x) {\
	      __asm__ __volatile__ (\
        "movl %%eax,%%cr3\n"\
		    : :"a" ( x ));}

//刷新页表高速缓冲区,只要将页表的基址再次送入cr3寄存器,就会自动刷新
#define ReloadPageDir(pg_dir) \
        __asm__("movl %%eax,%%cr3": :"a"(pg_dir))




DWORD GetCr3(void);




void page_init(void);
void page_init_dir();
BOOL page_maping(DWORD phy_addr, DWORD linear_addr, DWORD size, DWORD attr);
void page_fault();

#endif

