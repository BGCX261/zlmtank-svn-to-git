#include "def.h"

#include "objtank.h"

objtank::objtank()
{
    
}

BOOL objtank::Init()
{
    UpdateGun();
    return true;
}

void objtank::DeInit()
{

}

void objtank::SetXY(float x, float y)
{

}

void objtank::UpdateGun()
{
    m_info.m_gunupdate = true;
}

void objtank::DrawGun()
{
	//draw gun
	FPOINT v_gun_end,v_gun_end_scr;
 	tank2 *ptank = (tank2 *)Game::Instance()->GetUE();
    FLOAT r;
		
    if (m_info.m_gunupdate)
    {
        r = m_info.m_gunangle * M_PI / 180.0f;
		v_gun_end.x = m_info.m_gunlen * cosf(r) + x + m_info.m_w/2;
		v_gun_end.y = m_info.m_gunlen * sinf(r) + y + m_info.m_h/2;
        ptank->WorldToScreen(v_gun_end,v_gun_end_scr);
		m_info.m_gunv2 = v_gun_end_scr;
		m_info.m_gunv1.x = sx + m_info.m_w/2;
		m_info.m_gunv1.y = sy + m_info.m_h/2;
	}

	m_hge->Gfx_RenderLine(m_info.m_gunv1.x,
		                  m_info.m_gunv1.y,
		                  m_info.m_gunv2.x,
		                  m_info.m_gunv2.y,
		                  0xffff0000);
	m_info.m_gunupdate = false;
}

void objtank::Fire()
{
    tank2 *ptank2 = (tank2 *)Game::Instance()->GetUE();
	FPOINT v1,v2;
    v1.Set(m_info.m_gunv2.x, m_info.m_gunv2.y);

    ptank2->ScreenToWorld(v1,v2);
    m_bullet.Set(m_info.m_attr.m_type,
                 m_info.m_power,
                 v2,
                 m_info.m_gunangle,
                 0);
	m_bullet.SetEnable(true);
}


bool objtank::FrameFunc(float dt)
{
    m_bullet.FrameFunc(dt);


    
    return false;
}

BOOL objtank::RenderFunc(float dt)
{
    //draw a tank
    hgeQuad quad;
	memset(&quad,0,sizeof(quad));
	quad.v[0].col = 0xffffffff;
	quad.v[0].x = sx;
	quad.v[0].y = sy;
    quad.v[1].col = 0xffffffff;
	quad.v[1].x = sx + m_info.m_w;
	quad.v[1].y = sy;
    quad.v[2].col = 0xffffffff;
	quad.v[2].x = sx + m_info.m_w;
	quad.v[2].y = sy + m_info.m_h;
	quad.v[3].col = 0xffffffff;
	quad.v[3].x = sx;
	quad.v[3].y = sy + m_info.m_h;

    m_hge->Gfx_RenderQuad(&quad);


	// draw gun
	DrawGun();
	
    m_bullet.RenderFunc(dt);

	return b2obj::RenderFunc(dt);
}

bool objtank::EventFunc(int, void *)
{
    return false;
}

mytank::mytank()
{
	m_powering = false;
}

bool mytank::FrameFunc(float dt)
{
	b2Vec2 Rot(0,0);
    HGE *hge = Game::Instance()->GetSysInterface()->GetHge();

    

	if (m_bullet.GetEnable())
	{
	    //
	}
	else
	{

        if (hge->Input_GetKeyState(HGEK_LEFT))
        {
            m_info.m_gunangle += 0.1f;
    		if (m_info.m_gunangle > 180)
    		{
                m_info.m_gunangle = 180;
    		}
    		else
    		{
                UpdateGun();
    		}
    
    	}
        if (hge->Input_GetKeyState(HGEK_RIGHT))
        {
            m_info.m_gunangle -= 0.1f;
    		if (m_info.m_gunangle < 0)
    		{
                m_info.m_gunangle = 0;
    		}
    		else
    		{
                UpdateGun();
    		}
    
    	}
        if (hge->Input_GetKeyState(HGEK_UP))
        {
            if (m_info.m_gunangle <90.0f)
            {
                m_info.m_gunangle += 0.1f;
        		if (m_info.m_gunangle > 90.0f)
        		{
                    m_info.m_gunangle = 90.0f;
        		}
        		else
        		{
                    UpdateGun();
        		}
            }
            if (m_info.m_gunangle >90.0f)
            {
                m_info.m_gunangle -= 0.1f;
        		if (m_info.m_gunangle < 90.0f)
        		{
                    m_info.m_gunangle = 90.0f;
        		}
        		else
        		{
                    UpdateGun();
        		}
            }

    	}

        if (hge->Input_GetKeyState(HGEK_DOWN))
        {
            if (m_info.m_gunangle <90.0f)
            {
                m_info.m_gunangle -= 0.1f;
        		if (m_info.m_gunangle < 0.0f)
        		{
                    m_info.m_gunangle = 0.0f;
        		}
        		else
        		{
                    UpdateGun();
        		}
            }
            if (m_info.m_gunangle >90.0f)
            {
                m_info.m_gunangle += 0.1f;
        		if (m_info.m_gunangle > 180.0f)
        		{
                    m_info.m_gunangle = 180.0f;
        		}
        		else
        		{
                    UpdateGun();
        		}
            }

    	}


        if (!m_powering)
        {
			m_info.m_power = 0;
		}
	
        if (hge->Input_GetKeyState(HGEK_SPACE))
        {

			m_info.m_power+= 0.05f;
			m_powering = true;
			if (m_info.m_power >= 100.0f)
			{
				m_info.m_power = 100.0f;
			}
    	}
		else
		{
            if (m_powering)
            {
                Fire();
				m_powering = false;
			}
		}
		

    
	}
    return objtank::FrameFunc(dt);;
}

