#ifndef NTLDR_H
#define NTLDR_H
#include <types.h>
#include <syscfg.h>

//��ȡϵͳ������Ϣ
SYSTEMCONFIG *ntldr_get_syscfg();


//ntldr�����׼��Ϣ�ӿ�
#define NTLDR_PRINTF ntldr_printf
DWORD ntldr_printf(CHAR *format,...);


// ntldr�ĵ�����Ϣ����ӿ�,�����������Ļ�ʹ��ڻ����ļ�
#define NTLDR_DBG ntldr_dbg
DWORD ntldr_dbg(CHAR *format,...);


//ntldr����ϵͳ
#define NTLDR_HALT() ntldr_hlt(__FILE__, __LINE__)
void ntldr_hlt(CHAR *file,DWORD line);









#endif
