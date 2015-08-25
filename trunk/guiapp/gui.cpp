#include <STDIO.H>    
#include "gui.h"    
   
GUIApp::GUIApp(HGE *hge) : hgeGUI () {   
    myHge = hge;   
    dummyCtrl *ctlr = new dummyCtrl ();   
    AddCtrl(ctlr);   
}   
   
bool GUIApp::LoadCursor ( const char*fname, float w, float h ) {   
    cursorTexture = myHge->Texture_Load(fname);   
    if ( cursorTexture ) {   
        cursorSprite=new hgeSprite(cursorTexture,0,0,w,h);   
        SetCursor(cursorSprite);   
        return true;   
    } else {   
        return false;   
    }   
}   
   
bool GUIApp::Input_GetKeyState (int key) {   
    return myHge->Input_GetKeyState(key);   
}   
   
void GUIApp::PreRender () {   
}   
   
void GUIApp::PostRender () {   
}   
   
bool GUIApp::FrameFunc(float dt) {
    Update(dt);     
    return false;   
}   

bool GUIApp::RenderFunc(float dt) {   
    //myHge->Gfx_BeginScene();   
    //myHge->Gfx_Clear(0);   
    PreRender();   
    Render();   
    PostRender();   
    //myHge->Gfx_EndScene();   
    return false;   
}   

hgeFont *GUIApp::CreateFont ( const char *fname ) {   
    FILE * in = fopen (fname,"r");   
    if ( !in ) {   
        return 0;   
    } else {   
        fclose(in);   
        hgeFont *fnt = new hgeFont(fname);   
        return fnt;   
    }   
}   
