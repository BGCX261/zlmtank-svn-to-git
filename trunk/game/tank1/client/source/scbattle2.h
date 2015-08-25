#ifndef _SCBATTLE2_H
#define _SCBATTLE2_H

#include "tank.h"

class scBattle2: public scene
{
public:
    tank2 *m_tank;
	HGE *m_hge;
	mytank *m_mytank;

	




    scBattle2(tank2*);

	
    BOOL init();
    void deinit();

	virtual BOOL Entry();

    virtual void PreRender(float d);
    virtual void PostUpdate(float d);



	BOOL InitBattle();
    BOOL CreateMap();
	BOOL CreateTank(player2 &);

    void DrawMap();
	void DrawUI();

	void MoveCamByMouse();
};

#endif

