#ifndef __GUIAPPOBJ_H__ 
#define __GUIAPPOBJ_H__ 

#include "hgegui.h" 

class GUIAppWindow; 
 
class GUIAppObject : public hgeGUIObject { 
protected: 
	GUIAppObject *parentwin; 
	bool isSelected; 
public: 
	GUIAppObject () { 
		parentwin = 0; 
		isSelected = false; 
	} 
 
	virtual void SetParent (GUIAppObject*window) { 
		parentwin = window; 
	} 
	 
	virtual bool TestPoint(float x, float y) { 
		return rect.TestPoint(x,y); 
	} 
 
	virtual void Selected ( bool selected ) { 
		isSelected = selected; 
	} 
 
	virtual void OnEvent ( int id ) { 
	} 
 
	virtual float GetWidth () { 
		return rect.x2 - rect.x1; 
	} 
 
	virtual float GetHeight () { 
		return rect.y2 - rect.y1; 
	} 
 
	virtual void Move (float dx, float dy ) { 
		rect.Set(rect.x1-dx,rect.y1-dy,rect.x2-dx,rect.y2-dy); 
	} 
 
	virtual bool IsSelected () { 
		return isSelected; 
	} 
}; 
 
 
#endif 

