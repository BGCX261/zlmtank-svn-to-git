#ifndef _OBJ_H
#define _OBJ_H



class obj
{
public:
    virtual bool FrameFunc(float){return false;};
    virtual bool RenderFunc(float){return false;};
    virtual bool OnEvent(unsigned long, void *){return false;};
};

#endif
