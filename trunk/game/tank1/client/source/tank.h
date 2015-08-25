#ifndef _TANK_H
#define _TANK_H

#define RES_NULL_STR "NULL"
#define RES "res\\"

#define RES_MAPS              "maps.dat"
#define RES_MAPS_MAPS         "maps"
#define RES_MAPS_MAPS_MAPLIST "maplist"
#define RES_MAPS_MAPS_MAPLIST_SPLIT ','
#define RES_MAPS_ITEM_NAME    "name"
#define RES_MAPS_ITEM_PIC     "pic"
#define RES_MAPS_ITEM_W       "w"
#define RES_MAPS_ITEM_H       "h"
#define RES_MAPS_ITEM_B2W     "b2w"
#define RES_MAPS_ITEM_B2H     "b2h"
#define RES_MAPS_ITEM_PLAYERNUM "playernum"
#define RES_MAPS_ITEM_B2STARTPOINT "b2startpoint"

/*
pic=
w=1280
h=1280
raw=
b2w=400
b2h=400

*/


#define RES_TANKS "tanks.dat"
#define RES_TANKS_TANKS         "tanks"
#define RES_TANKS_TANKS_TANKLIST "tanklist"
#define RES_TANKS_TANKS_TANKLIST_SPLIT ','
#define RES_TANKS_ITEM_NAME    "name"
#define RES_TANKS_ITEM_PIC     "pic"
#define RES_TANKS_ITEM_W       "w"
#define RES_TANKS_ITEM_H       "h"
#define RES_TANKS_ITEM_B2W     "b2w"
#define RES_TANKS_ITEM_B2H     "b2h"


#define TANK_FONT_1  1


#define RES_MAP_PLAYERNUM_MAX 8
class res_map
{
public:
    HTEXTURE bg_pic;
    FLOAT w;
    FLOAT h;
    FLOAT b2w;
    FLOAT b2h;
	DWORD playernum;
	b2Vec2 b2startpoint[RES_MAP_PLAYERNUM_MAX];
	
	res_map()
	{
        bg_pic = NULL;
		w = 0.0f;
		h = 0.0f;
		b2w = 0.0f;
		b2h = 0.0f;
		playernum = 0;
	};
};

class res_tank
{
public:
    HTEXTURE bg_pic;
    FLOAT w;
    FLOAT h;
    FLOAT b2w;
    FLOAT b2h;
	res_tank()
	{
        bg_pic = NULL;
		w = 0.0f;
		h = 0.0f;
		b2w = 0.0f;
		b2h = 0.0f;
	};

};

typedef std::map<DWORD,res_map>::iterator RES_MAP_ITR;

class res_mapinfo
{
public:
    std::map<DWORD,res_map> m_maps;
    res_mapinfo();
    ~res_mapinfo();
    BOOL LoadMapsDat();
    BOOL GetMap(DWORD, res_map&);
};

typedef std::map<DWORD,res_tank>::iterator RES_TANK_ITR;

class res_tankinfo
{
public:
    std::map<DWORD,res_tank> m_tanks;
    res_tankinfo();
    ~res_tankinfo();
    BOOL LoadTanksDat();
    BOOL GetTank(DWORD, res_tank&);

};


#define PLAYER_NAME_MAXLEN 32
#define PLAYER_SIDE_A  1
#define PLAYER_SIDE_B  2
class player
{
public:
	CHAR Name[PLAYER_NAME_MAXLEN];
	DWORD Id;      //用户id
	DWORD TankId; //坦克id
	DWORD Money;
	DWORD Level;
	DWORD side;
	b2Vec2 b2vec; //位子

	player()
	{
        memset(Name,0,PLAYER_NAME_MAXLEN);
		Id = 0;
		TankId = 0;
		Money = 0;
		Level = 0;
		b2vec.SetZero();
	}
};

class player2
{
public:
	CHAR Name[PLAYER_NAME_MAXLEN];
	DWORD Id;      //用户id
	DWORD TankId; //坦克id
	DWORD Money;
	DWORD Level;
	DWORD side;
	DWORD position;
	void *ptank;

	player2()
	{
        memset(Name,0,PLAYER_NAME_MAXLEN);
		Id = 0;
		TankId = 0;
		Money = 0;
		Level = 0;
		position = 0;
		ptank = NULL;
	}
};


typedef std::vector<player>::iterator PLAYER_ITR; 
typedef std::vector<player2>::iterator PLAYER2_ITR; 

#define ROOM_NAME_MAXLEN 32
class room
{
public:
	CHAR Name[ROOM_NAME_MAXLEN];
	DWORD StageId;
	DWORD MasterId;
	DWORD MyId;
	std::vector<player> m_player_vec;
    room()
    {
        memset(Name,0,ROOM_NAME_MAXLEN);
		StageId = 0;
		MasterId = 0;
		MyId = 0;
	};
};

