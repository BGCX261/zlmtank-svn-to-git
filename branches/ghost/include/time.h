#ifndef _TIME_H
#define _TIME_H

//-------------------------ʱ��ṹ-------------------------//
typedef struct
{
	unsigned char sec,min,hour;
}Time;

//-------------------------���ڽṹ-------------------------//
typedef struct
{
	int week,day,month,year;
}Date;


//---------------------------------------------------------------------------------------------------------------------------------------------
// ʱ�������ӿں���
//---------------------------------------------------------------------------------------------------------------------------------------------

//....
extern Time currentTime;

//ʱ�ӳ�ʼ������
void timer_init(void);

//ȡ��ϵͳʱ��
void timer_getcmostime(Time* t);

void timer_getcmosdate(Date* d);

void timer_sleep(unsigned int n);

#endif
