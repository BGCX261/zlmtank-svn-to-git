#ifndef _B2OBJ_H
#define _B2OBJ_H
#include "vobj.h"
#include <include\box2d.h>

#define B2OBJ_PARENT_CLASS vobj

class b2obj :public vobj
{
public:

	b2Body *m_b2body;
    b2obj();
	~b2obj();
	virtual bool RenderFunc(float);
};

#endif
