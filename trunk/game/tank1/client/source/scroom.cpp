#include "def.h"
#include "tank.h"
#include "scroom.h"
#include "guiloginwnd.h"

RoomWindow::RoomWindow(GUIApp *parentApp, int _id, float x, float y,bool fixed ) :  GUIAppWindow(parentApp,_id, x,y, 800, 600 )

{

    font = Game::Instance()->GetSysInterface()->GetFont(SYS_FONT_TYPE);
    m_RoomList = new GUIAppListBox(GUIAPP_ID_LISTBOX_ROOMLIST,
                                                        50,50,400,500,
                                                        NULL,NULL,NULL,NULL,NULL,NULL);
    AddCtrl(m_RoomList);
    // active elements
    join = new GUIAppButton(GUIAPP_ID_BNT_JOIN, 600, 400, 100, 20,NULL,0,0);
    AddCtrl(join);
    create = new GUIAppButton(GUIAPP_ID_BNT_CREATE, 600, 450, 100, 20,NULL,0,0);
    AddCtrl(create);
}

void RoomWindow::OnEvent (int id) {
    switch (id)
    {
        case GUIAPP_ID_BNT_JOIN:
            Game::Instance()->GotoNextScene();
        break;
        case GUIAPP_ID_BNT_CREATE:
            Game::Instance()->GotoPrevScene();

        break;
    default:
        break;
    }
}

scRoom::scRoom(tank2 *t)
{
    m_tank = t;
}

BOOL scRoom::init()
{
    HGE *hge = Game::Instance()->GetSysInterface()->GetHge();
    GUIApp *app = new GUIApp(hge);
    
    RoomWindow *wnd = new RoomWindow(app,1,0.0  , 0.0, TRUE);
    wnd->SetBGColor(ARGB(0xaf,0x10,0x10,0xf0));
    app->AddCtrl(wnd);

     
    InsertGUIApp(app);

	return true;
}

void scRoom::deinit()
{

}
void scRoom::PostRender(float d)
{
    hgeFont *fnt = Game::Instance()->GetSysInterface()->GetFont(SYS_FONT_TYPE);
    fnt->printf(100.0,100.0,HGETEXT_LEFT,"i am scRoom!!!");

 


    
    return;
}


BOOL scRoom::Leave()
{
    BOOL bret = true;
    //test

    m_tank->m_state.m_state = STATE_BATTLE;
	m_tank->m_state.m_username = "zlm";
	m_tank->m_state.m_password = "aaa";
	m_tank->m_state.m_room.MyId = 1;
	m_tank->m_state.m_room.MasterId = 1;
	m_tank->m_state.m_room.StageId = 1;
    m_tank->m_state.m_room.MapType = 1;
	m_tank->m_state.m_room.MapUnits = 80;
	
	player2 my;
    strcpy(my.Name,"sb1");
	my.Id = 1;
	my.Level = 50;
	my.Money = 10000;
	my.side = PLAYER_SIDE_A;
	my.TankId = 1;
	my.position = 0;

	m_tank->m_state.m_room.m_player_vec.push_back(my);

    strcpy(my.Name,"sb2");
    my.Id = 2;
	my.TankId = 2;
	my.position = 16;
	m_tank->m_state.m_room.m_player_vec.push_back(my);

	return bret;
}

