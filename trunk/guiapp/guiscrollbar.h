#ifndef __GUIAPPSCROLLBAR_H__ 
#define __GUIAPPSCROLLBAR_H__ 
 
 #include "guibutton.h"
 
class GUIAppScrollBar : public GUIAppObject { 
protected: 
	void RenderV (); 
	void RenderH (); 
	void InitV (); 
	void InitH (); 
	void SetupSize(float x, float y, float w, float h); 
	float Value2Pos( float value ); 
	float Pos2Value( float pos ); 
public: 
	enum ScrollDir { 
		V_SCROLL, H_SCROLL 
	}; 
 
	enum ScrollBtn { 
		B_LEFTUP, B_RIGHTDN, B_SCROLL 
	}; 
 
       GUIAppScrollBar(int _id, ScrollDir dir, float sx, float sy, float l, const char *btn1f, const char *btn2f, const char *btn3f );
	virtual			~GUIAppScrollBar(); 
	 
	virtual void	Render(); 
	virtual void	Move(float dx, float dy); 
	virtual bool	MouseLButton(bool bDown); 
	virtual bool	MouseMove(float x, float y); 
	virtual void	Focus(bool focus); 
	virtual void	Selected(bool focus); 
	virtual bool	KeyClick(int key, int chr ); 
	virtual void	OnEvent( int event ); 
	virtual bool	MouseWheel(int params); 
 
	void SetLimits ( float min, float max, float step = 1.0f ); 
	void SetBGColor (DWORD clr); 
	void SetFrameColor (DWORD clr); 
	 
	float GetValue (); 
	float GetMinValue (); 
	float GetMaxValue (); 
	void SetValue ( float v ); 
	void SetLenght ( float lenght ); 
 
private: 
	ScrollDir		direction; 
	GUIAppButton	*btn1; 
	GUIAppButton	*btn2; 
	GUIAppButton	*btn3; 
	hgeQuad			background; 
	DWORD			frameColor; 
	DWORD			bgColor; 
	// movement 
	float			oldValue; 
	bool			firstMove; 
	// limits 
	float			x, y; 
	float			lenght; 
	float			minValue; 
	float			maxValue; 
	float			curValue; 
	float			stepValue; 
}; 
 
#endif 


