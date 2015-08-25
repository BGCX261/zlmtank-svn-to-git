#ifndef BOOTINFO_H
#define BOOTINFO_H

//要与def.inc里面定义保持一致
#define BOOTINFO_PTR 0x600




typedef struct _e820_map_item
{
  DWORD mem_base_low32;
  DWORD mem_base_up32;
  DWORD mem_len_low32;
  DWORD mem_len_up32;
  DWORD mem_type;
}E820_MAP_ITEM;


typedef struct _bootinfo
{
    BYTE	bootdriver;
	DWORD	e801_mem_len;
	DWORD	e820_map_num;
	E820_MAP_ITEM	e820_map_ptr[1];

}BOOTINFO;



#endif
