#ifndef _SCROOM_H
#define _SCROOM_H

#include "tank.h"

class scRoom: public scene
{
public:
    tank2 *m_tank;
	scRoom(tank2*);
	
    BOOL init();
    void deinit();
    virtual void PostRender(float d);
	virtual BOOL Leave();
};

// window
class RoomWindow : public GUIAppWindow {

	GUIAppButton *join;
	GUIAppButton *create;
	GUIAppListBox *m_RoomList;

	hgeFont *font;
	int clickCnt;
public:
	enum {
		GUIAPP_ID_LISTBOX_ROOMLIST = 10, 
		GUIAPP_ID_BNT_JOIN = 20,
		GUIAPP_ID_BNT_CREATE = 40,
	};
	RoomWindow(GUIApp *parentApp, int _id, float x, float y,bool fixed ) ;

	virtual void OnEvent (int id);
};









#endif

