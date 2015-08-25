#include "def.h"
#include "WinSock.h"
#pragma comment(lib, "ws2_32.lib")
#include "process.h"
#include <include\box2d.h>
#include "objtank.h"


#include "sclogin.h"
#include "scRoom.h"
#include "scBattle.h"
#include "scBattle2.h"
#include "tank.h"


res_mapinfo::res_mapinfo()
{

}


res_mapinfo::~res_mapinfo()
{
    m_maps.clear();
}

BOOL res_mapinfo::LoadMapsDat()
{
    HGE *hge = GETHGE();
	CHAR mapdat[128] = {0};
    CHAR maplist[128] = {0};
	CHAR mapname[32] = {0};

	CHAR *p = NULL;
	CHAR *pbegin = NULL;
	CHAR *pend = NULL;
	BOOL bEnd = false;

	CHAR buf[128] = {0};
	DWORD ii = 0;

    strcpy(mapdat,RES);
	strcat(mapdat,RES_MAPS);
	
	hge->System_SetState(HGE_INIFILE, mapdat);
	p = hge->Ini_GetString(RES_MAPS_MAPS,RES_MAPS_MAPS_MAPLIST,RES_NULL_STR);
	if (!strcmp(RES_NULL_STR,p))
    {
        return false;
	}
	strncpy(maplist, p, 128);


	// example: maplist=map1,map2,map3,split by ','
    pbegin = maplist;
	for (pend = maplist; pend < maplist + sizeof(maplist) && !bEnd; pend++)
	{
		if (0 == *pend)
	    {
            bEnd = true;
		}

        if (RES_MAPS_MAPS_MAPLIST_SPLIT != *pend && !bEnd)
        {
            continue;
		}

        *pend = 0;

		HTEXTURE tex = 0;
		res_map rmap;
		p = hge->Ini_GetString(pbegin,RES_MAPS_ITEM_PIC,RES_NULL_STR);
		if (!strcmp(RES_NULL_STR,p))
		{
            return false;
		}
		
		memset(mapname,0,sizeof(mapname));
        strncat(mapname, RES, strlen(RES));
		strncat(mapname, p, 32);

		rmap.bg_pic = hge->Texture_Load(hge->Resource_MakePath(mapname));
		if (NULL == rmap.bg_pic)
		{
            return false;
		}

		rmap.w = hge->Ini_GetFloat(pbegin,RES_MAPS_ITEM_W,0.0f);
		rmap.h = hge->Ini_GetFloat(pbegin,RES_MAPS_ITEM_H,0.0f);
		rmap.b2w = hge->Ini_GetFloat(pbegin,RES_MAPS_ITEM_B2W,0.0f);
		rmap.b2h = hge->Ini_GetFloat(pbegin,RES_MAPS_ITEM_B2H,0.0f);

        rmap.playernum = hge->Ini_GetInt(pbegin,RES_MAPS_ITEM_PLAYERNUM,0);
		ASSERT(rmap.playernum <= 8);
		
		p = hge->Ini_GetString(pbegin,RES_MAPS_ITEM_B2STARTPOINT,RES_NULL_STR);
		if (!strcmp(RES_NULL_STR,p))
		{
            return false;
		}

		sscanf(p,"[%f,%f],[%f,%f],[%f,%f],[%f,%f],[%f,%f],[%f,%f],[%f,%f],[%f,%f]",
			&rmap.b2startpoint[0].x,
			&rmap.b2startpoint[0].y,
			&rmap.b2startpoint[1].x,
			&rmap.b2startpoint[1].y,
			&rmap.b2startpoint[2].x,
			&rmap.b2startpoint[2].y,
			&rmap.b2startpoint[3].x,
			&rmap.b2startpoint[3].y,
			&rmap.b2startpoint[4].x,
			&rmap.b2startpoint[4].y,
			&rmap.b2startpoint[5].x,
			&rmap.b2startpoint[5].y,
			&rmap.b2startpoint[6].x,
			&rmap.b2startpoint[6].y,
			&rmap.b2startpoint[7].x,
			&rmap.b2startpoint[8].y);


		m_maps[ii++]=rmap;
        
		pbegin = pend+1;
	}
	return true;
}

BOOL res_mapinfo::GetMap(DWORD idx, res_map& map)
{
    map = m_maps[idx-1];
    return true;
}

res_tankinfo::res_tankinfo()
{

}

res_tankinfo::~res_tankinfo()
{
    m_tanks.clear();
}


