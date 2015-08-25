#include <types.h>
#include <io.h>
#include <irq.h>
#include <time.h>
#include <stdlib.h>
#include <ntldr.h>

//from linux
#define BCD_TO_BIN(val) ((val)=((val)&15)+((val)>>4)*10);

//ϵͳʱ�ӵ��ж�Ƶ��
#define HZ 1000
#define LATCH (1193182 / HZ)//ϣ����ʱ���ж�Ƶ�ʣ�ÿ����1000/1193182��,һ��1/600s
static unsigned long clock_tick=0;
static unsigned long ntick=0;

//������ϵͳ���õ�ʱ��\���ڽṹ
Time	CurrentTime;
Date	CurrentDate;
static int sec=0;

void timer_interrupt_handler(void);


void timer_init(void)
{
	io_writebyte(0x43 , 0x34);				//binary,mode 2,lsb/msb,ch 0
	io_writebyte(0x40 , LATCH & 0xff);		//lsb
	io_writebyte(0x40 , LATCH >> 8);		//msb

	
	irq_setirqfun(IRQ_TIME,timer_interrupt_handler);	//��װʱ���ж�
	//io_writebyte(0x21 , io_readbyte_p(0x21) &~0x01);	//����ʱ���ж�
	//irq_enableirq(IRQ_TIME);
	
	timer_getcmostime(&CurrentTime);
	timer_getcmosdate(&CurrentDate);


	clock_tick=0;

	//misc_printf("  The Timer Frequency is %d HZ\n",HZ);
	NTLDR_PRINTF("  SYSTEM start at %02d:%02d:%02d %04d-%02d-%02d\n",
		            CurrentTime.hour,CurrentTime.min,CurrentTime.sec,
					CurrentDate.year,CurrentDate.month,CurrentDate.day);
	//timer_sleep(2000);
		
	return;
}


//---------------------------------------------------------------------------------------------------------------------------
//
// ��� CMOS ʱ��
//
//---------------------------------------------------------------------------------------------------------------------------
void timer_getcmostime(Time* t)
{
	if (t) {
		t->sec =   io_readcmos(0);
		t->min =  io_readcmos(2);
		t->hour=  io_readcmos(4);
	}
	BCD_TO_BIN(t->sec);
	BCD_TO_BIN(t->min);
	BCD_TO_BIN(t->hour);
	return;
}


//-----------------------------------------------------------------------------------------------------------------------------
//
//��� CMOS ����
//
//-----------------------------------------------------------------------------------------------------------------------------
void timer_getcmosdate(Date* d)
{
	if (d) {
		d->week =io_readcmos(6);
		d->day =io_readcmos(7);
		d->month =io_readcmos(8);
		d->year = io_readcmos(9);
	}
	BCD_TO_BIN(d->day);
	BCD_TO_BIN(d->month);
	BCD_TO_BIN(d->year);
	d->year+=2000;
	return;
}



//----------------------------------------------------------------------------------------------------------------------------------------
//
// ʱ���жϾ��
//
//----------------------------------------------------------------------------------------------------------------------------------------
void timer_interrupt_handler(void)
{
	int x,y;
	if(clock_tick>=2^32-1)
	{
	clock_tick=0;
	}
	clock_tick++;
	ntick++;

	return;
}



//----------------------------------------------------------------------------------------------------------------------------------------
//
// ��ʱ���� n ms
//
//----------------------------------------------------------------------------------------------------------------------------------------
void timer_sleep(unsigned int n)
{

	irq_enable();
	ntick=0;	
	unsigned long tick_old,tick_new;
	tick_old=tick_new=ntick;
	
	while(tick_new - tick_old < n)
	{
		tick_new=ntick;
	}

	return;
}
