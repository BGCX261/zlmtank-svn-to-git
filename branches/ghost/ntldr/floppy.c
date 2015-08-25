#include <types.h>
#include <floppy.h>
#include <io.h>
#include <stdlib.h>
#include <irq.h>
#include <dma.h>
#include <ntldr.h>

#define ERROR_NO_FLOPPY		5
#define IoWait()			io_readbyte(0x80)



struct Floppy_Struct
{
	char type[8];
	unsigned char	tracks;
	unsigned char	heads;
	unsigned char	sectors;

	unsigned char	request_sector;
	unsigned char*	buffer;
} FloppyDev;



static unsigned char FloppyCashAddr[512];	//软盘高速缓冲区指针


#define MAX_REPLIES		7
static unsigned char FloppyReplyBuffer[MAX_REPLIES];
#define ST0 (FloppyReplyBuffer[0])
#define ST1 (FloppyReplyBuffer[1])
#define ST2 (FloppyReplyBuffer[2])
#define ST3 (FloppyReplyBuffer[3])


static char FloppyIncName[24];                                            //软驱型号名

static volatile int FloppyIntStatus=0;                                    //软驱中断状态字节

static int  FloppyMotor=0;                                                //软驱马达状态字节

void floppy_interrupt_handler(void);                                      //软驱中断句柄


int    InrType=0;

//-------------------------------------------------------------------------------------------------------------------------------
//
//  延迟函数
//
//-------------------------------------------------------------------------------------------------------------------------------
void floppy_delay(void)
{
	io_kerneldelay();
}

//-------------------------------------------------------------------------------------------------------------------------------
//
//  获得软驱响应状态
//
//-------------------------------------------------------------------------------------------------------------------------------
static int floppy_result(void)
{
	unsigned char stat, i,count;
	i=0;
	for(count=0;count<10000;count++){
		stat = io_readbyte( FD_STATUS ) & (STATUS_READY|STATUS_DIR|STATUS_BUSY); //读取状态寄存器
		if (stat == STATUS_READY)
			return i;
		if (stat == (STATUS_READY|STATUS_DIR|STATUS_BUSY))
		{
			if(i>7) break;
			FloppyReplyBuffer[i++]=io_readbyte_p(FD_DATA);
		}
	}

	return -1;
}

//-------------------------------------------------------------------------------------------------------------------------------
//
//  软驱等待中断完成
//
//-------------------------------------------------------------------------------------------------------------------------------
void floppy_waitint(void)
{

	for(;;)
	{
		if(FloppyIntStatus==1) {FloppyIntStatus=0;break;}
	}

	FloppyIntStatus=0;

	return;
}


//-------------------------------------------------------------------------------------------------------------------------------
//
//  软驱中断处理函数
//
//-------------------------------------------------------------------------------------------------------------------------------
void floppy_interrupt_handler(void)
{
	FloppyIntStatus=1;
}


//-------------------------------------------------------------------------------------------------------------------------------
//
//  向软驱控制寄存器发送一个控制字节
//
//-------------------------------------------------------------------------------------------------------------------------------
int floppy_sendbyte( int value )
{
	unsigned char stat, i;

	for ( i = 0; i < 128; i++ ) {
		stat = io_readbyte( FD_STATUS ) & (STATUS_READY|STATUS_DIR); //读取状态寄存器
		if  ( stat  == STATUS_READY )
		{
			io_writebyte( value ,FD_DATA);                             //将参数写入数据寄存器
			return 1;
		}
		floppy_delay();                                       // 作一些延迟
	}
	return 0;
}

//-------------------------------------------------------------------------------------------------------------------------------
//
//  从软驱数据寄存器得到一个数据字节
//
//-------------------------------------------------------------------------------------------------------------------------------
int floppy_readbyte(void)
{
	unsigned char stat, i;

	for ( i = 0; i < 128; i++ ) {
		stat = io_readbyte( FD_STATUS ) & (STATUS_READY|STATUS_DIR|STATUS_BUSY); //读取状态寄存器
		if (stat == STATUS_READY)
			return -1;
		if ( stat  == 0xD0 )
			return io_readbyte(FD_DATA);                    //获取数据
		floppy_delay();                            // 作一些延迟
	}
	return 0;
}

