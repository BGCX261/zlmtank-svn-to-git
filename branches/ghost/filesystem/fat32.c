#include "..\include\z.h"
#include "..\include\fat.h"
#include "..\include\vbe.h"
#include "..\include\misc.h"
#include "..\include\hd.h"

#include <memory.h>
#include <string.h>
#include <kconsole.h>
#include <stdio.h>
#include <z.h>

#define NR_FILE 32

static FILE MsOpenFile[NR_FILE];

static MSDOS_INFO32 Fat32_Info;

static unsigned char Fat32_RootDir[1024];

void PrintsFat32info(void);

void Fat32_LoadRootDir(void);
void Fat32_PrintRootDir(void);
//---------------------------------------------------------------------------------------------------------------------
//
//  Fat32 文件系统初始化
//
//---------------------------------------------------------------------------------------------------------------------
int fat32_init(void)
{
	unsigned char buffer[512];
	MSDOS_SUPER32* sp;

	TRACE();
	
	Fat32_Info.fs_base = hd[1].lowsec;

	MEMSET( buffer , 0 , 512);

	hd_rw( READ , Fat32_Info.fs_base , buffer);

	sp= (MSDOS_SUPER32*)buffer;

	Fat32_Info.dos_sp.fats = sp->fats;
	Fat32_Info.cluster_size = sp->cluster_size;       //cluster size
	Fat32_Info.blk_size = sp->cluster_size*512;       //every block size.

	Fat32_Info.dos_sp.hidden = sp->hidden;           //unused
	Fat32_Info.dos_sp.reserved =  sp->reserved;

	Fat32_Info.dos_sp.fat16_length = sp->fat16_length;  //fat size
	Fat32_Info.dos_sp.fat32_length = sp->fat32_length;  //fat size
  	misc_strcpy(Fat32_Info.dos_sp.oem, sp->oem);

	Fat32_Info.dos_sp.total_sectors = sp->total_sectors;
	Fat32_Info.sector_size = sp->sector_size[0] + (sp->sector_size[1]<<8);
	Fat32_Info.dir_entries = sp->dir_entries[0] + (sp->dir_entries[1]<<8);

	Fat32_Info.fat_base =  Fat32_Info.fs_base + Fat32_Info.dos_sp.reserved;   
  
     if (sp->fat16_length != 0)
	{

		 Fat32_Info.flag = FAT16;
		 Fat32_Info.fat_size = sp->fat16_length;
	 	Fat32_Info.fat_entries = sp->fat16_length*512/(sizeof(unsigned short));

	  	//if(Fat32_Info.fat_entries <= FAT12_MAGIC)
    		//msdos_loadfat12hd(dev);

	 	Fat32_Info.fat_root =  Fat32_Info.fat_base + (sp->fats*sp->fat16_length);  //Value is sector
   		Fat32_Info.blk_base =  Fat32_Info.fat_root + 32;  //32 origin: (dos_info.dir_entries*32)/512;
	 	//dos_root= msdos_iget(dev, FAT12_ROOT);
	 }
	 else
	 {

		 Fat32_Info.flag = FAT32;
		 Fat32_Info.fat_size = sp->fat32_length;
		 Fat32_Info.fat_entries = ((int)sp->fat32_length*512) / sizeof(unsigned long);

	  	 Fat32_Info.fat_root = sp->fat32_root_cluster;  //But this value is cluster.
    		 Fat32_Info.blk_base = Fat32_Info.fat_base + ((int)sp->fats * (int)sp->fat32_length);
		 //dos_root = msdos_iget(dev, dos_info.fat_root);
	}

		 //super[dev].root = dos_root;
      		//super[dev].ops = &msdos_ops;	

  	MEMSET(MsOpenFile,0,32*sizeof(FILE));

	SHOW_OK();	
  	if(Fat32_Info.flag==FAT16)PUTS("     FILMSYSTEM : FAT16");	

	if(Fat32_Info.flag==FAT32)PUTS("     FILMSYSTEM : FAT32");
	PUTS("\n");
	PrintsFat32info();
	
	Fat32_LoadRootDir();
	Fat32_PrintRootDir();

	
	
	return 1;
}
//
//---------------------------------------------------------------------------------------------------------------------
//
//  显示 Fat32 文件系统信息
//
//---------------------------------------------------------------------------------------------------------------------
void PrintsFat32info(void)
{
	PRINTF("  fat oem is : %s\n",Fat32_Info.dos_sp.oem);
	PRINTF("  fat cluster size is : %ld\n",Fat32_Info.cluster_size);
	PRINTF("  fat fat filenum is : %ld\n",Fat32_Info.dos_sp.fats);
	if(Fat32_Info.flag==FAT16)
	PRINTF("  fat length   is : %ld\n",Fat32_Info.dos_sp.fat16_length);
	if(Fat32_Info.flag==FAT32)
	PRINTF("  fat length   is : %ld\n",Fat32_Info.dos_sp.fat32_length);
	PRINTF("  fat blk  base  is : %ld\n",Fat32_Info.blk_base);
	PRINTF("  fat root enter is : %ld\n",Fat32_Info.fat_root);
	PRINTF("  fat fat  enter is : %ld\n",Fat32_Info.fat_base);
	return;
}

//---------------------------------------------------------------------------------------------------------------------
//
//  Fat32 簇号转扇区号
//
//---------------------------------------------------------------------------------------------------------------------
int BlkToSec( int cluster )
{
	if(cluster > Fat32_Info.fat_entries)
	   PUTS("A Bad Fat32 Cluster!\n");

    if ( cluster > 1 )
	    return Fat32_Info.blk_base+(cluster-2) * Fat32_Info.cluster_size;
    
	return Fat32_Info.fat_root;  //root sector
}

//---------------------------------------------------------------------------------------------------------------------
//
//  读取软盘根目录
//
//---------------------------------------------------------------------------------------------------------------------
void Fat32_LoadRootDir(void)
{
	int i;
	unsigned char* p;
	//Fat32_RootDir=(MSDOS_DIR*)VMalloc(1024);
	p=Fat32_RootDir;
		PRINTF(" FAT32_rootdirentry=%d",Fat32_Info.fat_root);
		hd_rw(READ,BlkToSec(Fat32_Info.fat_root),p);
		p+=512;
		hd_rw(READ,BlkToSec(Fat32_Info.fat_root+1),p);
	
	
	return;
}


void Fat32_PrintRootDir(void)
{
	MSDOS_DIR* p=NULL;
	p=(MSDOS_DIR*) Fat32_RootDir;
	int i=0;
	PUTS("void Fat32_PrintRootDir(void)\n");
	
	while(p->file_name[0]!=0x00)
	{
		PRINTF("  Fat32 Root Dir File[%d] : %s\n",i++,(char*)p->file_name);
		p++;
	}
	return;
}


void getfat32info(void)
{
	PrintsFat32info();
	return;
}
