#ifndef _OBJTANK_H
#define _OBJTANK_H

#define TANKACTION_NONE       0
#define TANKACTION_MOVE       1
#define TANKACTION_ATTACK    2
#define TANKACTION_USEITEM  3

#define DIR_LEFT   1
#define DIR_RIGHT 0

#include "tank.h"
class objtank;
class bullet :public vobj
{
public:
    BOOL m_enable;
    DWORD m_guntype;
	FLOAT m_power;
	FPOINT m_v1;
	FLOAT m_r;
	FLOAT m_wind;
	FLOAT m_time;
	
	tank2 *m_tank2;
	FPOINT m_v1_onscr;
	FPOINT m_v1_onscr1;
	FPOINT m_v1_onscr2;
	FPOINT m_v1_onscr3;


	
	bullet();

	
	void Set(DWORD guntype,FLOAT power,FPOINT v1,FLOAT r,FLOAT wind);
	void SetEnable(BOOL b);
	BOOL GetEnable();
	BOOL ContactCheck();
	BOOL ContactGroundCheck();
	BOOL ContactTankCheck(objtank **);
    virtual bool FrameFunc(float dt);
    virtual bool RenderFunc(float dt);
};


class tankattr
{
public:
    WORD m_type;
    WORD m_hp;
    WORD m_movment;
    WORD m_defect;
    WORD m_gunangle;
    WORD m_firack;
    WORD m_secack;
    tankattr()
    {
        m_type = 0;
        m_hp = 100;
        m_movment = 0;
        m_defect = 1;
        m_gunangle = 45;
        m_firack = 10;
        m_secack = 5;
    };
};

class tankinfo
{
public:
    DWORD m_id;
    FLOAT m_x;
    FLOAT m_y;
	FLOAT m_w;
	FLOAT m_h;
    WORD m_dir;
	
	FLOAT m_gunlen;
	FLOAT m_gunangle;
	FPOINT m_gunv1;
	FPOINT m_gunv2;
	BOOL m_gunupdate;
	FLOAT m_gunspeed;

	FLOAT m_power;
    WORD m_state;

    tankattr m_attr;
	player2 player;
    tankinfo()
    {
        m_id = 0;
        m_x = 0;
        m_y = 0;
		m_w = 0;
		m_h = 0;
		
		m_gunangle = 0.0f;
		m_gunlen = 20.0f;
        m_gunupdate = true;
		m_gunspeed = 0.1f;
        m_gunv1.SetZero();
		m_gunv2.SetZero();

		m_power = 5.0f;
		
        m_dir = DIR_LEFT;
    }
};



class tankaction
{
    WORD m_id;
    WORD m_actionid;
    
};



class objtank : public b2obj
{
public:
    tankinfo m_info;
	bullet m_bullet;
	BOOL m_powering;

public:
    objtank();
    ~objtank();
    bool Init();
    void DeInit();
    void SetXY(float x, float y);
	void UpdateGun();
    void DrawGun();
	void Fire();
	
    virtual bool FrameFunc(float dt);
    virtual bool RenderFunc(float dt);
    virtual bool EventFunc(int, void *);

};

class mytank : public objtank
{
public:
	mytank();
	virtual bool FrameFunc(float dt);
};




#endif
