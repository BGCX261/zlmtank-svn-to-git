#ifndef _DMA_H
#define _DMA_H

#define MAX_DMA_CHANNELS	8

/* 8237 DMA controllers */
#define IO_DMA1_BASE	0x00                   	  /* 8 bit slave DMA, channels 0..3 */
#define IO_DMA2_BASE	0xC0	                    /* 16 bit master DMA, ch 4(=slave input)..7 */

/* DMA controller registers */
#define DMA1_CMD_REG					0x08	                  /* command register (w) */
#define DMA1_STAT_REG					0x08                   	/* status register (r) */
#define DMA1_REQ_REG					0x09                   /* request register (w) */
#define DMA1_MASK_REG					0x0A	                   /* single-channel mask (w) */
#define DMA1_MODE_REG					0x0B	                   /* mode register (w) */
#define DMA1_CLEAR_FF_REG				0x0C	                   /* clear pointer flip-flop (w) */
#define DMA1_TEMP_REG					 0x0D                    /* Temporary Register (r) */
#define DMA1_RESET_REG					 0x0D	                    /* Master Clear (w) */
#define DMA1_CLR_MASK_REG				0x0E                     /* Clear Mask */
#define DMA1_MASK_ALL_REG				0x0F                      /* all-channels mask (w) */

#define DMA2_CMD_REG		             0xD0	/* command register (w) */
#define DMA2_STAT_REG		             0xD0	/* status register (r) */
#define DMA2_REQ_REG					0xD2    /* request register (w) */
#define DMA2_MASK_REG		            0xD4	/* single-channel mask (w) */
#define DMA2_MODE_REG		            0xD6	/* mode register (w) */
#define DMA2_CLEAR_FF_REG				0xD8	/* clear pointer flip-flop (w) */
#define DMA2_TEMP_REG					0xDA    /* Temporary Register (r) */
#define DMA2_RESET_REG		             0xDA	/* Master Clear (w) */
#define DMA2_CLR_MASK_REG				0xDC    /* Clear Mask */
#define DMA2_MASK_ALL_REG				0xDE    /* all-channels mask (w) */

#define DMA_ADDR_0              0x00    /* DMA address registers */
#define DMA_ADDR_1              0x02
#define DMA_ADDR_2              0x04
#define DMA_ADDR_3              0x06
#define DMA_ADDR_4              0xC0
#define DMA_ADDR_5              0xC4
#define DMA_ADDR_6              0xC8
#define DMA_ADDR_7              0xCC

#define DMA_CNT_0               0x01    /* DMA count registers */
#define DMA_CNT_1               0x03
#define DMA_CNT_2               0x05
#define DMA_CNT_3               0x07
#define DMA_CNT_4               0xC2
#define DMA_CNT_5               0xC6
#define DMA_CNT_6               0xCA
#define DMA_CNT_7               0xCE

#define DMA_PAGE_0              0x87    /* DMA page registers */
#define DMA_PAGE_1              0x83
#define DMA_PAGE_2              0x81
#define DMA_PAGE_3              0x82
#define DMA_PAGE_5              0x8B
#define DMA_PAGE_6              0x89
#define DMA_PAGE_7              0x8A

#define DMA_MODE_READ	0x44	/* I/O to memory, no autoinit, increment, single mode */
#define DMA_MODE_WRITE	0x48	/* memory to I/O, no autoinit, increment, single mode */
#define DMA_MODE_CASCADE 0xC0   /* pass thru DREQ->HRQ, DACK<-HLDA only */

//---------------------------------------------------------------------------------------------------------------------------------------------
// DMA驱动接口函数
//---------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------------
//
//  启用指定的DMA通道
//
//-------------------------------------------------------------------------------------------------------------------------------
static __inline__ void EnableDma(unsigned int dmanr)
{
	if (dmanr<=3)
		io_writebyte( DMA1_MASK_REG , dmanr);
	else
		io_writebyte( DMA2_MASK_REG , dmanr & 3);
}

//-------------------------------------------------------------------------------------------------------------------------------
//
//  禁用指定的DMA通道
//
//-------------------------------------------------------------------------------------------------------------------------------
static __inline__ void DisableDma(unsigned int dmanr)
{
	if (dmanr<=3)
		io_writebyte( DMA1_MASK_REG , dmanr | 4);
	else
		io_writebyte( DMA2_MASK_REG , (dmanr & 3) | 4);
}