BOOL res_tankinfo::LoadTanksDat()
{
	HGE *hge = GETHGE();
	CHAR tankdat[128] = {0};
	CHAR tanklist[128] = {0};
	CHAR tankname[32] = {0};

	CHAR *p = NULL;
	CHAR *pbegin = NULL;
	CHAR *pend = NULL;
	BOOL bEnd = false;

	CHAR buf[128] = {0};
	DWORD ii = 0;

	strcpy(tankdat,RES);
	strcat(tankdat,RES_TANKS);
	
	hge->System_SetState(HGE_INIFILE, tankdat);
	p = hge->Ini_GetString(RES_TANKS_TANKS,
		                   RES_TANKS_TANKS_TANKLIST,
		                   RES_NULL_STR);
	if (!strcmp(RES_NULL_STR,p))
	{
		return false;
	}
	strncpy(tanklist, p, 128);


	// example: tanklist=tank1,tank2...,split by ','
	pbegin = tanklist;
	for (pend = tanklist; pend < tanklist + sizeof(tanklist) && !bEnd; pend++)
	{
		if (0 == *pend)
		{
			bEnd = true;
		}

		if (RES_TANKS_TANKS_TANKLIST_SPLIT != *pend && !bEnd)
		{
			continue;
		}

		*pend = 0;

		HTEXTURE tex = 0;
		res_tank rtank;
		p = hge->Ini_GetString(pbegin,RES_TANKS_ITEM_PIC,RES_NULL_STR);
		if (!strcmp(RES_NULL_STR,p))
		{
			return false;
		}

        memset(tankname,0,sizeof(tankname));
		strncat(tankname, RES, strlen(RES));
		strncat(tankname, p, 32);

		rtank.bg_pic = hge->Texture_Load(hge->Resource_MakePath(tankname));
		if (NULL == rtank.bg_pic)
		{
			return false;
		}

		rtank.w = hge->Ini_GetFloat(pbegin,RES_TANKS_ITEM_W,0.0f);
		rtank.h = hge->Ini_GetFloat(pbegin,RES_TANKS_ITEM_H,0.0f);
		rtank.b2w = hge->Ini_GetFloat(pbegin,RES_TANKS_ITEM_B2W,0.0f);
		rtank.b2h = hge->Ini_GetFloat(pbegin,RES_TANKS_ITEM_B2H,0.0f);
		
		m_tanks[ii++]=rtank;
		
		pbegin = pend+1;
	}
	return true;
}


BOOL res_tankinfo::GetTank(DWORD idx, res_tank& tank)
{
    tank = m_tanks[idx-1];
    return true;

}


tank::~tank()
{

}

bool tank::InitGame()
{
    BOOL bret = true;
    m_hge = Game::Instance()->GetSysInterface()->GetHge();
    m_hge->System_Log("tank::InitGame()\n");

    // load resource
    bret =LoadResource();
    if (!bret)return false;


	
    scLogin *plogin = new scLogin;
    bret = plogin->init();
    if (!bret)return false;
    Game::Instance()->InsertScene(plogin);

    //scRoom *proom = new scRoom(this);
    //bret = proom->init();
    //if (!bret)return false;
    //Game::Instance()->InsertScene(proom);

    scBattle *pbattel = new scBattle(this);
    bret = pbattel->init();
    if (!bret)return false;
    Game::Instance()->InsertScene(pbattel);

    return bret;
}

bool tank::StartGame()
{
    return Game::Instance()->GotoFirstScene();
}

BOOL tank::LoadResource()
{
    BOOL bret = m_maps.LoadMapsDat();
	if (!bret)return false;

	bret = m_tanks.LoadTanksDat();
	if (!bret)return false;

    return bret;
}

void tank::SetCamerab2XY(b2Vec2 &v)
{
    if (v.x < m_cam_range.x && v.y < m_cam_range.y &&
		v.x >= 0 && v.y >=0)
    m_camxy = v;
}

void tank::GetCamerab2XY(b2Vec2 &v)
{
    v = m_camxy;
}

void tank::GetB2XYinCamera(b2Vec2 &from,b2Vec2 &to)
{
    to.x = from.x - m_camxy.x;// + m_cam_rect.x/2.0f;
	to.y = from.y - m_camxy.y;// + m_cam_rect.y/2.0f;
}

void tank::B2ToScreen(b2Vec2 &v,FLOAT &x,FLOAT& y)
{
    FLOAT ratio = 64.0f/10.0f;
	x = (v.x + m_cam_rect.x/2.0f) *ratio;
	y = fabs(v.y - m_cam_rect.y/2.0f) *ratio;
	
}



#define TANK2

tank2::tank2()
{
    m_net_thread_id = 0;
    m_recv_num = 0;
    m_send_num = 0;
    m_error_num = 0;
}

