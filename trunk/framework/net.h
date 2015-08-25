#ifndef NET_H
#define NET_H



typedef struct _netconfig
{
    DWORD addr;
	WORD port;

}NetConfig;

typedef struct _session_cb
{
    SOCKET socket;
	struct sockaddr_in addr;

}SESSIONCB;
typedef std::map<SOCKET,SESSIONCB *>::iterator SESSIONCB_ITR;
typedef DWORD SESSION;

class ue
{
public:
    virtual BOOL Init(){return false;};
	virtual BOOL deInit(){return false;};
	virtual BOOL OnException(SESSION){return false;};
    virtual BOOL OnRecieve(SESSION,void*,DWORD){return false;};
};



typedef struct _gamedb
{
    NetConfig config;
	DWORD version;	
	WSADATA ws;
	SOCKET ServerSocket;
	DWORD listen_backlog;
	DWORD max_socket_num;
	struct timeval timeout;
	BOOL loopflag;
	fd_set rfd;
	fd_set wfd;
	fd_set efd;
	CHAR *buf;
	DWORD buf_len;
    std::map<SOCKET,SESSIONCB *> session_map;


	ue *pue;
}GAMEDB;






GAMEDB *GetGameBD();
BOOL InitGameDB();
BOOL FreeGameDB();
BOOL StartServer();

BOOL ShutdownServer();

BOOL MainLoop();
BOOL FillAllFd();
BOOL ProcAllFd();
BOOL OnException(SOCKET *socket);
BOOL OnRead(SOCKET *socket);
BOOL OnWrite(SOCKET *socket);
BOOL OnAccept(SOCKET *socket);

BOOL SendMsg(SESSION,void *,DWORD &);
BOOL RegisterUE(ue *);
ue *GetUE();

SESSIONCB *GetSessionCB();
void DelSessionCB(SESSIONCB **cb);

#endif
