#include "guiedit.h" 
   
GUIAppEdit::GUIAppEdit(int _id, float x, float y, float w,hgeFont* fnt,  DWORD flags, DWORD max_text) {   
    bStatic = false;   
    bVisible = true;   
    bEnabled = true;   
    id=_id;   
   
    m_Font = fnt;   
    m_Focus = false;   

    m_Flags = flags;
   
    m_Time = 0;   
    m_Blink = 0;   
    m_Pos = 0;
    
    m_MaxTextLen = max_text;
    if(m_MaxTextLen != 0)
    {
        m_Text = new char[m_MaxTextLen];
        m_Text[0]=0;
        m_tmp = new char[m_MaxTextLen];
        m_len = new char[m_MaxTextLen];
    }else{
        m_Text = NULL;
        m_tmp = NULL;
        m_len = NULL;
    }
   
    rect.Set(x-2, y-2, x+w+4, y+fnt->GetHeight()*fnt->GetScale()+4);   
    frameColor = ARGB(0xff,0xff,0xff,0xff);   
    textColor = ARGB(0xff,0xff,0xff,0xff);   
}   

GUIAppEdit::~GUIAppEdit()
{
    if (NULL != m_Text)delete []m_Text;
    if (NULL != m_tmp)delete []m_tmp;
    if (NULL != m_len)delete []m_len;
}

bool GUIAppEdit::KeyClick(int key, int chr) {   
    if ( m_Focus ) {   
        int len = strlen(m_Text);   
           
        switch (key) {   
        case HGEK_ENTER:   
            parentwin->OnEvent(id);   
            m_Focus = false;   
            break;   
        case HGEK_TAB:   
            parentwin->OnEvent(id);   
            m_Focus = false;   
            break;   
        case HGEK_LEFT:   
            if (m_Pos > 0) {   
                m_Pos--;   
            }   
            break;   
        case HGEK_RIGHT:   
            if (m_Pos < (int)strlen(m_Text)) {   
                m_Pos++;   
            }   
            break;   
        case HGEK_DELETE:   
            if (m_Pos >= len)break;
            strcpy(&m_Text[m_Pos], &m_Text[m_Pos+1]);
            break;
        case HGEK_BACKSPACE:
            if (m_Pos <= 0)break;
            strcpy(&m_Text[m_Pos-1], &m_Text[m_Pos]);
            m_Pos--;
            break;
        // 不处理的按键
        case HGEK_ESCAPE:

        case HGEK_PGUP:
        case HGEK_PGDN:
        case HGEK_HOME:
        case HGEK_END:
        case HGEK_INSERT:
        case HGEK_GRAVE:
        case HGEK_MINUS:
        case HGEK_EQUALS:
        case  HGEK_BACKSLASH:
        case HGEK_LBRACKET:
        case HGEK_RBRACKET:
        case  HGEK_SEMICOLON:
        case HGEK_APOSTROPHE:
        case  HGEK_COMMA:
        case  HGEK_PERIOD:
        case  HGEK_SLASH:
        case  HGEK_MULTIPLY:
        case  HGEK_DIVIDE:
        case  HGEK_ADD:
        case  HGEK_SUBTRACT:
        case  HGEK_DECIMAL:

        case  HGEK_F1:
        case  HGEK_F2:
        case  HGEK_F3:
        case  HGEK_F4:
        case  HGEK_F5:
        case  HGEK_F6:
        case  HGEK_F7:
        case  HGEK_F8:
        case  HGEK_F9:
        case  HGEK_F10:
        case  HGEK_F11:
        case  HGEK_F12:
            break;
        default:
            if (strlen(m_Text) >= m_MaxTextLen)break;
            if (m_Pos == strlen(m_Text))
            {
                m_Text[m_Pos] = chr;   
                m_Text[len+1] = 0;
            }else{
                strcpy(m_tmp, &m_Text[m_Pos]);
                
                m_Text[m_Pos] = chr;
                strcpy(&m_Text[m_Pos+1],m_tmp);
            }
            m_Pos++;   
            break;
        };   
           
        // keep caret from vanishing    
        m_Blink = true;    
        m_Time = 0;   
    }   
    return false;   
}   
   
void GUIAppEdit::Selected(bool bDown) {   
    m_Focus = bDown;   
    GUIAppObject::Selected(bDown);   
}   
   
void GUIAppEdit::Update(float dt) {   
    if (m_Focus) {   
        m_Time += dt;   
        if (m_Time > 0.3) {   
            m_Time = 0;   
            m_Blink = !m_Blink;   
        }   
    }   
}   
   
void GUIAppEdit::Render() {   
    hge->Gfx_RenderLine(rect.x1, rect.y1, rect.x2, rect.y1, frameColor);   
    hge->Gfx_RenderLine(rect.x2, rect.y1, rect.x2, rect.y2, frameColor);   
    hge->Gfx_RenderLine(rect.x2, rect.y2, rect.x1, rect.y2, frameColor);   
    hge->Gfx_RenderLine(rect.x1, rect.y2, rect.x1, rect.y1, frameColor);   
   
    m_Font->SetColor(textColor);
    if (GUIAPPEDIT_FLAGS_NOMARL == m_Flags)
        {
        m_Font->Render(rect.x1 + 2, rect.y1 + 2,HGETEXT_LEFT, m_Text);  
        }else{
        int len = strlen(m_Text);
        memset(m_tmp,'*',len);
        m_tmp[len]=0;
       m_Font->Render(rect.x1 + 2, rect.y1 + 2,HGETEXT_LEFT, m_tmp);  
        }
   
    if (m_Focus && m_Blink) {   

        

        float p;
        if (GUIAPPEDIT_FLAGS_NOMARL == m_Flags){
            strncpy(m_len, m_Text, m_Pos);
            m_len[m_Pos] = 0;
        }else{
            strncpy(m_len, m_tmp, m_Pos);
            m_len[m_Pos] = 0;
        }
        
        p = m_Font->GetStringWidth(m_len);   
        hge->Gfx_RenderLine(rect.x1 + 2 + p, rect.y1 + 2, rect.x1 + 2 + p, rect.y2 - 2, textColor);   
    }   
}   
   
void GUIAppEdit::Move(float dx, float dy) {   
    rect.Set(rect.x1-dx,rect.y1-dy,rect.x2-dx,rect.y2-dy);   
}  

