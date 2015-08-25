#include <types.h>
#include <serial.h>
#include <io.h>
#include <stdlib.h>
#include <ntldr.h>

SERIALPORT gSerialPort[COMNUM] = {{SERIAL_NO_INSTALLED,COM1BASE,FALSE},
									{SERIAL_NO_INSTALLED,COM2BASE,FALSE},
									{SERIAL_NO_INSTALLED,COM3BASE,FALSE},
									{SERIAL_NO_INSTALLED,COM4BASE,FALSE}};

void SerialTest(void)
{
	U32 ret = 0;

	ret = SerialDetect(0x3f8);
	NTLDR_PRINTF("Serial:detect port = %x,ret = %x\n", 0x3f8, ret);


	ret = SerialDetect(0x2f8);
	NTLDR_PRINTF("Serial:detect port = %x,ret = %x\n", 0x2f8, ret);

	ret = SerialDetect(0x3e8);
	NTLDR_PRINTF("Serial:detect port = %x,ret = %x\n", 0x3e8, ret);

	ret = SerialDetect(0x2e8);
	NTLDR_PRINTF("Serial:detect port = %x,ret = %x\n", 0x2e8, ret);

	SerialWriteString(COM1,"SerialTestugiuggyugtftftftfyfkyufkyfygfu\n");
	return;
}

BOOL InitSerialPort(VOID)
{
	//TRACE();依赖串口初始化
	//简化的初始化了串口
	DWORD ret = 0;
	for (DWORD ii = 0; ii<COMNUM; ii++)
	{
		ret = SerialDetect(gSerialPort[ii].IoAddr);
		if (ret != SERIAL_NO_INSTALLED)
		{
			gSerialPort[ii].IsInited = TRUE;
			gSerialPort[ii].ICType = ret;
		}
	}
	return TRUE;
}

U32 SerialDetect(U16 BaseAddr)
{
	//返回o说明没有安装
	//返回1说明是8250
	//2:16450 or 8250 with scratch reg
	//3:16550
	//4:16550A

	//trace();


	U16 x;
	U16 olddata;

	//check if UART is present anyway
	olddata = io_readbyte(BaseAddr + 4);
	io_writebyte(BaseAddr + 4, 0x10);

	if (io_readbyte(BaseAddr + 6) & 0xf0)
		return SERIAL_NO_INSTALLED;

	io_writebyte(BaseAddr + 4, 0x1f);
	if ((io_readbyte(BaseAddr + 6) & 0xf0) != 0xf0)
		return SERIAL_NO_INSTALLED;

	io_writebyte(BaseAddr + 4, olddata);
	//next thing to do is look for scratch register
	olddata = io_readbyte(BaseAddr + 7);
	io_writebyte(BaseAddr + 7, 0x55);
	if (io_readbyte(BaseAddr +7) != 0x55)
		return SERIAL_CHIP_8250;
	io_writebyte(BaseAddr + 7, 0xAA);
	if (io_readbyte(BaseAddr +7) != 0xAA)
		return SERIAL_CHIP_8250;


	io_writebyte(BaseAddr + 7, olddata);//we dont need restore it if it's not there
	//then check if there's a FIFO
	io_writebyte(BaseAddr +2, 1);
	x = io_readbyte(BaseAddr +2);
	//some old-fashioned software relies on this

	io_writebyte(BaseAddr+2 , 0x0);
	if (x&0x80 == 0)
		return SERIAL_CHIP_16450_OR_8250;
	if (x&0x40 == 0)
		return SERIAL_CHIP_16550;
	return SERIAL_CHIP_16550A;
	
}


/////////////////////////////////////////////////////////////////////////////////
//
//
/////////////////////////////////////////////////////////////////////////////////
BOOL SerialSendByte(DWORD PortID, BYTE ch)
{
	DWORD timeout;

	timeout = 0x00FFFFL;

	// Wait for transmitter to clear
	while ((io_readbyte((WORD)(gSerialPort[PortID-1].IoAddr + LSR)) & XMTRDY) == 0)
	{
	 	if (!(--timeout))
		{
			return FALSE;
		}
	}
	io_writebyte((WORD)(gSerialPort[PortID-1].IoAddr + TXR), ch);

	return TRUE;
}





/////////////////////////////////////////////////////////////////////////////////
//
//
/////////////////////////////////////////////////////////////////////////////////
BOOL SerialWriteString(DWORD PortID, BYTE *s)
{
	if (s == NULL)
		return FALSE;
	if (gSerialPort[PortID - 1].IsInited == FALSE)
		return FALSE;
	
	while(*s != '\0')
	{
		SerialSendByte(PortID, *s);
		s++;
	}
	return TRUE;

}


