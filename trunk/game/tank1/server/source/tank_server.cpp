#include "def.h"
#include "protocol.h"
#include "tank_server.h"

BOOL tank::Init()
{
    PRINTF(" tank::Init()\n");
    m_userid = 1;
    m_roomid = 1;

    return true;
}

BOOL tank::deInit()
{
    user *puser = NULL;

    PRINTF(" tank::deInit()\n");

    USER_ITR itr = user_online_map.begin();
    for (;itr != user_online_map.end();itr++)
    {
        puser = itr->second;
        delete puser;
        user_online_map.erase(itr);
    }

    return true;
}

BOOL tank::OnException(SESSION s)
{
    user *puser = NULL;
	room *proom = NULL;
    ROOM_ITR room_itr;
    PRINTF(" tank::OnException()\n");
    
    USER_ITR itr = user_online_map.find(s);
    if (itr != user_online_map.end())
    {
        PRINTF(" delete session : %x\n",s);
        puser = itr->second;

        
        if (puser->roomid != 0)
        {
             room_itr = room_map.find(puser->roomid);
             if (room_itr != room_map.end())
             {
				 proom = room_itr->second;
				 proom->DelUser(puser);
             }
        }
        

        delete puser;
        user_online_map.erase(itr);
    }
    
    return true;
}

BOOL tank::OnRecieve(SESSION s,void* buf,DWORD buf_len)
{
    ASSERT(NULL != buf);
    ASSERT(buf_len);
    
    MSG_HEAD *phead = (MSG_HEAD *)buf;
    PRINTF(" tank::OnRecieve()\n");
    switch (phead->msgtype)
    {
        case MSG_C_LOGIN:
            return OnLogin(s,buf,buf_len);
        case MSG_C_LOGOUT:
            return OnLogout(s,buf,buf_len);
        case MSG_C_GETROOMLIST:
            return OnGetRoomList(s,buf,buf_len);
        case MSG_C_CREATEROOM:
            return OnCreateRoom(s,buf,buf_len);
        case MSG_C_GETREADY:
            return OnGetReady(s,buf,buf_len);
        case MSG_C_GETPLAYERLSIT:
            return OnGetPlayerList(s,buf,buf_len);
        case MSG_C_JOINROOM:
            return OnJoinRoom(s,buf,buf_len);
        case MSG_C_STARTBATTLE:
            return OnStartBattle(s,buf,buf_len);
        case MSG_C_EXITROOM:
            return OnExitRoom(s,buf,buf_len);
        case MSG_C_SHOOT:
            return OnFire(s,buf,buf_len);

        default:
            PRINTF(" receive unknown msgtype = %x\n", phead->msgtype);
            return true;
    }
    
    return true;
}

BOOL tank::FindUserBySession(SESSION s, user **puser)
{
    USER_ITR itr = user_online_map.find(s);
    if (itr != user_online_map.end())
    {
		*puser = itr->second;
        return true;
    }
    return false;

}

BOOL tank::FindRoomBySession(SESSION s,room **proom)
{
    user *puser = NULL;
    ROOM_ITR itr;
    if (FindUserBySession(s, &puser))
    {
        ASSERT(puser);
        itr = room_map.find(puser->roomid);
        if (itr != room_map.end())
        {
			*proom = itr->second;
            return true;
        }
        else
        {
            return false;
        }
    }
    return false;
}

DWORD tank::GenUserId()
{
    m_userid++;
    if (0 == m_userid)
    {
        m_userid = 1;
    }
    return m_userid;
}

DWORD tank::GenRoomId()
{
    m_roomid++;
    if (0 == m_roomid)
    {
        m_roomid = 1;
    }
    return m_roomid;
}


BOOL tank::SendMsg(SESSION s,void *buf,DWORD buf_len)
{
    return ::SendMsg(s,buf,buf_len);
}

BOOL tank::AckResult(SESSION s, DWORD c, DWORD r, void* buf = NULL, DWORD buf_len = 0)
{
    BOOL bret = false;
    DWORD packet_buf_len = 0;
    packet_buf_len = sizeof(MSG_HEAD) + sizeof(MSG_ACK_HEAD) + buf_len;
    MSG_HEAD *phead = (MSG_HEAD*)new BYTE[packet_buf_len];
    MSG_ACK_HEAD *pack = (MSG_ACK_HEAD *)(phead + 1);
    ASSERT(NULL != phead);

    phead->msgtype = c;
    phead->msgack = MSG_ACK;
    phead->msglen = sizeof(MSG_ACK_HEAD) + buf_len;
    pack->result = r;

    if (buf != 0 && buf_len != 0)
    {
        BYTE *pbuf = (BYTE *)(pack + 1);
        memcpy(pbuf, buf, buf_len);
    }

    PRINTF(" tank::AckResult()\n");
    PRINTF(" session = %x, msgtype = %x, result = %x append buf_len = %d\n", s, c, r, buf_len);
    bret = SendMsg(s,phead,packet_buf_len);
    delete phead;
    return bret;
}

