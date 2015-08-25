#include "..\include\z.h"

#include "..\include\hd.h"
#include "..\include\io.h"
#include "..\include\irq.h"
//for test
#include "..\include\vbe.h"
#include "..\include\misc.h"

#include "..\include\msg.h"
#include "..\include\timer.h"


#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <dbg.h>






unsigned char NR_HD_DEV;
struct HardDiskStruct HardDiskInfo[2];
static char HardDiskName[16];

static volatile int reading=0;


#define MAIN_PART 4
struct  disk_partition hd[MAIN_PART+1];


unsigned char ide_sector_read_buffer[SECTOR_SIZE];
unsigned char ide_sector_write_buffer[SECTOR_SIZE];


struct partitiontypes partition_types[] = {
    {0, "Unknow Partition"},
    {1, "DOS 12-bit FAT"},		/* Primary DOS with 12-bit FAT */
    {2, "XENIX /"},			/* XENIX / filesystem */
    {3, "XENIX /usr"},			/* XENIX /usr filesystem */
    {4, "DOS 16-bit FAT <32M"},		/* Primary DOS with 16-bit FAT */
    {5, "DOS Extended"},		/* DOS 3.3+ extended partition */
    {6, "DOS 16-bit FAT >=32M"},
    {7, "OS/2 IFS (e.g., HPFS) or NTFS or QNX2 or Advanced UNIX"},
    {8, "AIX boot or SplitDrive"},
    {9, "AIX data or Coherent"},
    {0x0a, "OS/2 Boot Manager or Coherent swap"},
    {0x0b, "Microsoft Windows FAT32"},
    {0x0c, "Windows FAT32 (lba)"},
    {0x0d, "Windows FAT16(lba)"},
    {0x0e, "MSDOS FAT16. CHS-mapped"},
    {0x0f, "Ext. partition, CHS-mapped"},
    {0x10, "OPUS"},
    {0x11, "OS/2 BM: hidden DOS 12-bit FAT"},
    {0x12, "Compaq diagnostics"},
    {0x14, "OS/2 BM: hidden DOS 16-bit FAT <32M"},
    {0x16, "OS/2 BM: hidden DOS 16-bit FAT >=32M"},
    {0x17, "OS/2 BM: hidden IFS"},
    {0x18, "AST Windows swapfile"},
    {0x24, "NEC DOS"},
    {0x3c, "PartitionMagic recovery"},
    {0x40, "Venix 80286"},
    {0x41, "Linux/MINIX (sharing disk with DRDOS)"},
    {0x42, "SFS or Linux swap (sharing disk with DRDOS)"},
    {0x43, "Linux native (sharing disk with DRDOS)"},
    {0x50, "DM (disk manager)"},
    {0x51, "DM6 Aux1 (or Novell)"},
    {0x52, "CP/M or Microport SysV/AT"},
    {0x53, "DM6 Aux3"},
    {0x54, "DM6"},
    {0x55, "EZ-Drive (disk manager)"},
    {0x56, "Golden Bow (disk manager)"},
    {0x5c, "Priam Edisk (disk manager)"}, /* according to S. Widlake */
    {0x61, "SpeedStor"},
    {0x63, "GNU HURD or Mach or Sys V/386 (such as ISC UNIX)"},
    {0x64, "Novell Netware 286"},
    {0x65, "Novell Netware 386"},
    {0x70, "DiskSecure Multi-Boot"},
    {0x75, "PC/IX"},
    {0x77, "QNX4.x"},
    {0x78, "QNX4.x 2nd part"},
    {0x79, "QNX4.x 3rd part"},
    {0x80, "MINIX until 1.4a"},
    {0x81, " MINIX Version 2. "},
    {0x82, "Unite Journal File System  Jicama native"},
    {0x83, "Linux native:Ext2 Fs"},
    {0x84, "OS/2 hidden C: drive"},
    {0x85, "Linux extended"},
    {0x86, "NTFS volume set??"},
    {0x87, "NTFS volume set??"},
    {0x90, "???????"},
    {0x93, "Amoeba"},
    {0x94, "Amoeba BBT"},		/* (bad block table) */
    {0xa0, "IBM Thinkpad hibernation"}, /* according to dan@fch.wimsey.bc.ca */
    {0xa5, "BSD/386"},			/* 386BSD */
    {0xa7, "NeXTSTEP 486"},
    {0xb7, "BSDI fs"},
    {0xb8, "BSDI swap"},
    {0xc1, "DRDOS/sec (FAT-12)"},
    {0xc4, "DRDOS/sec (FAT-16, < 32M)"},
    {0xc6, "DRDOS/sec (FAT-16, >= 32M)"},
    {0xc7, "Syrinx"},
    {0xdb, "CP/M or Concurrent CP/M or Concurrent DOS or CTOS"},
    {0xe1, "DOS access or SpeedStor 12-bit FAT extended partition"},
    {0xe3, "DOS R/O or SpeedStor"},
    {0xe4, "SpeedStor 16-bit FAT extended partition < 1024 cyl."},
    {0xf1, "SpeedStor"},
    {0xf2, "DOS 3.3+ secondary"},
    {0xf4, "SpeedStor large partition"},
    {0xfe, "SpeedStor >1024 cyl. or LANstep"},
    {0xff, "Xenix Bad Block Table"},
	
