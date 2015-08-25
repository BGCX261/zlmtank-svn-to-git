#ifndef _TANK_SERVER_H
#define _TANK_SERVER_H

#define USER_STATE_NOMARL  0
#define USER_STATE_BATTLE_READY  1

class user
{
public:
	DWORD userid;
    SESSION session;
	CHAR username[USER_NAME_LEN];
	DWORD roomid;
	DWORD state;
	user()
    {
        userid = 0;
        session = 0;
		memset(username,0, sizeof(username));
		roomid = 0;
		state = USER_STATE_NOMARL;
	}
};

#define ROOM_MAX_USER  8
#define ROOM_STATE_WAITING  0
#define ROOM_STATE_BATTLING 1


class room
{
public:
	DWORD roomid;
	DWORD masterid;
	DWORD max_user;
	DWORD state;
	std::map<DWORD,user*> user_map;
	room()
    {
        roomid = 0;
		masterid = 0;
		max_user = ROOM_MAX_USER;
		state = ROOM_STATE_WAITING;
	}

	BOOL AddUser(user *);
	BOOL DelUser(user *);
	DWORD GetUserCount();
};

typedef std::map<SESSION,user*>::iterator USER_ITR;
typedef std::map<SESSION,room*>::iterator ROOM_ITR;

class tank : public ue
{
public:
    std::map<SESSION,user*> user_online_map;
	std::map<DWORD,room*> room_map;
	DWORD m_userid;
	DWORD m_roomid;

    virtual BOOL Init();
	virtual BOOL deInit();
	virtual BOOL OnException(SESSION);
    virtual BOOL OnRecieve(SESSION,void*,DWORD);

	BOOL FindUserBySession(SESSION, user **);
	BOOL FindRoomBySession(SESSION, room **);
	DWORD GenUserId();
	DWORD GenRoomId();

    BOOL SendMsg(SESSION,void *,DWORD);
	BOOL AckResult(SESSION, DWORD, DWORD, void *, DWORD);
	BOOL OnLogin(SESSION,void*,DWORD);
	BOOL OnLogout(SESSION,void*,DWORD);
	BOOL OnGetRoomList(SESSION,void*,DWORD);
	BOOL OnJoinRoom(SESSION,void*,DWORD);
	BOOL OnExitRoom(SESSION,void*,DWORD);
	BOOL OnGetReady(SESSION,void*,DWORD);
	BOOL OnStartBattle(SESSION,void*,DWORD);
	BOOL OnCreateRoom(SESSION,void*,DWORD);
	BOOL OnSendTextMsg(SESSION,void*,DWORD);
    BOOL OnRecvTextMsg(SESSION,void*,DWORD);
	BOOL OnGetPlayerList(SESSION,void*,DWORD);
	BOOL OnFire(SESSION,void*,DWORD);

};


#endif
