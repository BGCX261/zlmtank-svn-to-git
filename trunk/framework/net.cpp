#include "def.h"


GAMEDB *g_gamedb = NULL;

GAMEDB *GetGameBD()
{
    if (!g_gamedb)
    {
        //g_gamedb = (GAMEDB *)malloc(sizeof(GAMEDB));
        g_gamedb = new GAMEDB;
    }
    return g_gamedb;
}


BOOL InitGameDB()
{
    GAMEDB *gamedb = GetGameBD();

    // 填写本服务器信息
    gamedb->config.addr = 0;
    gamedb->config.port = 1400;

    gamedb->version = 0x1;

    gamedb->ServerSocket = 0;
    gamedb->listen_backlog = 10;
    gamedb->max_socket_num = FD_SETSIZE;
    gamedb->loopflag = false;

    gamedb->timeout.tv_sec=1;
    gamedb->timeout.tv_usec=0;

    gamedb->buf_len = 512;
    gamedb->buf = (CHAR *)malloc(gamedb->buf_len);

    gamedb->session_map.clear();

    gamedb->pue = NULL;

    return true;
}

BOOL FreeGameDB()
{
    GAMEDB *gamedb = GetGameBD();
    // todo
    free(gamedb->buf);


    delete g_gamedb;
    g_gamedb = NULL;
	return true;
    
}


BOOL StartServer()
{
    GAMEDB *gamedb = GetGameBD();
    DWORD rc = 0;
    BOOL bret = false;
    DWORD ul = 1;
    
    rc = WSAStartup(MAKEWORD(2,2), &(gamedb->ws));
    if (rc)
    {
        PRINTF("StartServer::WSAStartup faild! e:%x\n",GetLastError());
        goto EXIT;
    }

    gamedb->ServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (INVALID_SOCKET == gamedb->ServerSocket)
    {
        PRINTF("StartServer::socket faild! e:%x\n",GetLastError());
        goto EXIT;
    }
    
    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET; 
    saddr.sin_port = htons(gamedb->config.port); 
    saddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    rc = bind(gamedb->ServerSocket,(struct sockaddr *)&saddr,sizeof(saddr));
    if (rc)
    {
        PRINTF("StartServer::bind faild! e:%x\n",GetLastError());
        goto EXIT;
    }

    rc = listen(gamedb->ServerSocket, gamedb->listen_backlog);
    if (rc)
    {
        PRINTF("StartServer::listen faild! e:%x\n",GetLastError());
        goto EXIT;
    }
    
    ioctlsocket(gamedb->ServerSocket,FIONBIO,&ul);    

    PRINTF(" start server success!!!\n");
    gamedb->loopflag = true;
    bret = MainLoop();




    
EXIT:

    return bret;
}

BOOL ShutdownServer()
{
    GAMEDB *gamedb = GetGameBD();

    //do something


    if (INVALID_SOCKET != gamedb->ServerSocket)
    {        
        closesocket(gamedb->ServerSocket);
        gamedb->ServerSocket = INVALID_SOCKET;
    }
    WSACleanup();
    return true;
}

BOOL MainLoop()
{
    BOOL bret = false;
    BOOL loopflag = true;
    DWORD sel_rc = 0;

    GAMEDB *gamedb = GetGameBD();
    PRINTF(" start MainLoop...\n");

    while (gamedb->loopflag)
    {
        bret = FillAllFd();
        ASSERT(true == bret);//肯定有一个listen用的socket

        

        sel_rc = select(0,
                        &(gamedb->rfd),
                        &(gamedb->wfd),
                        &(gamedb->efd),
                        &(gamedb->timeout));
        if (0 == sel_rc)
        {
            continue;
        }

        bret = ProcAllFd();
		Sleep(800);
    }

    PRINTF(" stop MainLoop...\n");
    return bret;
}

BOOL FillAllFd()
{
    GAMEDB *gamedb = GetGameBD();
    SESSIONCB_ITR itr;
    
    FD_ZERO(&(gamedb->rfd));
    FD_ZERO(&(gamedb->wfd));
    FD_ZERO(&(gamedb->efd));

    // 加入listen用的socket先
    FD_SET(gamedb->ServerSocket,&(gamedb->rfd));

    // 加入所有session socket到读写fd
    ASSERT(gamedb->session_map.size() < gamedb->max_socket_num);
    itr = gamedb->session_map.begin();
    for (;itr != gamedb->session_map.end(); itr++)
    {
        SESSIONCB *cb;
        cb = itr->second;
        FD_SET(cb->socket, &(gamedb->rfd));
        FD_SET(cb->socket, &(gamedb->wfd));
        FD_SET(cb->socket, &(gamedb->efd));
    }

    return true;
}

BOOL ProcAllFd()
{
    DWORD ul_rc = 0;
    BOOL bret = true;
    GAMEDB *gamedb = GetGameBD();
    DWORD ii = 0;
    
    // 现处理异常socket
    for (ii = 0; ii < gamedb->efd.fd_count; ii++)
    {
        bret = OnException(&(gamedb->efd.fd_array[ii]));
    }
    
    // 检查是否有人connect过来
    ul_rc = FD_ISSET(gamedb->ServerSocket,&(gamedb->rfd));
    if (0 != ul_rc)
    {
        // 移除这个listen socket
        FD_CLR(gamedb->ServerSocket, &(gamedb->rfd));
        bret = OnAccept(&(gamedb->ServerSocket));
    }

    // 然后处理要读的socket
    for (ii = 0; ii < gamedb->rfd.fd_count; ii++)
    {
        bret = OnRead(&(gamedb->rfd.fd_array[ii]));
    }
    
    // 最后处理写socket
    for (ii = 0; ii < gamedb->wfd.fd_count; ii++)
    {
        bret = OnWrite(&(gamedb->wfd.fd_array[ii]));
    }

    return true;
}

