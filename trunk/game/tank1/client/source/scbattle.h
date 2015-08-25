#ifndef _SCBATTLE_H
#define _SCBATTLE_H
#include "tank.h"

class scBattle: public scene
{
public:
    tank *m_tank;

    world *m_world;

	


    scBattle(tank*);

	
    BOOL init();
    void deinit();

	virtual BOOL Entry();

    virtual void PreRender(float d);
    virtual void PostUpdate(float d);



	BOOL InitBattle();
	BOOL CreateTank(player &);

	void MoveCamByMouse();
};

#endif
