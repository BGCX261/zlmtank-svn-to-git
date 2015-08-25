

//#include "def.h"
#include "types.h"
#include "game.h"



Game *Game::pinst = NULL;

Game *Game::Instance()
{
    if (NULL == pinst)
    {
        pinst = new Game;
    }
    return pinst;
}

Game::Game()
{
    m_pue = NULL;
    m_bClearBG = TRUE;
    m_bShowFPS = TRUE;
    m_dt = 0;
    m_scene_itr = m_scene_list.end();
}

Game::~Game()
{

}

BOOL Game::InitGame()
{
    // Here we use global pointer to HGE interface.
    // Instead you may use hgeCreate() every
    // time you need access to HGE. Just be sure to
    // have a corresponding hge->Release()
    // for each call to hgeCreate()
    HGE *hge = hgeCreate(HGE_VERSION);
    m_SysInterface.SetHge(hge);

    // Set up log file, frame function, render function and window title
    hge->System_SetState(HGE_LOGFILE, "tank.log");
    // Set our frame function
    hge->System_SetState(HGE_FRAMEFUNC, FrameFunc);
    hge->System_SetState(HGE_RENDERFUNC, RenderFunc);
    //hge->System_SetState(HGE_EXITFUNC, ExitFunc);
    // Set the window title
    hge->System_SetState(HGE_TITLE, "MY SLG ENG");
    
    // Run in windowed mode
    hge->System_SetState(HGE_SCREENWIDTH, 800);
    hge->System_SetState(HGE_SCREENHEIGHT, 600);
    hge->System_SetState(HGE_SCREENBPP, 32);
    hge->System_SetState(HGE_WINDOWED, true);
    
    // Don't use BASS for sound
    hge->System_SetState(HGE_USESOUND, false);
    hge->System_SetState(HGE_HIDEMOUSE, false);

    hge->System_SetState(HGE_DONTSUSPEND, true);
    //hge->System_SetState(HGE_FPS,256);




	
    // Tries to initiate HGE with the states set.
    // If something goes wrong, "false" is returned
    // and more specific description of what have
    // happened can be read with System_GetErrorMessage().
    if (hge->System_Initiate())
    {
        //todo:
        hgeFont *fnt=new hgeFont(hge->Resource_MakePath("res\\ComicSansMs.fnt"));
        hge->System_GetErrorMessage();
        m_SysInterface.SetFont(SYS_FONT_TYPE,fnt);

        if (m_pue->InitGame())
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }

    }
    else
    {
        return FALSE;
    }
}

BOOL Game::StartGame()
{
    // Starts running FrameFunc().
    // Note that the execution "stops" here
    // until "true" is returned from FrameFunc().
    if(!m_pue->StartGame())
		return false;
    return m_SysInterface.m_hge->System_Start();
}

BOOL Game::StopGame()
{
    m_pue->ShutdonwGame();

    for (DWORD ii = 0; ii < MAX_FONT_TYPE; ii++)
    {
        if (NULL == m_SysInterface.m_pfnt[ii])
        {
            delete m_SysInterface.m_pfnt[ii];
            m_SysInterface.m_pfnt[ii] = NULL;
        }
    }




    // Restore video mode and free
    // all allocated resources
    m_SysInterface.m_hge->System_Shutdown();    
    return TRUE;
}

BOOL Game::FreeGame()
{
    // Release the HGE interface.
    // If there are no more references,
    // the HGE object will be deleted.
    m_scene_itr = m_scene_list.begin();
	for (; m_scene_itr != m_scene_list.end();)
	{
        delete *m_scene_itr;
		m_scene_list.erase(m_scene_itr++);
	}
    m_SysInterface.m_hge->Release();
    return FALSE;
}


// This function will be called by HGE once per frame.
// Put your game loop code here. In this example we
// just check whether ESC key has been pressed.
BOOL Game::FrameFunc()
{
    Game *game =  Game::Instance();
    // By returning "true" we tell HGE
    // to stop running the application.
    if (Game::Instance()->m_scene_itr != Game::Instance()->m_scene_list.end())
    {
        (*(Game::Instance()->m_scene_itr))->FrameFunc(game->m_dt);
    }
    
    // Continue execution
    return false;
}