BOOL tank::OnLogin(SESSION s,void *buf,DWORD buf_len)
{
    MSG_HEAD *phead = NULL;
    MSG_LOGIN *msg_login = NULL;
    user *puser = NULL;
    DWORD ul_rc = 0;
    PRINTF(" tank::OnLogin()\n");
    ASSERT(NULL != buf);
    ASSERT(buf_len);
    phead = (MSG_HEAD *)buf;
    phead++;
    msg_login = (MSG_LOGIN *)phead;
    //校验用户名密码,暂时不搞

    //
    if (0 == msg_login->username[0] ||
        ' ' == msg_login->username[0])
    {
        PRINTF(" user : [%s], session = %x username invalid !\n",msg_login->username,s);
        AckResult(s,MSG_C_LOGIN,MSG_ACK_USERNAME_OR_PASSWORD_INVALID);
    }
    
    //重复登录
    USER_ITR itr = user_online_map.begin();
    for (;itr != user_online_map.end();itr++)
    {
        puser = itr->second;
        ul_rc = strncmp(puser->username,
                        msg_login->username,
                        sizeof(puser->username));
        if (0 == ul_rc)
        {
            ASSERT(puser->session == s);
            PRINTF(" user : [%s], session = %x already login !\n",msg_login->username,s);
            AckResult(s,MSG_C_LOGIN,MSG_ACK_USER_ALREADY_LOGIN);
            return false;
        }
    }

    PRINTF(" user : %s, session = %x ok!\n",msg_login->username,s);
    AckResult(s,MSG_C_LOGIN,MSG_ACK_SUCCESS);
    puser = NULL;
    puser = new user;
    puser->session = s;
    puser->roomid = 0;
    puser->userid = GenUserId();
    strncpy(puser->username, msg_login->username, sizeof(puser->username));

    user_online_map[s] = puser;

    
    return true;
}

BOOL tank::OnLogout(SESSION s,void *buf,DWORD buf_len)
{
    user *puser = NULL;
    PRINTF(" tank::OnLogout()\n");
    PRINTF(" delete session : %x ...",s);
    ASSERT(NULL != buf);
    ASSERT(buf_len);
    USER_ITR itr = user_online_map.find(s);
    if (itr != user_online_map.end())
    {
        PRINTF(" ok!\n",s);
   
        puser = itr->second;
        if (0 != puser->roomid)
        {
            room *proom = NULL;
            if(FindRoomBySession(s,&proom))
            {
                proom->DelUser(puser);
            }
        }

        delete puser;
        user_online_map.erase(itr);
        return true;
    }
    else
    {
        PRINTF(" not exist\n",s);
        return false;
    }

    return true;
}

BOOL tank::OnGetRoomList(SESSION s,void *buf,DWORD buf_len)
{
    BOOL bret = true;
    ROOM_ITR room_itr;
    DWORD room_count = room_map.size();

    // room_count + roomlist
    DWORD ack_buf_len = room_count * sizeof(MSG_ACK_ROOMINFO) + sizeof(DWORD);
    BYTE *ack_buf = new BYTE[ack_buf_len];
    ASSERT(ack_buf);
    DWORD *proom_count = (DWORD *)ack_buf;
    MSG_ACK_ROOMINFO *ack_room_info = (MSG_ACK_ROOMINFO *)(proom_count + 1);

    // fill room count
    *proom_count = room_count;

    // fill room list
    for (room_itr = room_map.begin();room_itr != room_map.end();room_itr++)
    {
        ack_room_info->room_id = room_itr->second->roomid;
        ack_room_info->room_master_id = room_itr->second->masterid;
        ack_room_info->room_state = room_itr->second->state;
        ack_room_info++;
    }

    bret = AckResult(s,MSG_C_GETROOMLIST,MSG_ACK_SUCCESS,ack_buf,ack_buf_len);

    return bret;
}

BOOL tank::OnJoinRoom(SESSION s,void *buf,DWORD buf_len)
{
    BOOL bret = true;
    user *puser = NULL;
    room *proom = NULL;
    ROOM_ITR room_itr;
    MSG_HEAD *pmsg_head = NULL;
    MSG_JOINROOM *pmsg_joinroom = NULL;

    // 
    bret = FindUserBySession(s,&puser);
    if (!bret)
    {
        bret = AckResult(s,MSG_C_JOINROOM,MSG_ACK_COULDNOT_JOINROOM);
        return false;
    }

    ASSERT(puser);
    if (0 != puser->roomid)
    {
        bret = AckResult(s,MSG_C_JOINROOM,MSG_ACK_COULDNOT_JOINROOM);
        return false;
    }

    pmsg_head = (MSG_HEAD *)buf;
    pmsg_joinroom = (MSG_JOINROOM *)(pmsg_head + 1);

	room_itr = room_map.find(pmsg_joinroom->room_id);
    if (room_itr == room_map.end())
    {
        bret = AckResult(s,MSG_C_JOINROOM,MSG_ACK_COULDNOT_JOINROOM);
        return false;
    }

    ASSERT(room_itr != room_map.end());

	bret = room_itr->second->AddUser(puser);
    if (!bret)
    {
        bret = AckResult(s,MSG_C_JOINROOM,MSG_ACK_COULDNOT_JOINROOM);
        return false;
    }
    
    bret = AckResult(s,MSG_C_JOINROOM,MSG_ACK_SUCCESS);
    return bret;
}

