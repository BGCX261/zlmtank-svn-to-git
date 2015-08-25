//#include "def.h"
#include "vobj.h"
#include "game.h"


vobj::vobj()
{
    m_hgeSprite = NULL;
	x = 0.0f;
	y = 0.0f;
	sx = 0.0f;
	sy = 0.0f;
	m_hge = GETHGE();
}

vobj::~vobj()
{
    if (m_hgeSprite)
    {
        delete m_hgeSprite;
		m_hgeSprite = NULL;
	}
	m_hge = NULL;
}

bool vobj::RenderFunc(FLOAT dt)
{
    if(m_hgeSprite)m_hgeSprite->Render(sx,sy);
	return false;
}