bullet::bullet()
{
    SetEnable(false);
	m_time = 0;
	m_tank2 = (tank2 *)Game::Instance()->GetUE();
}

void bullet::SetEnable(BOOL b)
{
    m_enable = b;
}

BOOL bullet::GetEnable()
{
    return m_enable;
}

void bullet::Set(DWORD guntype,FLOAT power,FPOINT v1,FLOAT r,FLOAT wind)
{
    m_guntype = guntype;
	m_power = power;
	m_v1 = v1;
	m_r = r;
	m_wind = wind;
	m_time = 0;
}

bool bullet::FrameFunc(float dt)
{
    if (!GetEnable())
    {
        return false;
	}
    m_time +=dt;
	
    FLOAT arc = m_r*M_PI/180.0f;
	x = m_time*m_power*cosf(arc) + m_v1.x;
	y = m_time*m_power*sinf(arc) - m_time*m_time*4.9f + m_v1.y; //4.9=9.8/2
    FPOINT v1;
	v1.Set(x,y);

    // ´¦Àí²ÐÓ°
    if (m_time/0.5f == 0.0f)
    m_v1_onscr3 = m_v1_onscr2;
    m_v1_onscr2 = m_v1_onscr1;
    m_v1_onscr1 = m_v1_onscr;

	m_tank2->WorldToScreen(v1,m_v1_onscr);

	if (m_v1_onscr.x <0 || m_v1_onscr.x > m_tank2->m_state.m_ScreenW ||
		m_v1_onscr.y <0 || m_v1_onscr.y > m_tank2->m_state.m_ScreenH)
	{
	    // out of screen
        SetEnable(false);
	}

    if (ContactCheck())
    {
	    // Contact
        SetEnable(false);

    }

	
    return false;
}

bool bullet::RenderFunc(float dt)
{
    if (!GetEnable())
    {
        return false;
	}
	/*
	hgeTriple t;
	t.v[0].x = m_v1_onscr.x;
	t.v[0].y = m_v1_onscr.y;

	t.v[1].x = m_v1_onscr.x + 1.0f;
	t.v[1].y = m_v1_onscr.y;

	t.v[2].x = m_v1_onscr.x;
	t.v[2].y = m_v1_onscr.y + 1.0f;


	m_hge->Gfx_RenderTriple(&t);
       */


    //hgeFont *fnt = GETFONT(SYS_FONT_TYPE);

    //fnt->printf(float x,float y,int align,const char * format,...)
    DWORD color = 0xffffffff;
    DWORD color1 = 0xffff0000;
    DWORD color2 = 0xffff0000;
    DWORD color3 = 0xff800000;
    
    m_hge->Gfx_RenderLine(m_v1_onscr.x,m_v1_onscr.y,
                          m_v1_onscr.x+0.5f,m_v1_onscr.y,
                          color,
                          1.0f);
    m_hge->Gfx_RenderLine(m_v1_onscr1.x,m_v1_onscr1.y,
                          m_v1_onscr1.x+0.5f,m_v1_onscr1.y,
                          color1,
                          0.5f);
    m_hge->Gfx_RenderLine(m_v1_onscr2.x,m_v1_onscr2.y,
                          m_v1_onscr2.x+0.5f,m_v1_onscr2.y,
                          color2,
                          0.5f);
    m_hge->Gfx_RenderLine(m_v1_onscr3.x,m_v1_onscr3.y,
                          m_v1_onscr3.x+0.5f,m_v1_onscr3.y,
                          color3,
                          0.5f);



    return false;
}


BOOL bullet::ContactCheck()
{
    return (ContactGroundCheck() || ContactTankCheck(NULL));
}
BOOL bullet::ContactGroundCheck()
{
    BOOL bret = false;
    tank2 *ptank = (tank2 *)Game::Instance()->GetUE();
    DWORD n = x/(FLOAT)ptank->m_state.m_objunit;
    DWORD h = ptank->m_state.m_room.m_vmap[n];
    if (y <= h)
    {
        bret = true;
    }

    return bret;
    
}
BOOL bullet::ContactTankCheck(objtank **contacttank)
{
    BOOL bret = false;
    tank2 *ptank = (tank2 *)Game::Instance()->GetUE();

    PLAYER2_ITR itr;
	itr = ptank->m_state.m_room.m_player_vec.begin();
    for (;itr != ptank->m_state.m_room.m_player_vec.end();
	     itr++)
	{
	    objtank *pobjtank = (objtank *)(itr->ptank);
        if (x > pobjtank->m_info.m_x && x < (pobjtank->m_info.m_x + pobjtank->m_info.m_w) &&
            y > pobjtank->m_info.m_y && y < (pobjtank->m_info.m_y + pobjtank->m_info.m_h))
        {
            if (contacttank != NULL && *contacttank != NULL)
            *contacttank = pobjtank;
            return true;
        }
    }

    return bret;

}

