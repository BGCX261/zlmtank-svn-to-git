#ifndef _PAGE_H
#define _PAGE_H

#define PAGE_ATTR_PRESENT	(1<<0)	//ҳ�����
#define PAGE_ATTR_WRITE		(1<<1)	//ҳ���д
#define PAGE_ATTR_USER		(1<<2)	//ҳ��Ϊ�û���
//#define PAGE_ATTR_PWT       (1<<3)  //page write through д�ڴ�ʱ˳��ˢ�¸��ٻ���,�������д�������֧��
//#define PAGE_ATTR_PCD       (1<<4)  //page cache disable.�������ж�֧��
#define PAGE_ATTR_ACCESS	(1<<5)	//ҳ�汻���ʹ�
//#define PAGE_ATTR_PSE       (1<<7)  //=1ʱ,ҳ��ָ�����4M��ҳ

//�Զ���
//#define PAGE_ATTR_GLOBAL	(1<<8)	//ȫ��ҳ
//#define PAGE_ATTR_SHARE		(1<<9)	//ϵͳ���壺����ҳ
//#define PAGE_ATTR_COPYONWRITE	(1<<11)	//ϵͳ���壺дʱ����

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


#define PAGE_LINEAR_TO_PDT(ADDR) ((ADDR >> 22) & 0x3ff)   //��10λ��ҳĿ¼����
#define PAGE_LINEAR_TO_PET(ADDR) ((ADDR >> 12) & 0x3ff)   //��11-21λ��ҳĿ¼����



#define SetCr3(x) {\
	      __asm__ __volatile__ (\
        "movl %%eax,%%cr3\n"\
		    : :"a" ( x ));}

//ˢ��ҳ����ٻ�����,ֻҪ��ҳ��Ļ�ַ�ٴ�����cr3�Ĵ���,�ͻ��Զ�ˢ��
#define ReloadPageDir(pg_dir) \
        __asm__("movl %%eax,%%cr3": :"a"(pg_dir))




DWORD GetCr3(void);




void page_init(void);
void page_init_dir();
BOOL page_maping(DWORD phy_addr, DWORD linear_addr, DWORD size, DWORD attr);
void page_fault();

#endif