tank2::~tank2()
{

}

bool tank2::InitGame()
{
    BOOL bret = true;
    m_hge = Game::Instance()->GetSysInterface()->GetHge();
    m_hge->System_Log("tank2::InitGame()\n");
    
    hgeFont *fnt=new hgeFont(m_hge->Resource_MakePath("res\\ComicSansMs2.fnt"));
    m_hge->System_GetErrorMessage();
    Game::Instance()->GetSysInterface()->SetFont(TANK_FONT_1,fnt);

    // load resource
    bret =LoadResource();
    if (!bret)return false;





	
    scLogin *plogin = new scLogin;
    bret = plogin->init();
    if (!bret)return false;
    Game::Instance()->InsertScene(plogin);

    scRoom *proom = new scRoom(this);
    bret = proom->init();
    if (!bret)return false;
    Game::Instance()->InsertScene(proom);

    scBattle2 *pbattel = new scBattle2(this);
    bret = pbattel->init();
    if (!bret)return false;
    Game::Instance()->InsertScene(pbattel);




    return bret;
}

bool tank2::StartGame()
{
    BOOL bret = true;
    
    bret = StartNetWork();
    if (!bret)return false;

    return Game::Instance()->GotoFirstScene();
}
bool tank2::ShutdonwGame()
{
    StopNetWork();
    
    return true;
}
bool tank2::FreeGame()
{
    return true;
}

BOOL tank2::LoadResource()
{
    BOOL bret = true;

    return bret;
}

void tank2::SetCamera(FPOINT &v)
{
    if (v.x < m_cam_range.x && v.y < m_cam_range.y &&
		v.x >= 0 && v.y >=0)
    m_camxy = v;
}

void tank2::GetCamera(FPOINT &v)
{
    v = m_camxy;
}

void tank2::WorldToScreen(const FPOINT &from, FPOINT &to)
{
    to.x = from.x - m_camxy.x;
	to.y = m_state.m_ScreenH - (from.y - m_camxy.y);
}

void tank2::ScreenToWorld(const FPOINT &from, FPOINT &to)
{
    to.x = from.x + m_camxy.x;
    to.y = m_state.m_ScreenH - from.y + m_camxy.y;
}

BOOL tank2::StartNetWork()
{
    DWORD rc = 0;
    BOOL bret = false;
    DWORD ul = 1;
    //初始化临界锁
    InitializeCriticalSection(&m_cs);

    rc = WSAStartup(MAKEWORD(2,2), &(m_state.ws));
    if (rc)
    {
        goto EXIT;
    }

    m_state.ClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (INVALID_SOCKET == m_state.ClientSocket)
    {
        goto EXIT;
    }
    
    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET; 
    saddr.sin_port = htons(m_state.ServerPort); 
    saddr.sin_addr.s_addr = htonl(m_state.ServerAddr); 
    rc = connect(m_state.ClientSocket, (sockaddr *)&saddr, sizeof(saddr));
    if (rc != 0)
    {
        goto EXIT;
    }
    ioctlsocket(m_state.ClientSocket,FIONBIO,&ul);



    // 启动网络线程
    m_net_thread_stacksize = 512;
    m_net_thread_state = THREAD_RUNNING;
    m_net_thread_id = _beginthread(_net_thread,m_net_thread_stacksize,this);
    if (0 == m_net_thread_id)
    {
        goto EXIT;
    }

    bret = true;
EXIT:
    return bret;
}

BOOL tank2::StopNetWork()
{
    m_net_thread_state = THREAD_STOP;
    closesocket(m_state.ClientSocket);
    m_state.ClientSocket = INVALID_SOCKET;
    DeleteCriticalSection(&m_cs);

	return true;
}

DWORD tank2::GetNetWorkState()
{
    fd_set rfd,wfd,efd;
    DWORD ul_rc = 0;
    if (INVALID_SOCKET == m_state.ClientSocket)
    {
        return NETWORK_FLAG_EXC;
    }

    FD_ZERO(&rfd);
    FD_ZERO(&wfd);
    FD_ZERO(&efd);
    FD_SET(m_state.ClientSocket,&rfd);
    FD_SET(m_state.ClientSocket,&wfd);
    FD_SET(m_state.ClientSocket,&efd);

    ul_rc = select(0,&rfd,&wfd,&efd,&m_state.timeout);
    if (SOCKET_ERROR == ul_rc)
    {
        return NETWORK_FLAG_EXC;
    }
    else
    {
        ul_rc = 0;
        if (FD_ISSET(m_state.ClientSocket,&rfd))
        {
            ul_rc |= NETWORK_FLAG_READ;
        }
        if (FD_ISSET(m_state.ClientSocket,&wfd))
        {
            ul_rc |= NETWORK_FLAG_WRITE;
        }
        if (FD_ISSET(m_state.ClientSocket,&efd))
        {
            ul_rc |= NETWORK_FLAG_EXC;
        }
        return ul_rc;
    }
}
BOOL tank2::RecvMsg(void *buf,DWORD buf_len)
{
    BOOL bret = true;
    network_msg msg;
    EnterCriticalSection(&m_cs);
    NETWORK_MSG_ITR itr = m_recv_que.begin();
    if (itr != m_recv_que.end())
    {
        msg = *itr;
        m_recv_que.erase(itr);
    }
    else
    {
        bret = false;
    }
    LeaveCriticalSection(&m_cs);

    if (false == bret)return false;

    memcpy(buf, msg.buf, buf_len);
    free(msg.buf);
    return true;
}

