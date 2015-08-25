#ifndef __GUIAPPWINDOW_H__ 
#define __GUIAPPWINDOW_H__ 
 
#include "guiobj.h"
#include "m_list.h"
#include "hge.h"
 
class GUIApp; 
 
class GUIAppWindow : public GUIAppObject { 
protected: 
 
	TList <GUIAppObject> objects;; 
	hgeQuad windowBg; 
	float wWidth, wHeight; 
	float ox, oy; 
	GUIApp *parentapp; 
	int curSelection; 
	DWORD bgColor; 
	DWORD frameColor; 
 
	void SetupSize(float x, float y, float w, float h ); 
	void MoveObjects(float dx, float dy ); 
	void CheckObjectFocus (); 
	void SelectObject ( int id ); 
	void SelectNext(); 
	void OnTop(); 
public: 
 
	GUIAppWindow( GUIApp *parent, int _id, float x, float y, float w, float h ); 
 
	void AddCtrl ( GUIAppObject *obj ); 
	virtual void SetBGColor ( DWORD clr ); 
	virtual void SetFrameColor ( DWORD clr ); 
	virtual void Render(); 
	virtual void Move ( float dx, float dy ); 
 
	// overloads 
	virtual void Focus(bool bFocused); 
	virtual void MouseOver(bool bOver); 
	virtual bool MouseMove(float x, float y); 
	virtual bool MouseLButton(bool bDown); 
	virtual bool MouseRButton(bool bDown); 
	virtual bool MouseWheel(int nNotches); 
	virtual bool KeyClick(int key, int chr); 
	virtual void Update(float dt); 
 
	// implementation 
	virtual void OnEvent(int obj_id); 
}; 
 
#endif 

