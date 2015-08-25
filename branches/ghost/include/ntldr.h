#ifndef NTLDR_H
#define NTLDR_H
#include <types.h>
#include <syscfg.h>

//获取系统配置信息
SYSTEMCONFIG *ntldr_get_syscfg();


//ntldr输出标准信息接口
#define NTLDR_PRINTF ntldr_printf
DWORD ntldr_printf(CHAR *format,...);


// ntldr的调试信息输出接口,可以输出到屏幕和串口或者文件
#define NTLDR_DBG ntldr_dbg
DWORD ntldr_dbg(CHAR *format,...);


//ntldr挂起系统
#define NTLDR_HALT() ntldr_hlt(__FILE__, __LINE__)
void ntldr_hlt(CHAR *file,DWORD line);









#endif