BOOL tank2::SendMsg(void *buf,DWORD buf_len)
{
    ASSERT(buf != NULL);
    ASSERT(buf_len != 0);
    network_msg msg_buf;
    msg_buf.buf = malloc(buf_len);
    ASSERT(NULL != msg_buf.buf);
    memcpy(msg_buf.buf, buf, buf_len);
    msg_buf.buf_len = buf_len;
    EnterCriticalSection(&m_cs);
    m_send_que.push_back(msg_buf);
    LeaveCriticalSection(&m_cs);
    return true;

}


void tank2::_net_thread(void *para)
{
    tank2 *ptank = (tank2 *)para;
    DWORD networkstate = 0;
    DWORD sock_rc = 0;
    CHAR *buf = NULL;
    DWORD buf_len = 1024;



    buf = (CHAR *)malloc(sizeof(buf_len));
    while(1)
    {
        if (THREAD_PAUSE == ptank->m_net_thread_state)
        {
            Sleep(1000);
            continue;
        }
        if (THREAD_STOP == ptank->m_net_thread_state)
        {
            Sleep(1000);
            goto EXIT;
        }

        if (INVALID_SOCKET == ptank->m_state.ClientSocket)
        {
            goto EXIT;
        }

        Sleep(1000);
        networkstate = ptank->GetNetWorkState();
        if (networkstate & NETWORK_FLAG_EXC)
        {
            goto EXIT;
        }

        if (networkstate & NETWORK_FLAG_READ)
        {
            sock_rc = recv(ptank->m_state.ClientSocket,
                           (CHAR *)buf,
                           buf_len,
                           0);
            if (SOCKET_ERROR == sock_rc)
            {
                ptank->m_error_num++;
                continue;
            }
            ptank->m_recv_num++;
            ASSERT(0 != sock_rc);
            network_msg msg_buf;
            msg_buf.buf = malloc(sock_rc);
            msg_buf.buf_len = sock_rc;
			memcpy(msg_buf.buf, buf,sock_rc);
            EnterCriticalSection(&ptank->m_cs);
            ptank->m_recv_que.push_back(msg_buf);
            LeaveCriticalSection(&ptank->m_cs);
        }
        
        if (networkstate & NETWORK_FLAG_WRITE)
        {
            network_msg msg;
            BOOL need_send = false;
            EnterCriticalSection(&ptank->m_cs);
            NETWORK_MSG_ITR itr = ptank->m_send_que.begin();
            if (itr != ptank->m_send_que.end())
            {
                msg = *itr;
                ptank->m_send_que.erase(itr);
                need_send = true;
            }
            else
            {
                need_send = false;
            }
            LeaveCriticalSection(&ptank->m_cs);

            if (!need_send)
            {
                continue;
            }

            sock_rc = send(ptank->m_state.ClientSocket,
                           (CHAR *)msg.buf,
                           msg.buf_len,
                           0);
            if (SOCKET_ERROR == sock_rc)
            {
                ptank->m_error_num++;
                continue;
            }
            ptank->m_send_num++;

        }


    }
EXIT:
    if (buf)
    {
        free(buf);
        buf = NULL;
        buf_len = 0;
    }

}

void tank2::PrintDebugInfo()
{
    hgeFont *fnt = GETFONT(SYS_FONT_TYPE);
    FLOAT h = fnt->GetHeight();
    FLOAT x = 10;
    FLOAT y = 2*h;
    fnt->printf(x,y+=h,0," net_thread_status : %d", m_net_thread_state);
    fnt->printf(x,y+=h,0," recv : %d", m_recv_num);
    fnt->printf(x,y+=h,0," send : %d", m_send_num);
    fnt->printf(x,y+=h,0," error : %d", m_error_num);
}