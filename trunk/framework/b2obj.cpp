#include "types.h"
#include "b2obj.h"

b2obj::b2obj()
{
    m_b2body = NULL;
}

b2obj::~b2obj()
{
    //if (m_b2body)
    //{
        //delete m_b2body;
		//m_b2body = NULL;
	//}
}

BOOL b2obj::RenderFunc(float dt)
{

	vobj::RenderFunc(dt);


    return false;
}