//-------------------------------------------------------------------------------------------------------------------------------
//
//  得到软驱信息
//
//-------------------------------------------------------------------------------------------------------------------------------
int floppy_getinfo(void)
{
	unsigned int i;
	unsigned char CmType, FdType;

	floppy_sendbyte(0x10);
	i = floppy_readbyte(); 

	switch (i)
	{
	           case 0x80:	strcpy(FloppyIncName,"NEC765A controller"); break;
	           case 0x90:   strcpy(FloppyIncName,"NEC765B controller"); break;
	           default:     strcpy(FloppyIncName,"Enhanced controller"); break;
	}

	io_writebyte(0x70,0x10);
	CmType = io_readbyte(0x71);
	//CmType = io_readcmos(0x10);     //read floppy type from cmos
	FdType   = (CmType>>4) & 0x07;

	if ( FdType == 0 )  return ERROR_NO_FLOPPY;

	switch( FdType )
	{
	case 0x02: // 1.2MB
		strcpy(FloppyDev.type,"1.2MB");
	break;

	case 0x04: // 1.44MB       标准软盘
		strcpy(FloppyDev.type,"1.44MB");
		break;

	case 0x05: // 2.88MB
		strcpy(FloppyDev.type,"2.88MB");
		break;
	}

	return 1;
}

//-------------------------------------------------------------------------------------------------------------------------------
//
//  软驱驱动初始化
//
//-------------------------------------------------------------------------------------------------------------------------------
void floppy_init(void)
{
	if(!floppy_getinfo()) {
		return;};

	irq_setirqfun(IRQ_FD,floppy_interrupt_handler);
	//io_writebyte_p(io_readbyte_p(0x21)&~0x40,0x21);

	NTLDR_PRINTF("%s:%s\n",FloppyIncName,FloppyDev.type);
}

//-------------------------------------------------------------------------------------------------------------------------------
//
//  打开软驱马达
//
//-------------------------------------------------------------------------------------------------------------------------------
void floppy_motoron( void )
{
	if (!FloppyMotor)
	{
		__asm__("cli");
		io_writebyte(28,FD_DOR);
		FloppyMotor = 1;
		__asm__("sti");
	}
	return;
}

//-------------------------------------------------------------------------------------------------------------------------------
//
//  关闭软驱马达
//
//-------------------------------------------------------------------------------------------------------------------------------
void floppy_motoroff( void )
{
	if (FloppyMotor)
	{
		irq_disable();
		io_writebyte(12,FD_DOR);
		FloppyMotor = 0;
		irq_enable();

	}
	return;
}


//-------------------------------------------------------------------------------------------------------------------------------
//
//  软驱寻道函数
//
//-------------------------------------------------------------------------------------------------------------------------------
int floppy_seek(int driver,int track,int head)
{
	int r1,r2;

	floppy_sendbyte( FD_SEEK);                                 //seek command
	floppy_sendbyte( head*4 + driver);
	floppy_sendbyte( track );
	floppy_waitint();

	
	floppy_sendbyte(FD_SENSEI);
	r1 = floppy_readbyte();
	r2 = floppy_readbyte();

	//vga_puts("  Seek r1=%d r2=%d\n",r1,r2);
	/*  r1 is status register 0  */
	if ((r1 & 0xf8)==0x20 && r2==track)    {return 1;}
	return 0;
}


//-------------------------------------------------------------------------------------------------------------------------------
//
//  软驱模式设置
//
//-------------------------------------------------------------------------------------------------------------------------------
static void floppy_setmode(void)
{   
	floppy_sendbyte (FD_SPECIFY);
	floppy_sendbyte (0xcf);
	floppy_sendbyte (0x06);
	io_writebyte (0,FD_DCR);
}


//-------------------------------------------------------------------------------------------------------------------------------
//
//  逻辑块转为磁盘头、磁道号和扇区号
//
//-------------------------------------------------------------------------------------------------------------------------------
void BlockToHTS(unsigned long block, int *head, int *track, int *sector )
{
	*head = ( block % ( 18 * 2 ) ) /18;
	*track =  block / ( 18 * 2 );
	*sector = block % 18 + 1;
}