BOOL OnException(SOCKET *socket)
{
    GAMEDB *gamedb = GetGameBD();
    ue *ue = GetUE();
    SESSIONCB *cb = NULL;
    SESSIONCB_ITR itr;
    // 通知用户搞点什么
    if (NULL != ue)
    {
        ue->OnException((SESSION)(*socket));
    }

    itr = gamedb->session_map.find(*socket);
    if (itr != gamedb->session_map.end())
    {
		cb = itr->second;
        gamedb->session_map.erase(itr);

        PRINTF(" OnException::remove session from %d.%d.%d.%d : %d\n",
               cb->addr.sin_addr.s_net,
               cb->addr.sin_addr.s_host,
               cb->addr.sin_addr.s_lh,
               cb->addr.sin_addr.s_impno,
               cb->addr.sin_port);
        
        closesocket(cb->socket);
        DelSessionCB(&cb);
    }
    //

    
    return true;
}

BOOL OnRead(SOCKET *socket)
{
    ue *ue = GetUE();
    GAMEDB *gamedb = GetGameBD();
    DWORD socket_rc = 0;
    SESSIONCB *cb = NULL;
    SESSIONCB_ITR itr = gamedb->session_map.find(*socket);
    if (itr == gamedb->session_map.end())
    {
        return false;
    }

    cb = itr->second;
    PRINTF(" OnRead::recv from %d.%d.%d.%d:%d",
           cb->addr.sin_addr.s_net,
           cb->addr.sin_addr.s_host,
           cb->addr.sin_addr.s_lh,
           cb->addr.sin_addr.s_impno,
           cb->addr.sin_port);
    
    socket_rc = recv(*socket, gamedb->buf, gamedb->buf_len,0);
    if (SOCKET_ERROR == socket_rc)
    {
        PRINTF(" error!" );
        return false;
    }

    PRINTF(" , size = %d\n", socket_rc);

    if (0 == socket_rc)// disconnet
    {
        (void)OnException(socket);
        return true;
    }
    
    if (ue)
    {
        (void)ue->OnRecieve((SESSION)(*socket),gamedb->buf,gamedb->buf_len);
    }

    return true;
}

BOOL OnWrite(SOCKET *socket)
{
    return true;
}

BOOL OnAccept(SOCKET *socket)
{
    GAMEDB *gamedb = GetGameBD();
    SOCKET new_sock = INVALID_SOCKET;
    SESSIONCB *cb = NULL;
    //struct sockaddr addr;
    struct sockaddr_in addr_in;
	int addr_len = 0;


	addr_len = sizeof(addr_in);
    memset(&addr_in,0,addr_len);

    new_sock = accept(gamedb->ServerSocket,
                      (struct sockaddr *)&addr_in,
                      &addr_len);
    if (INVALID_SOCKET == new_sock)
    {
        PRINTF(" OnAccept::accept faild!\n");
        return false;
    }

    cb = GetSessionCB();
    if (NULL == cb)
    {
        PRINTF(" OnAccept::GetSessionCB faild!\n");
        return false;
    }

    cb->socket = new_sock;
    cb->addr = addr_in;
    gamedb->session_map[new_sock] = cb;

    PRINTF(" create a session from %d.%d.%d.%d:%d\n",
           addr_in.sin_addr.s_net,
           addr_in.sin_addr.s_host,
           addr_in.sin_addr.s_lh,
           addr_in.sin_addr.s_impno,
           addr_in.sin_port);
    
    return true;
}

BOOL SendMsg(SESSION s,void *buf,DWORD &buf_len)
{
    SESSIONCB_ITR itr;
    DWORD ul_rc = 0;
	SESSIONCB *cb = NULL;
    GAMEDB *gamedb = GetGameBD();
    SOCKET sock = s;

    itr = gamedb->session_map.find(sock);
    if (itr != gamedb->session_map.end())
    {
        cb = itr->second;
        ul_rc = send(cb->socket,
                      (CHAR *)buf,
                      buf_len,
                      0);//??
        if (SOCKET_ERROR == ul_rc)
        {
            return false;
        }
    }
    else
    {
        ASSERT(false);
        return false;
    }


    return true;
}

BOOL RegisterUE(ue *pue)
{
    GAMEDB *gamedb = GetGameBD();
    ASSERT(gamedb->pue == NULL);
    gamedb->pue = pue;
    return true;
}

ue *GetUE()
{
    return GetGameBD()->pue;
}

SESSIONCB *GetSessionCB()
{
    SESSIONCB *cb = (SESSIONCB *)malloc(sizeof(SESSIONCB));
    memset(cb,0,sizeof(SESSIONCB));

    return cb;
}

void DelSessionCB(SESSIONCB **cb)
{
    free(*cb);
    *cb = NULL;
    return;
}
