#ifndef _VOBJ_H
#define _VOBJ_H


#include "obj.h"
#include "hge.h"
#include "hgeSprite.h"

#define VOBJ_PARENT_CLASS obj

class vobj :public obj
{
public:
	FLOAT x;
	FLOAT y;
	FLOAT sx;
	FLOAT sy;
	HGE *m_hge;
	hgeSprite* m_hgeSprite;

    vobj();
	~vobj();

	virtual bool RenderFunc(FLOAT dt);
};

#endif

