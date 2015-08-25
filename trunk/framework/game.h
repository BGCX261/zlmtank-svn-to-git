#ifndef _GAME_H
#define _GAME_H

#include <list>
#include "scene.h"
#include "ue.h"
#include "sysinterface.h"


typedef std::list<scene *>::iterator SCENE_LSIT_ITR;

#define GETHGE() Game::Instance()->Instance()->GetSysInterface()->GetHge();
#define GETFONT(T) Game::Instance()->Instance()->GetSysInterface()->GetFont(T);
#define GETSYSFONT() GETFONT(SYS_FONT_TYPE)

class Game
{
public:
    static Game *pinst;
    ue *m_pue;


    std::list<scene *> m_scene_list;
    SCENE_LSIT_ITR m_scene_itr;

    float m_dt;
    // show FPS
    BOOL m_bShowFPS;

    // auto clear background to black
    BOOL m_bClearBG;

    sysinterface m_SysInterface;
	
    Game();
    ~Game();
    static Game *Instance();
	
    BOOL InitGame();
    BOOL StartGame();
    BOOL StopGame();
    BOOL FreeGame();
    BOOL PauseGame();
    BOOL ResumeGame();

    //scene mgr
    BOOL InsertScene(scene *);

    //scene contorl
    BOOL GotoFirstScene();
    BOOL GotoNextScene();
    BOOL GotoPrevScene();
    BOOL GotoLastScene();
    //RETCODE RemoveScene(scene *);

    //util
    BOOL RegisterUe(ue *);
	ue *GetUE(){return m_pue;};
    sysinterface *GetSysInterface();


private:
    static bool FrameFunc();
    static bool RenderFunc();	
    static bool ExitFunc();	

};




#endif