BOOL tank::OnExitRoom(SESSION s,void *buf,DWORD buf_len)
{
    BOOL bret = true;
    room *proom = NULL;
    user *puser = NULL;
    ROOM_ITR room_itr;
    bret = FindRoomBySession(s,&proom);
    ASSERT(bret);
    if (!bret)
    {
        bret = AckResult(s,MSG_C_EXITROOM,MSG_ACK_ROOM_NOT_EXIT);
        return false;
    }

    bret = FindUserBySession(s,&puser);
    ASSERT(bret);
    if (!bret)
    {
        bret = AckResult(s,MSG_C_EXITROOM,MSG_ACK_ROOM_NOT_EXIT);
        return false;
    }
    
    ASSERT(proom);
    bret = proom->DelUser(puser);
    if (!bret)
    {
        bret = AckResult(s,MSG_C_EXITROOM,MSG_ACK_ROOM_NOT_EXIT);
        return false;
    }

    if(0 == proom->GetUserCount())
    {
        room_itr = room_map.find(proom->roomid);
        ASSERT(room_itr != room_map.end());
        if (room_itr != room_map.end())
        {
            room_map.erase(room_itr);
            delete proom;
            proom = NULL;
        }
    }
    
    bret = AckResult(s,MSG_C_EXITROOM,MSG_ACK_SUCCESS);
    return bret;
}

BOOL tank::OnGetReady(SESSION s,void *buf,DWORD buf_len)
{
    BOOL bret = true;

    return bret;
}

BOOL tank::OnStartBattle(SESSION s,void *buf,DWORD buf_len)
{
    BOOL bret = true;

    return bret;
}

BOOL tank::OnCreateRoom(SESSION s,void *buf,DWORD buf_len)
{
    BOOL bret = true;
    user *puser = NULL;
    room *proom = NULL;
    ROOM_ITR room_itr;

    bret = FindUserBySession(s,&puser);
    if (!bret)
    {
        bret = AckResult(s,MSG_C_CREATEROOM,MSG_ACK_COULDNOT_CREATEROOM);
        return false;
    }

    if (0 != puser->roomid)
    {
        bret = AckResult(s,MSG_C_CREATEROOM,MSG_ACK_COULDNOT_CREATEROOM);
        return false;
    }

    proom = new room;
    ASSERT(proom != NULL);
    proom->roomid = GenRoomId();
    bret = proom->AddUser(puser);
    ASSERT(bret);
    room_map[puser->roomid] = proom;
    
    bret = AckResult(s,MSG_C_CREATEROOM,MSG_ACK_SUCCESS);
    return bret;
}

BOOL tank::OnSendTextMsg(SESSION s,void *buf,DWORD buf_len)
{
    BOOL bret = true;




    return bret;
}

BOOL tank::OnRecvTextMsg(SESSION s,void *buf,DWORD buf_len)
{
    BOOL bret = true;

    return bret;
}

BOOL tank::OnGetPlayerList(SESSION s,void* buf,DWORD buf_len)
{
    BOOL bret = true;

    return bret;
}

BOOL tank::OnFire(SESSION s,void *buf,DWORD buf_len)
{
    BOOL bret = true;

    return bret;
}


BOOL room::AddUser(user *puser)
{
    USER_ITR itr = user_map.begin();

    if (GetUserCount() > max_user)
    {
        return false;
    }

    if (state != ROOM_STATE_WAITING)
    {
        return false;
    }
    
    puser->roomid = roomid;

    user_map[puser->userid] = puser;

    if (itr == user_map.end())
    {
        masterid = puser->userid;
    }
    return true;
}

BOOL room::DelUser(user *puser)
{
    USER_ITR itr = user_map.find(puser->userid);
    if (itr != user_map.end())
    {
        if (masterid == puser->userid)
        {
            masterid = 0;
        }
        puser->roomid = 0;        
        user_map.erase(itr);

        itr = user_map.begin();
        if (itr != user_map.end())
        {
            masterid = puser->userid;
        }
        
        return true;
    }
    return false;
}

DWORD room::GetUserCount()
{
    return user_map.size();
}