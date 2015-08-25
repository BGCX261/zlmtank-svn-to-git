#ifndef _DBG_H
#define _DBG_H


#include <serial.h>
#include <stdlib.h>



#define DBG
//#define TRC


//---------------------debug message-----------------
#ifdef DBG

extern char misc_buf[1000];
#define DBGPORT COM1
#define DBG_ON	1
//#define DEBUGMSG(exp,arg)	((exp) ? SerialPrintf DBGPORT,arg,2 : (0))
void DEBUGMSG(char *format,...)
{
	int i;
	va_list args;

	va_start(args, format);
	i = vsnprintf(misc_buf, sizeof(misc_buf), format, args);
	va_end(args);

	SerialWriteString(DBGPORT, misc_buf);
	return;
}
#else
#define DEBUGMSG(S)
#endif


//----------------------trace message------------------------
#ifdef TRC
#define TRACE()	\
	SerialPrintf(DBGPORT, " trace:[0x%08x:%s]\n",__func__,__func__);	

#else
#define TRACE()
#endif

//--------------------dbg module
#define DBG_VESA	1
#define DBG_HEAP	1



#endif //endfile