//-------------------------------------------------------------------------------------------------------------------------------
//
//  清空DMA 晶体计数器
//
//-------------------------------------------------------------------------------------------------------------------------------
static __inline__ void ClearDmaFF(unsigned int dmanr)
{
	if (dmanr<=3)
		io_writebyte(DMA1_CLEAR_FF_REG , 0);
	else
		io_writebyte(DMA2_CLEAR_FF_REG, 0);
}

//-------------------------------------------------------------------------------------------------------------------------------
//
//  设定DMA 传输模式
//
//-------------------------------------------------------------------------------------------------------------------------------
static __inline__ void SetDmaMode(unsigned int dmanr, char mode)
{
	if (dmanr<=3)
		io_writebyte(mode | dmanr,  DMA1_MODE_REG);
	else
		io_writebyte(mode | (dmanr&3),  DMA2_MODE_REG);
}

//-------------------------------------------------------------------------------------------------------------------------------
//
//  设定DMA 页面寄存器
//
//-------------------------------------------------------------------------------------------------------------------------------
static __inline__ void SetDmaPage(unsigned int dmanr, char pagenr)
{
	switch(dmanr) {
		case 0:
			io_writebyte(DMA_PAGE_0 , pagenr);
			break;
		case 1:
			io_writebyte(DMA_PAGE_1 , pagenr);
			break;
		case 2:
			io_writebyte(DMA_PAGE_2 , pagenr);
			break;
		case 3:
			io_writebyte(DMA_PAGE_3 , pagenr);
			break;
		case 5:
			io_writebyte(DMA_PAGE_5 , pagenr & 0xfe);
			break;
		case 6:
			io_writebyte(DMA_PAGE_6 , pagenr & 0xfe);
			break;
		case 7:
			io_writebyte(DMA_PAGE_7 , pagenr & 0xfe);
			break;
	}
}


//-------------------------------------------------------------------------------------------------------------------------------
//
//  设定DMA 传输高速缓冲区地址
//
//-------------------------------------------------------------------------------------------------------------------------------
static __inline__ void SetDmaAddr(unsigned int dmanr, unsigned int a)
{
	SetDmaPage(dmanr, a>>16);
	if (dmanr <= 3)  {
	    io_writebyte(((dmanr&3)<<1) + IO_DMA1_BASE ,  a & 0xff );
        io_writebyte(((dmanr&3)<<1) + IO_DMA1_BASE , (a>>8) & 0xff );
	}  else  {
	    io_writebyte(((dmanr&3)<<2) + IO_DMA2_BASE ,(a>>1) & 0xff );
	    io_writebyte(((dmanr&3)<<2) + IO_DMA2_BASE , (a>>9) & 0xff);
	}
}


//-------------------------------------------------------------------------------------------------------------------------------
//
//  设定DMA 传输块数
//
//-------------------------------------------------------------------------------------------------------------------------------
static __inline__ void SetDmaCount(unsigned int dmanr, unsigned int count)
{
        count--;
	if (dmanr <= 3)  {
	    io_writebyte( ((dmanr&3)<<1) + 1 + IO_DMA1_BASE ,count & 0xff );
	    io_writebyte( ((dmanr&3)<<1) + 1 + IO_DMA1_BASE ,(count>>8) & 0xff );
        } else {
	    io_writebyte( ((dmanr&3)<<2) + 2 + IO_DMA2_BASE ,(count>>1) & 0xff);
	    io_writebyte( ((dmanr&3)<<2) + 2 + IO_DMA2_BASE ,(count>>9) & 0xff);
        }
}


//-------------------------------------------------------------------------------------------------------------------------------
//
//  获得DMA 传输剩余块数
//
//-------------------------------------------------------------------------------------------------------------------------------
static __inline__ int GetDmaResidue(unsigned int dmanr)
{
	unsigned int io_port = (dmanr<=3)? ((dmanr&3)<<1) + 1 + IO_DMA1_BASE
					 : ((dmanr&3)<<2) + 2 + IO_DMA2_BASE;

	/* using short to get 16-bit wrap around */
	unsigned short count;
	count = 1 + io_readbyte(io_port);
	count += io_readbyte(io_port) << 8;
	return (dmanr<=3)? count : (count<<1);
}

#endif 
