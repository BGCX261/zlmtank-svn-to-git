#ifndef __GUIAPPEDIT_H__ 
#define __GUIAPPEDIT_H__ 
 
#include "guiobj.h"
#include "hge.h" 
#include "hgefont.h" 
 #define GUIAPPEDIT_MAX_TEXT_LEN  1024

#define GUIAPPEDIT_FLAGS_NOMARL       0
#define GUIAPPEDIT_FLAGS_PASSWORD  1
 
class GUIAppEdit: public GUIAppObject { 
public: 
	GUIAppEdit(int _id, float x, float y, float w,hgeFont* fnt,  DWORD flags, DWORD max_text); 
       ~GUIAppEdit();
	char*			GetText(){return m_Text;}; 
	virtual void	Selected(bool bDown); 
	virtual bool	KeyClick(int key, int chr); 
	virtual void	Update(float dt); 
	virtual void	Move(float dx, float dy ); 
	void			Render(); 
	 
	void			SetTextColor(DWORD clr) { 
		textColor = clr; 
	} 
 
	void			SetFrameColor(DWORD clr) { 
		frameColor = clr; 
	} 
 
protected: 
	DWORD			textColor; 
	DWORD			frameColor; 
	hgeFont*		m_Font; 
	bool			m_Focus; 
	DWORD				m_Pos;
	char			*m_Text; //text
	char                *m_tmp;//*
	char                *m_len;   //|
	DWORD          m_Flags;
	DWORD          m_MaxTextLen;
	float			m_Time; 
	bool			m_Blink; 
}; 
 
#endif 


