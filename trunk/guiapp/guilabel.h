#ifndef __GUIAPPLABEL_H__ 
#define __GUIAPPLABEL_H__ 
 
#include "guiobj.h"
#include "hge.h" 
#include "hgefont.h" 
#include "hgevector.h" 
#include "m_str.h"

class GUIAppLabel : public GUIAppObject { 
public: 
	GUIAppLabel(int id, float x, float y, hgeFont *fnt, char* text, DWORD color = 0xFFFFFFFF, int align = HGETEXT_LEFT); 
 
	void SetAlign(int align); 
	void SetVisible(bool yesno); 
	void SetText(char *text); 
	void SetText(int value); 
	void SetColor(DWORD col); 
	virtual void Render(); 
	virtual void Move( float dx, float dy); 
 
private: 
	hgeFont*		m_Font; 
	hgeVector		m_Pos; 
	int				m_Align; 
	TStr			m_Text; 
	DWORD			m_Color; 
}; 
 
#endif 