//-------------------------------------------------------------------------------------------------------------------------------
//
//  软驱重置
//
//-------------------------------------------------------------------------------------------------------------------------------
/*
static void RecalibrateFloppy(void)
{
	floppy_sendbyte(FD_RECALIBRATE); //send recalibrate command 
	floppy_sendbyte(0);              // select driver A 
}
*/




//-------------------------------------------------------------------------------------------------------------------------------
//
//  设置软驱DMA通道
//
//-------------------------------------------------------------------------------------------------------------------------------
static void SetupFloppyDMA(void)
{  
	irq_disable();
	DisableDma(2);
	ClearDmaFF(2);
	SetDmaMode(2,DMA_MODE_READ);
	SetDmaAddr(2,(unsigned long)FloppyCashAddr);
	SetDmaCount(2,512);
	EnableDma(2);
	irq_enable();
}


//-------------------------------------------------------------------------------------------------------------------------------
//
//  从软盘上读取指定的逻辑块到缓冲区
//
//-------------------------------------------------------------------------------------------------------------------------------
void floppy_read(int blk,int secs,void* buf)
{
	int head;
	int track;
	int sector;
	int res;


	BlockToHTS(blk,&head,&track,&sector);


	while(1)
	{

	floppy_motoron();
	floppy_delay();                         // 作一些延迟
	__asm__("sti");

	//floppy_delay();    // 作一些延迟
	//if(!floppy_seek(0,track,head)) continue;
  //floppy_delay();    // 作一些延迟

	SetupFloppyDMA();
	floppy_delay();    // 作一些延迟

	floppy_setmode();
	floppy_delay();    // 作一些延迟
	floppy_sendbyte (FD_READ);              //send read command
	floppy_sendbyte (head*4 + 0);
	floppy_sendbyte (track);		             /*  Cylinder  */
	floppy_sendbyte (head);		             /*  Head  */
	floppy_sendbyte (sector);		           /*  Sector  */
	floppy_sendbyte (2);		                 /*  0=128, 1=256, 2=512, 3=1024, ...  */
	floppy_sendbyte (18);
	//floppy_sendbyte (sector+secs-1);	       /*  Last sector in track:here are  sectors count */
	floppy_sendbyte (0x1B);
	floppy_sendbyte (0xff);

	
	floppy_waitint();                       //直到发生了软驱中断


	res=floppy_result();
	floppy_delay();    // 作一些延迟

	if(ST1==0 && ST2 ==0)
	{
		 floppy_motoroff();
		 floppy_delay();    // 作一些延迟
		 memcpy(buf,(void*)FloppyCashAddr,secs*512);
		 return;
	}
		//misc_printf("  ST0 %d ST1 %d ST2 %d\n",ST0,ST1,ST2);
	}

  return;

}


//---------------------------------------------------------------------------------
//在vga模式下打印软盘信息
//
//--------------------------------------------------------------------------------
void getfdcinfo(void)
{

	unsigned int i;
	unsigned char CmType, FdType;

	floppy_sendbyte(0x10);
	i = floppy_readbyte(); 

	switch (i)
	{
	           case 0x80:	strcpy(FloppyIncName,"NEC765A controller"); break;
	           case 0x90:   strcpy(FloppyIncName,"NEC765B controller"); break;
	           default:     strcpy(FloppyIncName,"Enhanced controller"); break;
	}
	
	io_writebyte(0x70,0x10);
	CmType = io_readbyte(0x71);
	//CmType = io_readcmos(0x10);     //read floppy type from cmos
	FdType   = (CmType>>4) & 0x07;

	//vga_puts("\n");
	if ( FdType == 0 )  {
		//vga_puts("No FloppyDev Connect!");
		return;
	}

	switch( FdType )
	{
	case 0x02: // 1.2MB
		strcpy(FloppyDev.type,"1.2MB");
	break;

	case 0x04: // 1.44MB       标准软盘
		strcpy(FloppyDev.type,"1.44MB");
		break;

	case 0x05: // 2.88MB		没见过..-_-!
		strcpy(FloppyDev.type,"2.88MB");
		break;
	}

	
}
