#ifndef SYSCFG_H
#define SYSCFG_H

//�ڴ�ֲ�
//LA : Linear Address
//PA : Physical Address

#define SYSTEM_MAX_MEM_SIZE 0xffffffff   //4G
//#define SYSTEM_MIN_MEM_SIZE 0x03f00000   //64M ....bios�����ȡ������64m�ڴ�������ֵ
#define SYSTEM_MIN_MEM_SIZE 0x03ec0000   //64M ....bios�����ȡ������64m�ڴ�������ֵ
//boch��������0x3f00000
//virtual machine��������0x3ec0000


//
//  KERNEL segment
//
#define	KERNEL_BASE_LA   0x80000000       //3G
#define KERNEL_BASE_PA   0x00000000       //0
#define KERNEL_BASE_SIZE 0x02000000       //32M

//
//  esp
//
#define ESP_BASE_LA   0x90000
#define ESP_BASE_PA   0x90000
#define ESP_BASE_SIZE //....


//
//  BIOS
//
#define BIOS_BASE_LA  0x000000
#define BIOS_BASE_PA  0x000000
#define BIOS_BASE_SIZE 0x100000


//
//  NTLDR
//
#define NTLDR_BASE_LA  BIOS_BASE_LA+BIOS_BASE_SIZE  //0x0010 0000
#define NTLDR_BASE_PA  BIOS_BASE_PA+BIOS_BASE_SIZE  //0x0010 0000
#define NTLDR_BASE_SIZE 0x100000

//
//  PTE
//
#define PTE_BASE_LA KERNEL_BASE_LA                   //0x8000 0000
#define PTE_BASE_PA NTLDR_BASE_PA + NTLDR_BASE_SIZE  //0x0020 0000

  
  #define PTE_DIR_BASE_LA PTE_BASE_LA  //0x8000 0000
  #define PTE_DIR_BASE_PA PTE_BASE_PA  //0x0020 0000
  #define PTE_DIR_BASE_SIZE 1024*sizeof(DWORD) //0x1000
  
  #define PTE_TAB_BASE_LA PTE_DIR_BASE_LA + PTE_DIR_BASE_SIZE //0x8000 1000
  #define PTE_TAB_BASE_PA PTE_DIR_BASE_PA + PTE_DIR_BASE_SIZE  //0x0020 1000
  #define PTE_TAB_BASE_SIZE 1024*1024*sizeof(DWORD) //0x400000

#define PTE_BASE_SIZE PTE_DIR_BASE_SIZE + PTE_TAB_BASE_SIZE //401000



//
//  GDT
//
#define GDT_ITEM_NUM 256
#define GDT_BASE_LA PTE_BASE_LA + PTE_BASE_SIZE //0x8040 1000
#define GDT_BASE_PA PTE_BASE_PA + PTE_BASE_SIZE //0x0060 1000
#define GDT_BASE_SIZE 8*GDT_ITEM_NUM  //32��gdt slot,ÿ��dword*2      2048


//
//  IDT
//
#define IDT_ITEM_NUM 256
#define IDT_BASE_LA GDT_BASE_LA + GDT_BASE_SIZE      //0x8040 1800
#define IDT_BASE_PA GDT_BASE_PA + GDT_BASE_SIZE      //0x0060 1800
#define IDT_BASE_SIZE 4*IDT_ITEM_NUM



//
//  GDT system seletor
//
#define KERNEL_CODE_SEL 0x8
#define KERNEL_DATA_SEL 0x10

#define GDT_KERNEL_CODE_SEL KERNEL_CODE_SEL
#define GDT_KERNEL_DATA_SEL KERNEL_DATA_SEL
#define IDT_KERNEL_CODE_SEL KERNEL_CODE_SEL
#define IDT_KERNEL_DATA_SEL KERNEL_DATA_SEL







#include <bootinfo.h>
typedef struct _syscfg
{
  // ������Ϣָ��
  BOOTINFO *boot_info;
  
  //���������������
  DWORD boot_drive_no;
  
  //��չ�ڴ��С(in kb).64λ�汾������
  DWORD phy_mem_size;

  // ϵͳҪ�����С�ڴ�
  DWORD phy_mem_min_size;

  //�����ڴ���󳤶�
  DWORD linear_mem_max_size;
}SYSTEMCONFIG;


#endif
