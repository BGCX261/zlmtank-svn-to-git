//#include "def.h"
#include "scene.h"


scene::scene()
{
    m_obj_itr = m_obj_list.end();
    m_guiapp_itr = m_guiapp_list.end();
}

scene::~scene()
{
    m_obj_itr = m_obj_list.begin();
	for (; m_obj_itr != m_obj_list.end();)
	{
        delete *m_obj_itr;
		m_obj_list.erase(m_obj_itr++);
	}
    m_guiapp_itr = m_guiapp_list.begin();
	for (; m_guiapp_itr != m_guiapp_list.end();)
	{
        delete *m_guiapp_itr;
		m_guiapp_list.erase(m_guiapp_itr++);
	}
}

BOOL scene::InsertObj(obj *o)
{
    m_obj_list.push_back(o);
    return true;
}

BOOL scene::InsertGUIApp(GUIApp *a)
{
    m_guiapp_list.push_back(a);
    return true;
}

bool scene::FrameFunc(float d)
{
    PreUpdate(d);
    OBJ_LSIT_ITR itr = m_obj_list.begin();
    for (; itr != m_obj_list.end();itr++)
    {
        (*itr)->FrameFunc(d);
    }

    GUIAPP_LSIT_ITR itr_gui = m_guiapp_list.begin();
    for (; itr_gui != m_guiapp_list.end();itr_gui++)
    {
        (*itr_gui)->FrameFunc(d);
    }
    PostUpdate(d);
    return TRUE;
}

bool scene::RenderFunc(float d)
{
    PreRender(d);
    OBJ_LSIT_ITR itr = m_obj_list.begin();
    for (; itr != m_obj_list.end();itr++)
    {
        (*itr)->RenderFunc(d);
    }

    GUIAPP_LSIT_ITR itr_gui = m_guiapp_list.begin();
    for (; itr_gui != m_guiapp_list.end();itr_gui++)
    {
        (*itr_gui)->RenderFunc(d);
    }
    PostRender(d);
    return TRUE;
}