    {0x44,"Z File System 32"}
};


void hd_interrupt_handler(void);                                               //硬盘中断句柄
static void ide_write_to_port(unsigned char *pbuffer);
static void ide_read_from_port(unsigned char *pubuffer);
//-------------------------------------------------------------------------------------------------------------------------------
//
//  字符交换
//
//-------------------------------------------------------------------------------------------------------------------------------
void SwapChar(unsigned char* toswap)
{
	int      i = 0;
	unsigned char temp;

	while (toswap[i+1] != 0)
	{
		  temp = toswap[i];
		  toswap[i] = toswap[i+1];
		  toswap[i+1] = temp;

		  i += 2;
    }

	return;
}

//-------------------------------------------------------------------------------------------------------------------------------
//
//  检测硬盘存在函数
//
//-------------------------------------------------------------------------------------------------------------------------------
int	GetIDENum(void)
{
	 unsigned short    buffer[256];
	 char              *text;
	 unsigned char     c_disks;
 
   c_disks = io_readcmos(0x12);
	
	if (c_disks & 0xf0){
		if (c_disks & 0x0f){
			NR_HD_DEV = 2;
		}
		else{
			NR_HD_DEV = 1;
	   }
    }
	else{
		  NR_HD_DEV = 0;
		  return 0;
	}

	io_writebyte(0x1F6,0xA0); // Get first/second drive 
	io_writebyte(0x1F7,0xEC);  // 获取驱动器信息数据 
    
	MEMSET(HardDiskInfo, 0, sizeof(struct HardDiskStruct));

	while (io_readbyte(HD_STATUS)&0x80);
	insw(0x1f0, buffer, 256);  //将数据读到缓冲区
	HardDiskInfo[0].sect = buffer[6];
	HardDiskInfo[0].head = buffer[3];
	HardDiskInfo[0].cyl =  buffer[1];

	text = (unsigned char*)&buffer[27];
	SwapChar(text);
	STRCPY(HardDiskName, text);
  HardDiskName[15]=0;

	return NR_HD_DEV;
}


//-------------------------------------------------------------------------------------------------------------------------------
//
//  读取硬盘信息函数
//
//-------------------------------------------------------------------------------------------------------------------------------
void	ReadIDEInfo(void)
{
	int i, partsize, sect_count;

	sect_count = HardDiskInfo[0].head * HardDiskInfo[0].sect * HardDiskInfo[0].cyl;
	partsize =   sect_count /2048;
	PRINTF("  Harddisk Information:\n");
	PRINTF("  Total Space:%i MB\n", partsize );
	PRINTF("  Sectors:%i  Heads:%i Cylinder:%i \n\n", 
	HardDiskInfo[0].sect, HardDiskInfo[0].head, HardDiskInfo[0].cyl);

	 for (i = 1; i < 5; i++)
	 {
		 PRINTF("  PARTITION [%c:] start sect: %d size: %d type: %s\n",
			             hd[i].name, hd[i].lowsec,hd[i].nr_sects / 2048,  hd[i].type);
	 }
	 PRINTF("\n");
}