// This function will be called by HGE when
// the application window should be redrawn.
// Put your rendering code here.
BOOL Game::RenderFunc()
{
    Game *game =  Game::Instance();
    HGE *hge = game->m_SysInterface.GetHge();
    hgeFont * fnt = game->m_SysInterface.GetFont(SYS_FONT_TYPE);

    game->m_dt =hge->Timer_GetDelta();
    // Begin rendering quads.
    // This function must be called
    // before any actual rendering.
    hge->Gfx_BeginScene();

    if (TRUE == Game::Instance()->m_bClearBG)
    {
        // Clear screen with black color
        hge->Gfx_Clear(0);
    }
    
    if (Game::Instance()->m_scene_itr != Game::Instance()->m_scene_list.end())
    {
        (*(Game::Instance()->m_scene_itr))->RenderFunc(game->m_dt);
    }
    
    if (TRUE == Game::Instance()->m_bShowFPS)
    {
        fnt->printf(5, 5, HGETEXT_LEFT, "dt:%.3f\nFPS:%d", game->m_dt, hge->Timer_GetFPS());
    }

    
    // End rendering and update the screen
    hge->Gfx_EndScene();
    
    // RenderFunc should always return false
    return false;
}

BOOL Game::ExitFunc()
{
    //Game *game =  Game::Instance();
    //game->StopGame();
    //game->FreeGame();
    return true;
}

BOOL Game::InsertScene(scene *s)
{
    m_scene_list.push_back(s);
    return true;
}


BOOL Game::GotoFirstScene()
{
    SCENE_LSIT_ITR last_scene = m_scene_itr;
    m_scene_itr = m_scene_list.begin();
	if (last_scene != m_scene_list.end())
	{
        if (!(*last_scene)->Leave())
			return false;
	}
	if (m_scene_itr != m_scene_list.end())
	{
        if (!(*m_scene_itr)->Entry())
			return false;
	}
	
    return true;
}

BOOL Game::GotoNextScene()
{
    SCENE_LSIT_ITR last_scene = m_scene_itr;
    if (m_scene_itr != m_scene_list.end())
    {
        m_scene_itr++;
    }

    if (m_scene_itr == m_scene_list.end())
    {
        m_scene_itr = m_scene_list.begin();
    }

	if (last_scene != m_scene_list.end())
	{
        if (!(*last_scene)->Leave())
			return false;
	}
	if (m_scene_itr != m_scene_list.end())
	{
        if (!(*m_scene_itr)->Entry())
			return false;
	}

    return true;
}

BOOL Game::GotoPrevScene()
{
    SCENE_LSIT_ITR last_scene = m_scene_itr;
    if (m_scene_itr != m_scene_list.begin())
    {
        m_scene_itr--;
    }

    if (m_scene_itr == m_scene_list.begin())
    {
        m_scene_itr = m_scene_list.end();
        m_scene_itr--;
    }
	
	if (last_scene != m_scene_list.end())
	{
        if (!(*last_scene)->Leave())
			return false;
	}
	if (m_scene_itr != m_scene_list.end())
	{
        if (!(*m_scene_itr)->Entry())
			return false;
	}
    return true;
}
BOOL Game::GotoLastScene()
{
    SCENE_LSIT_ITR last_scene = m_scene_itr;
    m_scene_itr = m_scene_list.end();
    m_scene_itr--;

	if (last_scene != m_scene_list.end())
	{
        if (!(*last_scene)->Leave())
			return false;
	}
	if (m_scene_itr != m_scene_list.end())
	{
        if (!(*m_scene_itr)->Entry())
			return false;
	}
	
    return true;
}

sysinterface *Game::GetSysInterface()
{
    return &m_SysInterface;
}

bool Game::RegisterUe(ue *ue)
{
    if (ue != NULL)
    {
        m_pue = ue;
    }
    return TRUE;
}