enum
{
    MAP_TYPE_0 = 0,
	MAP_TYPE_1,
	MAP_TYPE_2,
	MAP_TYPE_3,
	MAP_TYPE_NUM,
};

class room2
{
public:
	CHAR Name[ROOM_NAME_MAXLEN];
	DWORD StageId;
	DWORD MasterId;
	DWORD MyId;
	DWORD MapUnits;
	DWORD MapType;
	std::vector<DWORD> m_vmap;

	std::vector<player2> m_player_vec;
    room2()
    {
        memset(Name,0,ROOM_NAME_MAXLEN);
		StageId = 0;
		MasterId = 0;
		MyId = 0;
		MapUnits = 0;
        MapType = 0;
	};
};

#define STATE_INIT     0
#define STATE_LOGIN    1
#define STATE_ROOM     2
#define STATE_BATTLE   3

class state
{
public:
	DWORD m_state;
	std::string m_username;
	std::string m_password;
	std::string m_serveraddr;
	std::string m_serverport;
	room m_room;

    state()
    {
        m_state = STATE_INIT;
	}

};

#define NETWORK_FLAG_READ    1
#define NETWORK_FLAG_WRITE   2
#define NETWORK_FLAG_EXC     4

class state2
{
public:
	DWORD m_state;
	std::string m_username;
	std::string m_password;
	DWORD ServerAddr;
	WORD ServerPort; 
	SOCKET ClientSocket;
	WSADATA ws;
	struct timeval timeout;
	DWORD NetWorkFlag;
	

	DWORD m_ScreenW;
	DWORD m_ScreenH;
	DWORD m_objunit;
	room2 m_room;
	DWORD MapTypes[MAP_TYPE_NUM];

    state2()
    {
        m_state = STATE_INIT;
		MapTypes[MAP_TYPE_0] = 0;
		MapTypes[MAP_TYPE_1] = 40;
		MapTypes[MAP_TYPE_2] = 100;
		MapTypes[MAP_TYPE_3] = 200;
		m_objunit = 20;
		m_ScreenH = 400;
		m_ScreenW = 800;
		ClientSocket = INVALID_SOCKET;
		NetWorkFlag = 0;
		ServerAddr = INADDR_LOOPBACK;
		ServerPort = 1400;
		timeout.tv_sec=1;
		timeout.tv_usec=0;

	}

};







class tank: public ue
{
public:
    HGE *m_hge;
    
    res_mapinfo m_maps;
	res_tankinfo m_tanks;

    state m_state;

	b2Vec2 m_camxy;
	b2Vec2 m_cam_range;
	b2Vec2 m_cam_rect;
	b2Vec2 m_screen_rect;

    tank()
	{
	    m_hge=NULL;
		m_camxy.SetZero();
		m_cam_range.Set(400.0f,400.0f);
		m_cam_rect.Set(800.0f/6.4f,600.0f/6.4f);
	};
	~tank();

	// call by game frame
    virtual bool InitGame();
    virtual bool StartGame();
    //virtual bool ShutdonwGame();
	//virtual bool FreeGame();

	BOOL LoadResource();

	void SetCamerab2XY(b2Vec2 &v);
	void GetCamerab2XY(b2Vec2 &v);

	
	void GetB2XYinCamera(b2Vec2 &from,b2Vec2 &to);
	
	void B2ToScreen(b2Vec2 &v,FLOAT &x,FLOAT& y);


};

class network_msg
{
public:
	void *buf;
	DWORD buf_len;
	network_msg()
    {
        buf = NULL;
		buf_len = 0;
	};
};

typedef std::list<network_msg>::iterator NETWORK_MSG_ITR;

class tank2: public ue
{
public:
    HGE *m_hge;

    state2 m_state;

	FPOINT m_camxy;
	FPOINT m_cam_range;
	uintptr_t m_net_thread_id;
	DWORD m_net_thread_stacksize;
	DWORD m_net_thread_state;
	std::list<network_msg> m_recv_que;
	std::list<network_msg> m_send_que;
	DWORD m_recv_num;
	DWORD m_send_num;
	DWORD m_error_num;
	CRITICAL_SECTION m_cs;
	enum{
        THREAD_INIT = 0,
		THREAD_RUNNING,
		THREAD_PAUSE,
		THREAD_STOP
	};

    tank2();
	~tank2();

	// call by game frame
    virtual bool InitGame();
    virtual bool StartGame();
    virtual bool ShutdonwGame();
	virtual bool FreeGame();

	BOOL LoadResource();

	void SetCamera(FPOINT&);
	void GetCamera(FPOINT&);

	void WorldToScreen(const FPOINT&, FPOINT&);
	void ScreenToWorld(const FPOINT&, FPOINT&);

    BOOL StartNetWork();
	BOOL StopNetWork();
	DWORD GetNetWorkState();
	BOOL RecvMsg(void *,DWORD);
	BOOL SendMsg(void *,DWORD);

    static void _net_thread(void *);
	void PrintDebugInfo();

};



#endif