//-------------------------------------------------------------------------------------------------------------------------------
//
//  读取硬盘已激活分区信息函数
//
//-------------------------------------------------------------------------------------------------------------------------------
static int FindActivePartition(struct partition *p, int i)
{
		if(p->indicator == 0x80){
			PRINTF("  Active partition : %d \n", i);
			return 1;
		}
		return 0;
}

//-------------------------------------------------------------------------------------------------------------------------------
//
//  读取硬盘分区类型名称函数
//
//-------------------------------------------------------------------------------------------------------------------------------
static char* GetPartitionName(unsigned char type)
{
       struct partitiontypes *pp;
       for (pp = partition_types; pp->type<0xff; pp++)
            if (pp->type == type)
	          return pp->name;
       return "  bad file system or disk.we can't read it!!";
}

//-------------------------------------------------------------------------------------------------------------------------------
//
//  读取硬盘分区信息函数
//
//-------------------------------------------------------------------------------------------------------------------------------
int ReadPartition(void)
{
	int i = 0;
	int sector, head, cylinder;
	unsigned char buffer[512];
	struct partition* p;

#ifndef HD_BOOT_SECTOR
#define HD_BOOT_SECTOR 0
#endif

  //misc_printf("Check the hard disk partition\n");

	sector = HD_BOOT_SECTOR &0xff;
	cylinder = (HD_BOOT_SECTOR &0xffff00)>>8;
	head = (HD_BOOT_SECTOR &0xf000000)>>24;
  
	while(io_readbyte(HD_STATUS) & 0x80);
	io_wait();
	io_writebyte(HD_CMD, 0);
	io_writebyte(0x1f2, 1);                   ///////////读写的扇区数.这个端口应该是:0X1F2,下面开始增加一
	io_writebyte(0x1f3, HD_BOOT_SECTOR);      ////////开始扇区
	io_writebyte(0x1f4, cylinder);            ///////开始柱面
	io_writebyte(0x1f5, cylinder>>8);         ///////开始柱面高位
	io_writebyte(0x1f6,0xE0|(0 <<4)|head);   /////主磁盘
	io_writebyte(0x1F7, 0x20);                ////read命令
  while(io_readbyte(HD_STATUS) & 0x80);
	insw(0x1F0, buffer, 256);

	if (buffer[510] != (unsigned char) 0x55 || buffer[511] != (unsigned char) 0xAA){
		//misc_printf(".FAULT");
		//misc_printf("  read_partition(): Can't Read Harddisk(IDE) partition data\n");
		return -1;
	}
  

	for (i=1;i<5;i++)
	{

	   p = (struct partition*)&buffer[0x1be + (i-1)*0x10]; //partition table offset after 446 byte.
		 hd[i].start_sect = p->start_sec;  	                 // 从0算起的绝对扇区(分区表的起始扇区，不是FAT表的)
		 hd[i].start_head=p->start_head;
		 hd[i].start_cyl=p->start_cyl;
		 hd[i].nr_sects = p->size;                           // 分区扇区总数 
		 hd[i].flag = p->type;
		 hd[i].lowsec = p->lowsec;
		 hd[i].type = GetPartitionName( p->type);
	   hd[i].name = 0x42+i;
	   FindActivePartition(p, i);
	}

	//ReadIDEInfo();

	return 0;
}

//-------------------------------------------------------------------------------------------------------------------------------
//
//  硬盘中断处理函数
//
//-------------------------------------------------------------------------------------------------------------------------------
void hd_interrupt_handler(void)
{
	//硬盘中断好像没啥用
	//vga_puts("ide interrupt occuring!\n");
	//if(reading){

	//	reading=0;
	//	ide_read_from_port(ide_sector_read_buffer);
		//vga_puts("read from io port !\n");
	//}
}


