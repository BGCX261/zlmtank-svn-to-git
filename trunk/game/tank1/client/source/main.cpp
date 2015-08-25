/*
** Haaf's Game Engine 1.8
** Copyright (C) 2003-2007, Relish Games
** hge.relishgames.com
**
** hge_tut01 - Minimal HGE application
*/

#include <list>

#include "def.h"

#include "tank.h"



int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    tank2 atank;
    Game::Instance()->RegisterUe(&atank);

    if(Game::Instance()->InitGame())
    {
        Game::Instance()->StartGame();
    }
    else
    {
        // If HGE initialization failed show error message
        //MessageBox(NULL, hge->System_GetErrorMessage(), "Error", MB_OK | MB_ICONERROR | MB_APPLMODAL);
    }
    
    // Now ESC has been pressed or the user
    // has closed the window by other means.
    
    
    Game::Instance()->StopGame();
    Game::Instance()->FreeGame();
    
    
    return 0;
}
