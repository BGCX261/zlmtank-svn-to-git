#ifndef __GUIAPPBUTTON_H__ 
#define __GUIAPPBUTTON_H__ 
#include "hge.h"
#include "guiobj.h" 
 
class GUIAppButton : public GUIAppObject { 
public: 
	GUIAppButton(int id, float x, float y, const char *file); 
	GUIAppButton(int id, float x, float y, float w, float h, HTEXTURE tex, float tx, float ty); 
	virtual			~GUIAppButton(); 
	 
	void			SetMode(bool _bTrigger) { bTrigger=_bTrigger; } 
	void			SetState(bool _bPressed) { bPressed=_bPressed; } 
	bool			GetState() const { return bPushed; } 
	float			GetWidth(); 
	float			GetHeight(); 
	void			SetPos(float x, float y ); 
	virtual void	Render(); 
	virtual void	Move(float dx, float dy); 
	virtual bool	MouseLButton(bool bDown); 
	virtual void	Focus(bool focus); 
	virtual void	Selected(bool focus); 
	virtual bool	KeyClick(int key, int chr ); 
private: 
	bool			bTrigger; 
	bool			bPressed; 
	bool			bPushed; 
	bool			bOldState; 
	bool			inFocus; 
	hgeSprite		*sprDef; 
	hgeSprite		*sprUp; 
	hgeSprite		*sprDown; 
}; 
 
#endif 


