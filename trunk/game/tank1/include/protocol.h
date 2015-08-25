#ifndef _PROTOCOL_H
#define _PROTOCOL_H


typedef struct _msghead
{
    DWORD msgtype;
	DWORD msgack;
    DWORD msglen;
}MSG_HEAD;

#define MSG_REQ    0x0
#define MSG_ACK    0x1


#define MSG_NULL                   0x0
#define MSG_KEEPALIVE             0x1
//#define MSG_ACK                    0x2
#define MSG_C_LOGIN                  0x3
#define MSG_C_LOGOUT                 0x4

#define MSG_C_GETROOMLIST            0x5
#define MSG_C_CREATEROOM             0x6
#define MSG_C_JOINROOM               0x7
#define MSG_C_EXITROOM               0x8

#define MSG_C_GETPLAYERLSIT         0x9
#define MSG_C_GETREADY               0xa
#define MSG_C_STARTBATTLE           0xb

#define MSG_C_SHOOT                  0xc
#define MSG_C_SENDTEXTMSG            0xd
#define MSG_C_RECVTEXTMSG            0xe




//ack
#define MSG_ACK_SUCCESS                           0x0
#define MSG_ACK_USERNAME_OR_PASSWORD_INVALID    0x1
#define MSG_ACK_USER_ALREADY_LOGIN               0x2
#define MSG_ACK_COULDNOT_CREATEROOM               0x3
#define MSG_ACK_COULDNOT_JOINROOM                 0x4
#define MSG_ACK_ROOM_NOT_EXIT                      0x5
//#define MSG_ACK_ROOM_NOT_EXIT                      0x5




typedef struct _msg_ack
{
    DWORD result;
}MSG_ACK_HEAD;

//login
#define USER_NAME_LEN 32
#define USER_PASSWORD_LEN 32
typedef struct _msg_login
{
    CHAR username[USER_NAME_LEN];
	CHAR password[USER_PASSWORD_LEN];
}MSG_LOGIN;


//create room
typedef struct _msg_createroom
{

}MSG_CREATEROOM;

//join room
typedef struct _msg_joinroom
{
    DWORD room_id;
}MSG_JOINROOM;


//get room list
typedef struct _msg_getroomlist
{

}MSG_GETROOMLIST;


//ack get room info
typedef struct _msg_ack_roominfo
{
    DWORD room_id;
	DWORD room_master_id;
	DWORD room_state;
}MSG_ACK_ROOMINFO;
//ack get room list 

typedef struct _msg_ack_getroomlist
{
	DWORD room_count;
}MSG_ACK_GETROOMLIST;

#endif