//-------------------------------------------------------------------------------------------------------------------------------
//
//  硬盘读写处理函数
//
//-------------------------------------------------------------------------------------------------------------------------------
void hd_rw(int RW, unsigned long sectors, void* buffer)
{
  unsigned eflag;
	int sector, head, cylinder;

	irq_disable();
	io_saveflag(eflag);

	sector   = sectors &0xff;
	cylinder = (sectors &0xffff00)>>8;
	head     = (sectors &0xf000000)>>24;

	while(io_readbyte(HD_STATUS) & 0x80);
	io_wait();
	io_writebyte(HD_CMD,0);

	io_writebyte(0x1f2, 1);                 ///////////读写的扇区数.这个端口应该是:0X1F2,下面开始增加一
	io_writebyte(0x1f3,sector);            ////////开始扇区
	io_writebyte(0x1f4,cylinder);          ///////开始柱面
	io_writebyte(0x1f5,cylinder>>8);       ///////开始柱面高位
	io_writebyte(0x1f6,0xE0|(0 <<4)|head); /////主磁盘

	reading=1;

	if (RW == READ)
	{                              ////read命令
	    io_writebyte(0x1F7,0x20);         
        while(io_readbyte(HD_STATUS) & 0x80);
	    ide_read_from_port(buffer);
	}
	else if (RW == WRITE)
	{                             ////write命令

	if (io_readbyte(HD_STATUS & 0x01)) {
		PRINTF("  issue write_command command error !\n");
		return;
	}

		 io_writebyte(0x1F7,0x30);   
	   while((io_readbyte(HD_STATUS) & 0x08) == 0);       //DRQ
	     ide_write_to_port(buffer);
     }
	else {
		PRINTF("  Panic:bad command:you only can read or write on harddisk!");
	}
	io_restoreflag(eflag);
	irq_enable();
	return;
}



//-------------------------------------------------------------------------------------------------------------------------------
//
//  硬盘初始化函数
//
//-------------------------------------------------------------------------------------------------------------------------------
void hd_init (void)
{
	
	TRACE();

	if(GetIDENum()==0) 
	{
		timer_sleep(1000);
		SHOW_FAULT();
		PRINTF("  NO hard disk connetted!\n");
    
	  return;}
	timer_sleep(1000);
	SHOW_OK();


	if(NR_HD_DEV==1)
	PRINTF("  System only one hard disk:");
	else if(NR_HD_DEV==2)
	PRINTF("  System have two hard disk:");

	PRINTF(" %s", HardDiskName );


	//for(i=0 ; i<NR_BLK_REQUEST ; i++) {
	//	hd_req[i].flag = -1;
	//	hd_req[i].next = NULL;
	//	semaphore_init (&(hd_req[i].sem), 0);
	//}

	//semaphore_init(&request_sem, NR_BLK_REQUEST);
	irq_setirqfun(IRQ_HD,hd_interrupt_handler);
	
	io_writebyte_p(0x21,io_readbyte_p(0x21)&0xfb);
	io_writebyte(0xA1,io_readbyte_p(0xA1)&0xbf);

	io_kerneldelay();
	io_kerneldelay();
	//ReadPartition();                                            //Must before fs!!!
	

}


/*------------------------------------------------------------------------
 Procedure:     ide_read_from_port ID:1
 Purpose:
 Input:
 Output:
 Errors:
------------------------------------------------------------------------*/
static void ide_read_from_port(unsigned char *pbuffer)
{
#if 0
	vga_puts(" enter ide_read_from_port \n");
#endif
	insw(0x1F0, pbuffer, 512/2);
	return;
}

/*------------------------------------------------------------------------
 Procedure:     ide_write_to_port ID:1
 Purpose:
 Input:
 Output:
 Errors:
------------------------------------------------------------------------*/
static void ide_write_to_port(unsigned char *pbuffer)
{
	/* batch write to port */
	outsw(0x1F0, pbuffer, 512 / 2);
	return;
}
/*---------------------------------------------------------------------



---------------------------------------------------------------------*/
void getideinfo(void)
{
	ReadIDEInfo();
	return;
}
