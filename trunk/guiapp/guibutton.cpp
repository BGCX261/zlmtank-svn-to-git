#include "hge.h"
#include "guibutton.h"
   
GUIAppButton::GUIAppButton(int _id, float x, float y, const char *file) : GUIAppObject () {   
    id=_id;   
    bStatic=false;   
    bVisible=true;   
    bEnabled=true;   
    bPressed=false;   
    bTrigger=false;   
    inFocus = false;   
    isSelected = false;   
    bPushed = false;
    HTEXTURE texture;
    if (NULL != file)
    {
        texture = hge->Texture_Load(file);   
        if ( texture ) {   
            float w = (float)hge->Texture_GetWidth(texture,true) / 3;   
            float h = (float)hge->Texture_GetHeight(texture,true);   
            rect.Set(x, y, x+w, y+h);   
            sprDef = new hgeSprite(texture, 0, 0, w, h);   
            sprUp = new hgeSprite(texture, w, 0, w, h);   
            sprDown = new hgeSprite(texture, w+w, 0, w, h);   
        }
    }
    else
    {
            float w =20;
            float h = 20;
            texture = hge->Texture_Create(w,h);
            rect.Set(x, y, x+20, y+20);   
            sprDef = new hgeSprite(texture, 0, 0, w, h);   
            sprUp = new hgeSprite(texture, 0, 0, w, h);   
            sprDown = new hgeSprite(texture, 0, 0, w, h);  
    }
}   
   
GUIAppButton::GUIAppButton(int _id, float x, float y, float w, float h, HTEXTURE tex, float tx, float ty) {   
    id=_id;   
    bStatic=false;   
    bVisible=true;   
    bEnabled=true;   
    inFocus = false;   
    isSelected = false;   
    rect.Set(x, y, x+w, y+h);   
       
    bPressed=false;   
    bTrigger=false;   
    sprDef = new hgeSprite(tex, tx, ty, w, h);    
    sprUp = new hgeSprite(tex, tx+w, ty, w, h);   
    sprDown = new hgeSprite(tex, tx+w+w, ty, w, h);   
}   
   
GUIAppButton::~GUIAppButton() {   
    if(sprUp) delete sprUp;   
    if(sprDown) delete sprDown;   
}   
   
void GUIAppButton::SetPos(float x, float y ) {   
    float w = sprDef->GetWidth();   
    float h = sprDef->GetHeight();   
    rect.Set(x, y, x+w, y+h);   
}   
   
float GUIAppButton::GetWidth() {   
    return sprDef->GetWidth();   
}   
   
float GUIAppButton::GetHeight() {   
    return sprDef->GetHeight();   
}   
   
void GUIAppButton::Focus(bool focus) {   
    inFocus = focus;   
    if ( !inFocus) {   
        bPressed = false;   
        bPushed = false;   
    }   
}   
   
bool GUIAppButton::KeyClick(int key, int chr ) {   
    if (( key == HGEK_ENTER || key == HGEK_SPACE )&& isSelected ) {   
        parentwin->OnEvent(id);   
    }   
    return false;   
}   
   
void GUIAppButton::Selected(bool selected) {   
    isSelected = selected;   
    if ( !isSelected) {   
        bPressed = false;   
        bPushed = false;   
    }   
}   
   
void GUIAppButton::Render() {   
    if ( !inFocus && !isSelected ) {   
        sprDef->Render(rect.x1, rect.y1);   
    } else {   
        if ( bPressed ) {   
            sprDown->Render(rect.x1, rect.y1);   
        } else {   
            sprUp->Render(rect.x1, rect.y1);   
        }   
    }   
}   
   
bool GUIAppButton::MouseLButton(bool bDown) {   
    bPushed = bDown;   
    if(bDown) {   
        bOldState=bPressed;    
        bPressed=true;   
        return false;   
    } else {   
        if(bPressed) {   
            parentwin->OnEvent(id);   
        }   
        bPressed=false;   
        return true;    
    }   
}   
   
   
void GUIAppButton::Move(float dx,float dy) {   
    rect.Set(rect.x1-dx,rect.y1-dy,rect.x2-dx,rect.y2-dy);   
}   

