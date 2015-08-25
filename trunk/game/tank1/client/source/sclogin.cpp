#include "def.h"
#include "protocol.h"
#include "sclogin.h"
#include "guiloginwnd.h"
#include "tank.h"


LoginWindow::LoginWindow(GUIApp *parentApp, int _id, float x, float y,bool fixed ) :  GUIAppWindow(parentApp,_id, x,y, 200, 200 )

{

font = Game::Instance()->GetSysInterface()->GetFont(TANK_FONT_1);

edit1 = new GUIAppEdit(GUIAPP_ID_EDIT_USERNAME,40, 50, 120, font, GUIAPPEDIT_FLAGS_NOMARL,16);
AddCtrl(edit1);

edit2 = new GUIAppEdit(GUIAPP_ID_EDIT_PASSWORD,40, 80, 120, font, GUIAPPEDIT_FLAGS_PASSWORD,16);
AddCtrl(edit2);

label1 = new GUIAppLabel(GUIAPP_ID_LABLE_STATUS, 40,110, font, "press connect button", 0xffffffff,0);
AddCtrl(label1);


// active elements
button = new GUIAppButton(GUIAPP_ID_BTN_LOGIN, 70, 140, 60, 20,NULL,0,0);
AddCtrl(button);

}

void LoginWindow::OnEvent (int id) {
    BOOL bret = true;
    switch (id)
    {
        case GUIAPP_ID_BTN_LOGIN:
        {
            bret = SendLoginMsgReq();
            // and disabel this button...
            label1->SetText("waiting for server respones...");
            //button->bVisible = false;
        }
        break;
    default:
        break;
    }
}

void LoginWindow::Update(float dt)
{
    RecvLoginMsgAck();
    GUIAppWindow::Update(dt);
}

BOOL LoginWindow::SendLoginMsgReq()
{
    BOOL bret = true;
    DWORD NetWorkState = 0;
    BYTE buf[128] = {0};
    DWORD buf_len = 128;
    MSG_HEAD *pmsg_head = NULL;
    MSG_LOGIN *pmsg_login = NULL;
    tank2 *ptank = (tank2 *)Game::Instance()->GetUE();

    ptank->m_state.m_username = edit1->GetText();
    ptank->m_state.m_password = edit2->GetText();
    

    pmsg_head = (MSG_HEAD *)buf;
    pmsg_head->msgtype = MSG_C_LOGIN;
    pmsg_head->msglen = sizeof(MSG_LOGIN);
    pmsg_head->msgack = MSG_REQ;
    pmsg_login = (MSG_LOGIN *)(pmsg_head + 1);
    strncpy(pmsg_login->username, ptank->m_state.m_username.c_str(), sizeof(pmsg_login->username)-1);
    strncpy(pmsg_login->password, ptank->m_state.m_password.c_str(), sizeof(pmsg_login->password)-1);
    buf_len = sizeof(MSG_HEAD) + sizeof(MSG_LOGIN);
    return ptank->SendMsg(buf,buf_len);

}


BOOL LoginWindow::RecvLoginMsgAck()
{
    BOOL bret = true;
    DWORD NetWorkState = 0;
    BYTE buf[64] = {0};
    DWORD buf_len = 64;
    DWORD buf_len_rcv = 0;
    MSG_HEAD *pmsg_head = NULL;
    MSG_ACK_HEAD *pmsg_ack_head = NULL;
    tank2 *ptank = (tank2 *)Game::Instance()->GetUE();


    buf_len_rcv = buf_len;
    bret = ptank->RecvMsg(buf,buf_len_rcv);
    if(!bret)
    {
        return false;
    }
    pmsg_head = (MSG_HEAD *)buf;

    if (MSG_C_LOGIN == pmsg_head->msgtype &&
        MSG_ACK == pmsg_head->msgack)
    {
        pmsg_ack_head = (MSG_ACK_HEAD *)(pmsg_head +1);
        if (MSG_ACK_SUCCESS == pmsg_ack_head->result)
        {
            Game::Instance()->GotoNextScene();
            return true;
        }
        else
        {
            return false;
        }
    }


}

BOOL scLogin::init()
{
    HGE *hge = Game::Instance()->GetSysInterface()->GetHge();
    m_app = new GUIApp(hge);
    
    m_wnd = new LoginWindow(m_app,1,100.0  , 100.0, TRUE);
    m_wnd->SetBGColor(ARGB(0x0,0xff,0xff,0xff));
    m_app->AddCtrl(m_wnd);
    InsertGUIApp(m_app);
	return true;
}

void scLogin::deinit()
{
    if (NULL != m_wnd)
    {
        delete m_wnd;
        m_wnd = NULL;
    }
}
void scLogin::PostRender(float d)
{
    tank2 *ptank = (tank2 *)Game::Instance()->GetUE();
    ptank->PrintDebugInfo();
    return;
}

