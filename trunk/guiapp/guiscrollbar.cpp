//#include "def.h"
#include "guiscrollbar.h"


GUIAppScrollBar::GUIAppScrollBar(int _id, ScrollDir dir, float sx, float sy, float l, const char *btn1f, const char *btn2f, const char *btn3f ) : GUIAppObject() {   
    bStatic=false;   
    bVisible=true;   
    bEnabled=true;   
       
    oldValue = 0;   
    firstMove = true;   
    id = _id;   
       
    lenght = l;   
    x = sx;   
    y = sy;   
    direction = dir;   
       
    btn1 = new GUIAppButton(B_LEFTUP,x,y,btn1f);   
    btn1->SetParent(this);   
    btn2 = new GUIAppButton(B_RIGHTDN,x,y,btn2f);   
    btn2->SetParent(this);   
    btn3 = new GUIAppButton(B_SCROLL,x,y,btn3f);   
    btn3->SetParent(this);   
       
    curValue = 0;   
    minValue = 0;   
    maxValue = 1;   
    stepValue = 3;   
   
    bgColor = ARGB(0xA0,0xff,0xff,0xff);   
    frameColor = ARGB(0xFF,0xff,0xff,0xff);   
       
    switch (dir) {   
        case V_SCROLL:   
            InitV();   
            break;   
        case H_SCROLL:   
            InitH();   
            break;   
        default:   
            break;   
    }   
}   
   
void GUIAppScrollBar::SetLenght ( float l ) {   
    lenght = l;   
    switch (direction) {   
    case V_SCROLL:   
        InitV();   
        break;   
    case H_SCROLL:   
        InitH();   
        break;   
    default:   
        break;   
    }   
}   
   
GUIAppScrollBar::~GUIAppScrollBar() {   
    delete btn1;   
    delete btn2;   
    delete btn3;   
}   
   
void GUIAppScrollBar::SetBGColor ( DWORD clr ) {   
    bgColor = clr;   
    background.v[0].col = bgColor;   
    background.v[1].col = bgColor;   
    background.v[2].col = bgColor;   
    background.v[3].col = bgColor;   
}   
   
void GUIAppScrollBar::SetFrameColor ( DWORD clr ) {   
    frameColor = clr;   
}   
   
float GUIAppScrollBar::Value2Pos( float value ) {   
    float btnWidth;   
    if ( direction == H_SCROLL ) {   
        btnWidth = btn1->GetWidth();   
    } else {   
        btnWidth = btn1->GetHeight();   
    }   
    float rLenght = lenght - btnWidth*3;   
    float vLenght = maxValue - minValue;   
    float coef = vLenght/rLenght;   
    float pos = value/coef+btnWidth;   
    return pos;   
}   
   
float GUIAppScrollBar::Pos2Value( float pos ) {   
    float btnWidth;   
    if ( direction == H_SCROLL ) {   
        btnWidth = btn1->GetWidth();   
    } else {   
        btnWidth = btn1->GetHeight();   
    }   
    float rLenght = lenght - btnWidth*3;   
    float vLenght = maxValue - minValue;   
    float coef = vLenght/rLenght;   
    float value = (pos-btnWidth)*coef;   
    return value;   
}   
   
void GUIAppScrollBar::InitH() {   
    float width = lenght;   
    float height = btn1->GetHeight();   
    SetupSize(x,y,width,height);   
   
    btn1->SetPos(rect.x1,rect.y1);   
    btn2->SetPos(rect.x1,rect.y1);   
    btn3->SetPos(rect.x1,rect.y1);   
   
    btn1->Move(0,0);   
    btn2->Move(-(lenght-btn1->GetWidth()), 0);   
    btn3->Move(-Value2Pos(curValue), 0);   
}   
   
void GUIAppScrollBar::InitV() {   
    float width = btn1->GetHeight();   
    float height = lenght;   
    SetupSize(x,y,width,height);   
   
    btn1->SetPos(rect.x1,rect.y1);   
    btn2->SetPos(rect.x1,rect.y1);   
    btn3->SetPos(rect.x1,rect.y1);   
       
    btn1->Move(0,0);   
    btn2->Move(0, -(lenght-btn1->GetWidth()));   
    btn3->Move(0, -Value2Pos(curValue));   
}   
   
