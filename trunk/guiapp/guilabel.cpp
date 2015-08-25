//#include "def.h" 
#include "guilabel.h"   
   
GUIAppLabel::GUIAppLabel(int _id, float x, float y, hgeFont *fnt, char* text, DWORD color, int align) : GUIAppObject () {   
    bStatic=true;   
    bVisible=true;   
    bEnabled=true;   
    id = _id;   
   
    m_Font = fnt;   
    m_Pos.x = x;   
    m_Pos.y = y;   
    m_Color = color;   
       
    m_Align = align;   
   
    SetText(text);   
}   
   
void GUIAppLabel::SetVisible(bool yesno) {   
    bVisible = yesno;   
}   
   
void GUIAppLabel::SetText(int v) {   
    char str[16];   
    itoa(v, str, 10);   
    SetText(str);   
}   
   
void GUIAppLabel::SetText(char* text) {    
    m_Text = text;   
    char *tmp = m_Text.c_str();   
   
    char* p = strchr(tmp, '\n');   
    int lines = 1;   
   
    float w = m_Font->GetStringWidth(tmp);   
   
    while (p++)                 // skip '\n' character    
    {   
        float tw = m_Font->GetStringWidth(p);   
        if (tw > w)  w = tw;   
   
        p = strchr(p, '\n');   
        lines++;   
    }   
   
    float h = m_Font->GetHeight() * m_Font->GetScale() * lines;   
   
    switch (m_Align)   
    {   
    case HGETEXT_LEFT:   
        rect.Set(m_Pos.x, m_Pos.y, m_Pos.x + w, m_Pos.y + h);   
        break;   
   
    case HGETEXT_CENTER:   
        rect.Set(m_Pos.x - w/2, m_Pos.y, m_Pos.x + w/2, m_Pos.y + h);   
        break;   
   
    case HGETEXT_RIGHT:   
        rect.Set(m_Pos.x - w, m_Pos.y, m_Pos.x, m_Pos.y + h);   
        break;   
    }   
}   
   
void GUIAppLabel::SetAlign(int align) {    
    m_Align = align;    
   
    float w = rect.x2 - rect.x1;   
    float h = rect.y2 - rect.y1;   
   
    switch (m_Align)   
    {   
    case HGETEXT_LEFT:   
        rect.Set(m_Pos.x, m_Pos.y, m_Pos.x + w, m_Pos.y + h);   
        break;   
   
    case HGETEXT_CENTER:   
        rect.Set(m_Pos.x - w/2, m_Pos.y, m_Pos.x + w/2, m_Pos.y + h);   
        break;   
   
    case HGETEXT_RIGHT:   
        rect.Set(m_Pos.x - w, m_Pos.y, m_Pos.x, m_Pos.y + h);   
        break;   
    }   
}   
   
void GUIAppLabel::SetColor(DWORD col) {   
    m_Color = col;   
}   
   
void GUIAppLabel::Render() {   
    m_Font->SetColor(m_Color);   
    m_Font->Render(m_Pos.x, m_Pos.y, m_Align, m_Text.c_str());   
}   
   
void GUIAppLabel::Move( float dx, float dy) {   
    m_Pos.x = m_Pos.x - dx;   
    m_Pos.y = m_Pos.y - dy;   
   
    float w = rect.x2 - rect.x1;   
    float h = rect.y2 - rect.y1;   
       
    switch (m_Align)   
    {   
    case HGETEXT_LEFT:   
        rect.Set(m_Pos.x, m_Pos.y, m_Pos.x + w, m_Pos.y + h);   
        break;   
           
    case HGETEXT_CENTER:   
        rect.Set(m_Pos.x - w/2, m_Pos.y, m_Pos.x + w/2, m_Pos.y + h);   
        break;   
           
    case HGETEXT_RIGHT:   
        rect.Set(m_Pos.x - w, m_Pos.y, m_Pos.x, m_Pos.y + h);   
        break;   
    }   
}   
