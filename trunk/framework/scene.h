#ifndef _SCENE_H
#define _SCENE_H

#include "obj.h"
#include "gui.h"
#include <list>

typedef std::list<obj *>::iterator OBJ_LSIT_ITR;
typedef std::list<GUIApp *>::iterator GUIAPP_LSIT_ITR;
class scene
{

public:
    std::list<obj *> m_obj_list;
    OBJ_LSIT_ITR m_obj_itr;

    std::list<GUIApp *> m_guiapp_list;
    GUIAPP_LSIT_ITR m_guiapp_itr;
    scene();
    ~scene();
    bool FrameFunc(float d);
    bool RenderFunc(float d);


    //scene mgr
    BOOL InsertObj(obj *);
    BOOL InsertGUIApp(GUIApp *);

    //interface
    virtual bool Entry(){return true;};
	virtual bool Leave(){return true;};
    virtual void PreUpdate(float d){return;};
    virtual void PostUpdate(float d){return;};
    virtual void PreRender(float d){return;};
    virtual void PostRender(float d){return;};
	
};


#endif