void GUIAppScrollBar::SetupSize(float x, float y, float w, float h ) {   
    rect.x1 = x;   
    rect.x2 = x + w;   
    rect.y1 = y;   
    rect.y2 = y + h;   
       
    background.v[0].x = x;   
    background.v[0].y = y;   
    background.v[0].z = 0;   
    background.v[0].col = bgColor;   
    background.v[0].tx = 0;   
    background.v[0].ty = 0;   
       
    background.v[1].x = x + w;   
    background.v[1].y = y;   
    background.v[1].z = 0;   
    background.v[1].col = bgColor;   
    background.v[1].tx = 0;   
    background.v[1].ty = 0;   
       
    background.v[2].x = x + w;   
    background.v[2].y = y + h;   
    background.v[2].z = 0;   
    background.v[2].col = bgColor;   
    background.v[2].tx = 0;   
    background.v[2].ty = 0;   
       
    background.v[3].x = x;   
    background.v[3].y = y + h;   
    background.v[3].z = 0;   
    background.v[3].col = bgColor;   
    background.v[3].tx = 0;   
    background.v[3].ty = 0;   
       
    background.tex = 0;   
    background.blend = BLEND_ALPHABLEND | BLEND_COLORMUL | BLEND_ZWRITE;   
}   
   
void GUIAppScrollBar::Render() {   
    hge->Gfx_RenderQuad(&background);   
    hge->Gfx_RenderLine(rect.x1, rect.y1, rect.x2, rect.y1, frameColor);   
    hge->Gfx_RenderLine(rect.x2, rect.y1, rect.x2, rect.y2, frameColor);   
    hge->Gfx_RenderLine(rect.x2, rect.y2, rect.x1, rect.y2, frameColor);   
    hge->Gfx_RenderLine(rect.x1, rect.y2, rect.x1, rect.y1, frameColor);   
    btn1->Render();   
    btn2->Render();   
    btn3->Render();   
}   
   
void GUIAppScrollBar::Move(float dx, float dy) {   
    x -= dx;   
    y -= dy;   
    rect.Set(rect.x1-dx,rect.y1-dy,rect.x2-dx,rect.y2-dy);   
    SetupSize(rect.x1, rect.y1,rect.x2-rect.x1,rect.y2-rect.y1);   
    btn1->Move(dx,dy);   
    btn2->Move(dx,dy);   
    btn3->Move(dx,dy);   
}   
   
bool GUIAppScrollBar::MouseMove(float x, float y) {   
    if ( btn3->GetState()) {   
        if ( direction == H_SCROLL ) {   
            float dx;   
            if ( firstMove ) {   
                dx = 0;   
                firstMove = false;   
            } else {   
                dx = oldValue - x;   
            }   
            if ( dx != 0 ) {   
                float cPos = btn3->rect.x1 - dx;   
                   
                if ( cPos < rect.x1 + btn1->GetWidth() ) {   
                    dx = btn3->rect.x1 - (rect.x1+btn1->GetWidth());   
                }   
                   
                if ( cPos > rect.x2 - btn1->GetWidth()*2) {   
                    dx = (rect.x2-btn1->GetWidth()*2) - btn3->rect.x1;   
                }   
   
                btn3->Move(dx,0);   
                curValue = Pos2Value(btn3->rect.x1-rect.x1);   
                parentwin->OnEvent(id);   
            }   
            oldValue = x;   
        } else {   
            float dy;   
            if ( firstMove ) {   
                dy = 0;   
                firstMove = false;   
            } else {   
                dy = oldValue - y;   
            }   
            if ( dy != 0 ) {   
                float cPos = btn3->rect.y1 - dy;   
                   
                if ( cPos < rect.y1 + btn1->GetHeight() ) {   
                    dy = btn3->rect.y1 - (rect.y1 + btn1->GetHeight());   
                }   
                   
                if ( cPos > rect.y2 - btn1->GetHeight()*2) {   
                    dy = (rect.y2-btn1->GetHeight()*2) - btn3->rect.y1;   
                }   
                   
                btn3->Move(0,dy);   
                curValue = Pos2Value(btn3->rect.y1-rect.y1);   
                parentwin->OnEvent(id);   
            }   
            oldValue = y;   
        }   
    } else {   
        firstMove = true;   
    }   
    return false;   
}   
   
bool GUIAppScrollBar::MouseLButton(bool bDown){   
    float x,y;   
    hge->Input_GetMousePos(&x,&y);   
    if ( btn1->TestPoint(x,y)) {   
        btn1->MouseLButton(bDown);   
    }   
    if ( btn2->TestPoint(x,y)) {   
        btn2->MouseLButton(bDown);   
    }   
    if ( btn3->TestPoint(x,y)) {   
        btn3->MouseLButton(bDown);   
    }   
    return false;   
}   
   
