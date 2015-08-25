#ifndef _TIME_H
#define _TIME_H

//-------------------------时间结构-------------------------//
typedef struct
{
	unsigned char sec,min,hour;
}Time;

//-------------------------日期结构-------------------------//
typedef struct
{
	int week,day,month,year;
}Date;


//---------------------------------------------------------------------------------------------------------------------------------------------
// 时钟驱动接口函数
//---------------------------------------------------------------------------------------------------------------------------------------------

//....
extern Time currentTime;

//时钟初始化函数
void timer_init(void);

//取得系统时间
void timer_getcmostime(Time* t);

void timer_getcmosdate(Date* d);

void timer_sleep(unsigned int n);

#endif
