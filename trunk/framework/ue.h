#ifndef _UE_H
#define _UE_H

class ue
{
public:
    virtual bool InitGame(){return TRUE;};
    virtual bool StartGame(){return TRUE;};
    virtual bool ShutdonwGame(){return TRUE;};
    virtual bool FreeGame(){return TRUE;};
};


#endif