void GUIAppScrollBar::Focus(bool focus) {   
    btn3->Focus(focus);   
    if ( !focus ) {   
        btn1->Focus(false);   
        btn2->Focus(false);   
    } else {   
        float x,y;   
        hge->Input_GetMousePos(&x,&y);   
        bool changed = false;   
        if ( btn1->TestPoint(x,y)) {   
            btn1->Focus(true);   
            btn2->Focus(false);   
            btn3->Focus(false);   
            changed = true;   
        }   
        if ( btn2->TestPoint(x,y)) {   
            btn2->Focus(true);   
            btn1->Focus(false);   
            btn3->Focus(false);   
            changed = true;   
        }   
        if ( btn3->TestPoint(x,y)) {   
            btn3->Focus(true);   
            btn1->Focus(false);   
            btn2->Focus(false);   
            changed = true;   
        }   
        if ( !changed) {   
            btn3->Focus(false);   
            btn1->Focus(false);   
            btn2->Focus(false);   
        }   
    }   
}   
   
void GUIAppScrollBar::Selected(bool focus) {   
    btn3->Selected(focus);   
    if ( !focus ) {   
        btn1->Selected(false);   
        btn2->Selected(false);   
    } else {   
        float x,y;   
        hge->Input_GetMousePos(&x,&y);   
        if ( btn1->TestPoint(x,y)) {   
            btn1->Selected(focus);   
            btn2->Selected(false);   
            btn3->Selected(false);   
        }   
        if ( btn2->TestPoint(x,y)) {   
            btn2->Selected(focus);   
            btn1->Selected(false);   
            btn3->Selected(false);   
        }   
        if ( btn3->TestPoint(x,y)) {   
            btn3->Selected(focus);   
            btn1->Selected(false);   
            btn2->Selected(false);   
        }   
    }   
    GUIAppObject::Selected(focus);   
}   
   
bool GUIAppScrollBar::KeyClick(int key, int chr ) {   
    if ( IsSelected()) {   
        btn1->KeyClick(key,chr);   
        btn2->KeyClick(key,chr);   
        btn3->KeyClick(key,chr);   
           
        if ( key == HGEK_UP || key == HGEK_LEFT ) {   
            SetValue(curValue-stepValue);   
            parentwin->OnEvent(id);   
        }   
           
        if ( key == HGEK_DOWN || key == HGEK_RIGHT ) {   
            SetValue(curValue+stepValue);   
            parentwin->OnEvent(id);   
        }   
        return true;   
    } else {   
        return false;   
    }   
}   
   
void GUIAppScrollBar::SetLimits ( float min, float max, float step ) {   
    minValue = min;   
    maxValue = max;   
    stepValue = step;   
}   
   
void GUIAppScrollBar::OnEvent( int event ) {   
    switch ( event ) {   
    case B_LEFTUP:   
        SetValue(curValue-stepValue);   
        parentwin->OnEvent(id);   
        break;   
    case B_RIGHTDN:   
        SetValue(curValue+stepValue);   
        parentwin->OnEvent(id);   
        break;   
    case B_SCROLL:   
        parentwin->OnEvent(id);   
        break;   
    default:   
        break;   
    }   
}   
   
float GUIAppScrollBar::GetValue () {   
    return curValue;   
}   
   
float GUIAppScrollBar::GetMinValue () {   
    return minValue;   
}   
   
float GUIAppScrollBar::GetMaxValue () {   
    return maxValue;   
}   
   
void GUIAppScrollBar::SetValue ( float v ) {   
    if ( v > maxValue ) {   
        v = maxValue;   
    }   
   
    if ( v < minValue ) {   
        v = minValue;   
    }   
   
    curValue = v;   
    float cPos = Value2Pos(curValue);   
       
    switch (direction) {   
    case V_SCROLL:   
        btn3->SetPos(x,y);   
        btn3->Move(0, -cPos);   
        break;   
    case H_SCROLL:   
        btn3->SetPos(x,y);   
        btn3->Move(-cPos,0);   
        break;   
    default:   
        break;   
    }   
}   
   
bool GUIAppScrollBar::MouseWheel(int param) {   
    if ( isSelected ) {   
        if ( direction == V_SCROLL ) {   
            param = -param;   
        }   
        if ( param > 0 ) {   
            curValue += stepValue;   
        } else {   
            curValue -= stepValue;   
        }   
        SetValue(curValue);   
        parentwin->OnEvent(id);   
    }   
   
    return false;   
}   

