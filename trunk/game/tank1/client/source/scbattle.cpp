#include "def.h"


#include "tank.h"
#include "scbattle.h"
#include "objtank.h"


scBattle::scBattle(tank *t)
{
    m_tank = t;
	m_world = NULL;
}	


BOOL scBattle::init()
{
    BOOL bret = true;
    HGE *hge = Game::Instance()->GetSysInterface()->GetHge();
    //GUIApp *app = new GUIApp(hge);
/*
    m_world = new world;
	WorldDef worlddef;
	worlddef.BoundSide = true;
	worlddef.g = 10.0f;
	worlddef.h = 400.0f;
	worlddef.w = 400.0f;
	strcpy(worlddef.name, "test_world");
	bret = m_world->InitWorld(worlddef);
    if (!bret)    
    {
        delete m_world;
		m_world = NULL;
		return false;
	}
*/
    return bret;
}



void scBattle::deinit()
{
    if (m_world)
    {
        delete m_world;
		m_world = NULL;
	}
}

BOOL scBattle::Entry()
{
    return InitBattle();
}

void scBattle::PreRender(float d)
{
    b2obj *pobj = NULL;
    b2Vec2 vworld,vcam;
	b2Shape *sharp;
    FLOAT x,y;

	
    OBJ_LSIT_ITR itr = m_obj_list.begin();
	for (;itr != m_obj_list.end();
	      itr++)
	{
        pobj = (b2obj*)(*itr);
		vworld = pobj->m_b2body->GetPosition();

		m_tank->GetB2XYinCamera(vworld,vcam);
		m_tank->B2ToScreen(vcam,x,y);
		pobj->x = x-32;
		pobj->y = y-32;
    }
    HGE *hge = GETHGE();
    FLOAT x1=0,y1=0,x2=400,y2=400;
    b2Vec2 xy1,xy2,xy3,xy4;
	xy1.Set(0,0);
	m_tank->GetB2XYinCamera(xy1,xy3);
	m_tank->B2ToScreen(xy3,x1,y1);
	
	xy2.Set(400,400);
	m_tank->GetB2XYinCamera(xy2,xy4);
	m_tank->B2ToScreen(xy4,x2,y2);

	hge->Gfx_RenderLine(x1,y1,x1,y2);
    hge->Gfx_RenderLine(x1,y1,x2,y1);
}



void scBattle::PostUpdate(float d)
{
    // update world
    if (NULL != m_world)
        m_world->Update(d);


    HGE *hge = Game::Instance()->GetSysInterface()->GetHge();
    b2Vec2 v;
	m_tank->GetCamerab2XY(v);


    MoveCamByMouse();

	
}

BOOL scBattle::InitBattle()
{
    BOOL bret = false;
    res_map cur_map;
	DWORD mapid;
	WorldDef worlddef;
	vobj *pobj = NULL;
	DWORD playernum = 0;
	PLAYER_ITR itr;
	b2Vec2 vcam;
	DWORD ii = 0;
	
    if (m_tank->m_state.m_state != STATE_BATTLE)
    {
        goto EXIT_LABEL;
	}

	// create b2world
	mapid = m_tank->m_state.m_room.StageId;
    bret = m_tank->m_maps.GetMap(mapid, cur_map);
	if (!bret)
	{
	    ASSERT(bret);
        goto EXIT_LABEL;
	}
	
    m_world = new world;

	worlddef.BoundSide = true;
	worlddef.g = -10.0f;
	worlddef.h = cur_map.b2h;
	worlddef.w = cur_map.b2w;
	strcpy(worlddef.name, "test_world");
	bret = m_world->InitWorld(worlddef);
    if (!bret)    
    {
        goto EXIT_LABEL;
	}

    //create world obj
    //pobj = new vobj;
	//pobj->m_hgeSprite = new hgeSprite(cur_map.bg_pic, 0, 0, cur_map.w, cur_map.h);
	//InsertObj(pobj);
	

    // create tanks
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

    vcam.Set(0.0f,0.0f);
    m_tank->SetCamerab2XY(vcam);

    bret = true;
EXIT_LABEL:
    if (!bret)
    {
        if (m_world)
        {
            delete m_world;
			m_world = NULL;
		}
	}

	return bret;
}

BOOL scBattle::CreateTank(player &p)
{
    BOOL bret = false;

	objtank *ptank = NULL;
	res_tank restank;
	ObjDef objdef;

    ptank = new objtank;

	
	bret = m_tank->m_tanks.GetTank(p.TankId,restank);
	if (!bret)
	{
        goto EXIT_LABEL;
	}
	ptank->m_hgeSprite = new hgeSprite(restank.bg_pic,
		                               0,
		                               0,
		                               restank.w,
		                               restank.h);

    objdef.x = p.b2vec.x;
	objdef.y = p.b2vec.y;
	objdef.w = restank.b2w;
	objdef.h = restank.b2h;
	bret = m_world->CreateObj(objdef,*ptank);
	if (!bret)
	{
        goto EXIT_LABEL;
	}

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

void scBattle::MoveCamByMouse()
{
    HGE *hge = GETHGE();
	FLOAT x,y;
	b2Vec2 v;
	hge->Input_GetMousePos(&x,&y);
	if (x < 30)
	{
	    m_tank->GetCamerab2XY(v);
		v.x-=0.2f;
        m_tank->SetCamerab2XY(v);
	}
	else if (x>770)
	{
	    m_tank->GetCamerab2XY(v);
		v.x+=0.2f;
        m_tank->SetCamerab2XY(v);
	}
	return;
}