#include "def.h"


#include "tank.h"
#include "objtank.h"
#include "scbattle2.h"



scBattle2::scBattle2(tank2 *t)
{
    m_tank = t;
	m_hge = NULL;
}	


BOOL scBattle2::init()
{
    BOOL bret = true;
    m_hge = GETHGE();
    //GUIApp *app = new GUIApp(hge);

	m_mytank = NULL;

    return bret;
}



void scBattle2::deinit()
{

}

BOOL scBattle2::Entry()
{
    return InitBattle();
}

void scBattle2::PreRender(float d)
{
    b2obj *pobj = NULL;
    b2Vec2 vworld,vcam;
	b2Shape *sharp;
    FLOAT x,y;

	
    DrawMap();
	DrawUI();
}



void scBattle2::PostUpdate(float d)
{




    MoveCamByMouse();

	
}

BOOL scBattle2::InitBattle()
{
    BOOL bret = false;
	DWORD playernum = 0;
	PLAYER2_ITR itr;
	FPOINT cam;

	
    if (m_tank->m_state.m_state != STATE_BATTLE)
    {
        goto EXIT_LABEL;
	}

	// genarate a amap
	bret = CreateMap();
	if (!bret)
	{
        goto EXIT_LABEL;
	}

    playernum = m_tank->m_state.m_room.m_player_vec.size();
	ASSERT(playernum >= 1);
	itr = m_tank->m_state.m_room.m_player_vec.begin();
    for (;itr != m_tank->m_state.m_room.m_player_vec.end();
	     itr++)
	{
        bret = CreateTank(*itr);
		if (!bret)
		{
            goto EXIT_LABEL;
		}
    }

    cam.Set(0,50);
    m_tank->SetCamera(cam);

    bret = true;
EXIT_LABEL:
    if (!bret)
    {

	}

	return bret;
}

BOOL scBattle2::CreateMap()
{
    BOOL bret = false;
    DWORD mapunits = m_tank->m_state.m_room.MapUnits;
	DWORD maptype = m_tank->m_state.m_room.MapType;
    DWORD ii = 0;
	DWORD high = 0;

	m_tank->m_state.m_room.m_vmap.clear();

    ASSERT(mapunits > m_tank->m_state.m_room.m_player_vec.size());
	ASSERT(maptype < MAP_TYPE_NUM);
	for (ii = 0; ii< mapunits; ii++)
	{
        high = m_hge->Random_Int(0,m_tank->m_state.MapTypes[maptype]);
		m_tank->m_state.m_room.m_vmap.push_back(high);
	}
    
    bret = true;
EXIT_LABEL:
	return bret;
}

BOOL scBattle2::CreateTank(player2 &p)
{
    BOOL bret = false;

	objtank *ptank = NULL;
	FPOINT v1,v2;

    if (m_tank->m_state.m_room.MyId == p.Id)
    {
        m_mytank = new mytank;
        ptank = m_mytank;
	}
    else
    {
        ptank = new objtank;
	}
	ptank->Init();

	ptank->x = p.position*m_tank->m_state.m_objunit;
	ptank->y = m_tank->m_state.m_room.m_vmap[p.position];
    v1.Set(ptank->x,ptank->y);
	m_tank->WorldToScreen(v1,v2);
	ptank->sx = v2.x;
	ptank->sy = v2.y - m_tank->m_state.m_objunit;

	ptank->m_info.m_x = ptank->x;
	ptank->m_info.m_y = ptank->y;
	ptank->m_info.m_w = m_tank->m_state.m_objunit;
	ptank->m_info.m_h = m_tank->m_state.m_objunit;
	

    p.ptank = ptank;

	InsertObj(ptank);

	bret = true;
EXIT_LABEL:
	if (!bret)
	{
        if (ptank->m_hgeSprite)
        {
            delete ptank->m_hgeSprite;
			ptank->m_hgeSprite = NULL;
		}

		//if (ptank->m_b2body)
	}
	
    return bret;
}

void scBattle2::MoveCamByMouse()
{
	FLOAT x,y;
	FPOINT v;
	m_hge->Input_GetMousePos(&x,&y);
	if (x < 30)
	{
	    m_tank->GetCamera(v);
		v.x-=0.2f;
        m_tank->SetCamera(v);
	}
	else if (x>770)
	{
	    m_tank->GetCamera(v);
		v.x+=0.2f;
        m_tank->SetCamera(v);
	}
	return;
}


void scBattle2::DrawMap()
{
    DWORD ii = 0;
	DWORD high;
	DWORD last_high = 0;
	FLOAT x1,y1,x2,y2;
	FPOINT p1,p2;
	for (ii = 0;ii< m_tank->m_state.m_room.m_vmap.size();ii++)
	{
        high = m_tank->m_state.m_room.m_vmap[ii];
		x1 = (FLOAT)(ii*m_tank->m_state.m_objunit);
		y1 = high;
        p1.Set(x1,y1);
		m_tank->WorldToScreen(p1,p2);
		x1 = p2.x;
		y1 = p2.y;

		
		x2 = x1 + m_tank->m_state.m_objunit;
		y2 = y1;
		
		m_hge->Gfx_RenderLine(x1,y1,x2,y2);
		m_hge->Gfx_RenderLine(x1,y1,x1,last_high);
		last_high = y2;
	}
}

void scBattle2::DrawUI()
{
    m_hge->Gfx_RenderLine(0,
		                  m_tank->m_state.m_ScreenH,
		                  m_tank->m_state.m_ScreenW,
		                  m_tank->m_state.m_ScreenH);

	hgeFont *fnt = GETSYSFONT();

	DWORD fontH = fnt->GetHeight();
	FLOAT x = 2;
	FLOAT y = m_tank->m_state.m_ScreenH + 2;
    fnt->printf(x,y,0," angle : %3.0f'", m_mytank->m_info.m_gunangle);
	y += fontH;
    fnt->printf(x,y,0," power : %3.0f'", m_mytank->m_info.m_power);


	// show other player infomation
	x = 200;
	y = m_tank->m_state.m_ScreenH + 2;
    DWORD playernum = m_tank->m_state.m_room.m_player_vec.size();
    PLAYER2_ITR itr = m_tank->m_state.m_room.m_player_vec.begin();
    for (;itr != m_tank->m_state.m_room.m_player_vec.end();
	     itr++)
	{
		objtank *p = (objtank *)itr->ptank;
        fnt->printf(x,y,0,"name : [%s]  hp : %d", itr->Name, p->m_info.m_attr.m_hp);
    	y += fontH;
    }
}