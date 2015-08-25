#ifndef __GUIAPPLISTBOX_H__ 
#define __GUIAPPLISTBOX_H__ 


#include "hge.h" 
#include "hgefont.h" 
#include "hgevector.h" 
#include "m_str.h"
#include "m_list.h"
#include "guiobj.h"
#include "guiscrollbar.h"

class GUIAppListBox : public GUIAppObject { 
protected: 
class GUIAppItemList : public TList  <GUIAppObject> { 
public: 
GUIAppItemList () : TList <GUIAppObject> (false){}; 
};
 
	enum Scrollers { 
		VSCROLL, HSCROLL 
	}; 
	 
	// list box data 
	GUIAppItemList objects; 
	hgeQuad selection; 
	hgeQuad background; 
	DWORD frameColor; 
	DWORD bgColor; 
	DWORD selColor; 
	 
	// selection 
	int selectIndex; 
	GUIAppObject *selectObject; 
 
	// tuning 
	float sepHeight; 
 
	// runtime data 
	float listWidth; 
	float listHeight; 
	int clipWidth; 
	int clipHeight; 
	bool vsVisible; 
	bool hsVisible; 
	float oldVValue; 
	float oldHValue; 
 
	// helpers 
	void SetupSize(float w, float h ); 
	void SetupList (); 
	void SetupSelection(); 
	void FocusSelection (); 
public: 
	// allow to set scroll parameters 
	GUIAppScrollBar *hscroll; 
	GUIAppScrollBar *vscroll; 
 
	GUIAppListBox ( int id, float x, float y, float width, float height,  
		const char *btn11, const char *btn12, const char *btn13,  
		const char *btn21, const char *btn22, const char *btn23 ); 
	~GUIAppListBox (); 
 
	// implementation 
	virtual void Move (float dx, float dy ); 
	virtual void Render(); 
	virtual void Selected ( bool selected ); 
	virtual void OnEvent ( int id ); 
	virtual bool MouseWheel(int params); 
	virtual bool MouseLButton(bool bDown); 
	virtual bool MouseMove(float x, float y); 
	virtual bool KeyClick(int key, int chr); 
	virtual void Focus(bool focus); 
	virtual void Update(float dt); 
 
	// design 
	void SetBGColor ( DWORD clr ); 
	void SetSelectionColor ( DWORD clr ); 
	void SetFrameColor ( DWORD clr ); 
	void SetSepSize ( float sz ); 
 
	// selection work 
	virtual void AddItem ( GUIAppObject *obj ); 
	virtual void DelItem ( GUIAppObject *obj ); 
	virtual void DelItem ( int idx ); 
	virtual int SelectionIndex (); 
	virtual GUIAppObject * SelectionObject (); 
	virtual bool SetSelection ( int idx ); 
	virtual int ItemCount (); 
}; 
 
#endif 


