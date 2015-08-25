#include <types.h>
#include <page.h>
#include <ntldr.h>

PAGE_DIR *pte_ptr = (PAGE_DIR *)PTE_DIR_BASE_PA;



//--------------------------------------------------------------------------------------------
//
// ���÷�ҳ�Ĵ���
//
//---------------------------------------------------------------------------------------------
void page_enable(void)
{
#if 0
  unsigned long cr0;

  __asm__ ("movl %%cr0, %0" : "=r" (cr0));
  __asm__ ("movl %%eax, %%cr3": :"a"(pte_ptr));

   /* set most significant bit of CR0 */
  cr0 |= 0x80000000;
  __asm__("movl %%eax, %%cr0": :"a"(cr0));
	__asm__ ("jmp  1f\n\t"\
		   "1:\n\t"\
			 "movl $1f,%eax\n\t"\
       "jmp *%eax\n\t"\
			 "1:");
#endif
  __asm__ __volatile__("mov %0, %%eax"::"m"(pte_ptr) );
  __asm__ __volatile__("mov %eax, %cr3");

}




//--------------------------------------------------------------------------------------------
//
// ҳĿ¼��ʼ��
//
//---------------------------------------------------------------------------------------------
void page_init(void)
{
  BOOL rc = TRUE;
  DWORD index = 0;
  PAGE_DIR *dir_ptr = pte_ptr;
  DWORD dir_num = PAGE_L1_ENTRIES;
  SYSTEMCONFIG *syscfg = ntldr_get_syscfg();
  DWORD phy_mem_size = syscfg->phy_mem_size;
  DWORD max_mem = syscfg->linear_mem_max_size;

  // ����ڴ�С��Ҫ�����С�ڴ�,�Ͳ�������
  if (syscfg->phy_mem_min_size > phy_mem_size)
  {
    NTLDR_HALT();
  }
  
  //��ʼ���ں�Ŀ¼��
  page_init_dir();

  //ӳ��ϵͳ�ε��ڴ�
  //1,ӳ�������ڴ�0-2M�������ڴ��0-2M
  rc = page_maping(0,0,0x200000,PAGE_ATTR_PRESENT | PAGE_ATTR_WRITE);
  if (TRUE != rc)
  {
    NTLDR_HALT();
  }


  //2,ӳ�������ڴ��2-32M�������ڴ��2G-2.030G
  rc = page_maping(0x200000,0x80000000,0x1e00000,PAGE_ATTR_PRESENT | PAGE_ATTR_WRITE);
  if (TRUE != rc)
  {
    NTLDR_HALT();
  }


  //3,������ҳ
  page_enable();

  SetCr3(GetCr3());

  //4,ˢ��gdi,idt

  //irq_setirqfun(irq_,IRQ_FUN fn)
}


//-----------------------------------------------------------------------



//-----------------------------------------------------------------------
unsigned long GetCr3(void)
{
  register unsigned long x;
  __asm__ __volatile__ ("movl %%cr3,%0" : "=r" (x));
  return (x);
}

void page_init_dir()
{
  DWORD index = 0;
  PAGE_DIR *dir_ptr = pte_ptr;
  PAGE_TABLE *tab_ptr = (PAGE_TABLE *)(dir_ptr++);

  
  for (index = 0; index < PAGE_L1_ENTRIES; index++)
  {
    dir_ptr->v = 0;
	dir_ptr->a.physicalAddress = ((DWORD)tab_ptr >> 20);
	dir_ptr++;
	tab_ptr+= PAGE_L2_ENTRIES;
  }
}

BOOL page_maping(DWORD phy_addr, DWORD linear_addr, DWORD size, DWORD attr)
{
  PAGE_DIR *dir_ptr = pte_ptr;
  PAGE_TABLE *tab_ptr = NULL;
  DWORD page_num = 0;
  DWORD ii = 0;
  DWORD jj = 0;
  DWORD tmp_phy_addr = phy_addr;
  DWORD tmp_linear_addr = linear_addr;

  // ӳ����ڴ泤�ȱ����ܱ�һ��ҳ������
  // ����,�߼���ַҲ�����谴4k����
  if (0 == size ||
  	  !PAGE_VALID_MAPPING_SIZE(size) ||
  	  !PAGE_VALID_MAPPING_SIZE(phy_addr) ||
  	  !PAGE_VALID_MAPPING_SIZE(linear_addr) ||
  	  !PAGE_ACTION_VALID_ATTR(attr))
  {
    return FALSE;
  }

  // 
  page_num = size/PAGE_SIZE;
  
  for (jj = 0; jj < page_num; jj++)
  {
    dir_ptr = pte_ptr + PAGE_LINEAR_TO_PDT(tmp_linear_addr);
    if (!PAGE_ACTION_IS_PRESENT(dir_ptr->v))
    {
      dir_ptr->v = attr;
    }

	// ָ��ǰҳ��ͷ
	tab_ptr = (PAGE_TABLE *)((DWORD)dir_ptr->a.physicalAddress << 20);
	
	tab_ptr += PAGE_LINEAR_TO_PET(tmp_linear_addr);

	tab_ptr->v = tmp_phy_addr;
	tab_ptr->v |= attr;
	
    //NTLDR_PRINTF("-d=%d,lin=%x-\n",PAGE_LINEAR_TO_PDT(tmp_linear_addr),
	//	tmp_linear_addr);
	
    tmp_phy_addr+=PAGE_SIZE;

	tmp_linear_addr+=PAGE_SIZE;

  }
  
  return TRUE;
}



