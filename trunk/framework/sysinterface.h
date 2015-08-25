#ifndef _SYSINTERFACE_H
#define _SYSINTERFACE_H

const DWORD MAX_FONT_TYPE = 3;
const DWORD SYS_FONT_TYPE = 0;

typedef hgeFont* PHGEFONT;

class sysinterface
{
public:
    HGE *m_hge;
    PHGEFONT m_pfnt[MAX_FONT_TYPE];

    sysinterface()
    {
        m_hge = NULL;
	memset(m_pfnt,0,sizeof(m_pfnt));
    };
    ~sysinterface(){};
    void SetHge(HGE *hge)
    	{m_hge = hge;}
    HGE *GetHge(){return m_hge;}
    void SetFont(DWORD type, hgeFont *fnt)
    {
        if (type >= MAX_FONT_TYPE){
          return;}
	m_pfnt[type]=fnt;};
    hgeFont *GetFont(DWORD type){
        if (type >= MAX_FONT_TYPE){
          return NULL;}
		  return m_pfnt[type];

	}
};

#endif
