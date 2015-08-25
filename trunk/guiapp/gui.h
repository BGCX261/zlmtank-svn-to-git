#ifndef __HGEAPP_H__ 
#define __HGEAPP_H__ 
 
#include "hge.h" 
#include "hgegui.h" 
#include "hgefont.h" 
 
class GUIApp : public hgeGUI { 
protected: 
	// cursor 
	HTEXTURE cursorTexture; 
	hgeSprite *cursorSprite; 
	HGE *myHge; 
	class dummyCtrl : public hgeGUIObject { 
	public: 
		dummyCtrl () : hgeGUIObject () { 
			bStatic=true; 
		} 
		 
		void Render () { 
		} 
	}; 
public: 
	GUIApp (HGE *hge); 
	bool LoadCursor ( const char*fname, float w = 32, float h = 32 ); 
	bool Input_GetKeyState (int key); 
	bool FrameFunc (float dt); 
	bool RenderFunc(float dt);
	hgeFont *CreateFont ( const char *fname ); 
 
	virtual void PreRender (); 
	virtual void PostRender (); 
}; 
 
#endif 

