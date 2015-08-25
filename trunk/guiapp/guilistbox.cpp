//#include "def.h"    
#include "guilistbox.h"  
   
GUIAppListBox::GUIAppListBox ( int _id, float x, float y, float w, float h,   
                               const char *btn11, const char *btn12, const char *btn13,    
                               const char *btn21, const char *btn22, const char *btn23) : GUIAppObject () {   
    bStatic=false;   
    bVisible=true;   
    bEnabled=true;   
    id = _id;   
    rect.Set(x,y,x+w,y+h);   
    bgColor = ARGB(0xaf,0xA0,0xA0,0xA0);   
    frameColor = ARGB(0xff,0xFF,0xFF,0xFF);   
    selColor = ARGB(0xaf,0xF0,0xF0,0xF0);   
    sepHeight = 1;   
    oldVValue = 0;   
    oldHValue = 0;   
    selectIndex = 0;   
    // scrollers    
    vscroll = new GUIAppScrollBar(VSCROLL,GUIAppScrollBar::V_SCROLL,x+w,y,h,btn11,btn12,btn13);   
    vscroll->SetParent(this);   
    hscroll = new GUIAppScrollBar(HSCROLL,GUIAppScrollBar::H_SCROLL,x,y+h,w,btn21,btn22,btn23);   
    hscroll->SetParent(this);   
    // moving    
    vscroll->Move(vscroll->GetWidth(),0);   
    hscroll->Move(0,hscroll->GetHeight());   
    // set size    
    SetupSize(w,h);   
    SetupSelection();   
}   
   
GUIAppListBox::~GUIAppListBox () {   
    objects.RemoveAll(true);   
    delete vscroll;   
    delete hscroll;   
}   
   
// implementation    
void GUIAppListBox::Move (float dx, float dy ) {   
    vscroll->Move(dx,dy);   
    hscroll->Move(dx,dy);   
    GUIAppObject::Move(dx,dy);   
    SetupSize(GetWidth(),GetHeight());   
    for ( int a = 0; a < objects.Count(); a ++ ) {   
        objects[a]->Move(dx, dy);   
    }   
    SetupSelection();   
}   
   
void GUIAppListBox::AddItem ( GUIAppObject *obj ) {   
    objects.Add(obj);   
    SetupList ();   
}   
   
void GUIAppListBox::DelItem ( GUIAppObject *obj ) {   
    if ( objects.Find(obj)) {   
        objects.Del(obj);   
    }   
    SetupList ();   
}   
   
void GUIAppListBox::DelItem ( int idx ) {   
    if ( idx >=0 && idx < objects.Count()) {   
        GUIAppObject *obj = objects[idx];   
        objects.Del(obj);   
    }   
    SetupList ();   
}   
   
void GUIAppListBox::SetupSize(float w, float h ) {   
    float x = rect.x1;   
    float y = rect.y1;   
   
    rect.x2 = x + w;   
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
   
void GUIAppListBox::Render() {   
    hge->Gfx_RenderQuad(&background);   
    hge->Gfx_RenderLine(rect.x1, rect.y1, rect.x2, rect.y1, frameColor);   
    hge->Gfx_RenderLine(rect.x2, rect.y1, rect.x2, rect.y2, frameColor);   
    hge->Gfx_RenderLine(rect.x2, rect.y2, rect.x1, rect.y2, frameColor);   
    hge->Gfx_RenderLine(rect.x1, rect.y2, rect.x1, rect.y1, frameColor);   
       
    hge->Gfx_SetClipping((int)rect.x1, (int)rect.y1, clipWidth, clipHeight);   
    // rendering selection    
    hge->Gfx_RenderQuad(&selection);   

    // rendering list
    for ( int a = 0; a <objects.Count(); a ++ ) {
        objects[a]->Render();
    }
    hge->Gfx_SetClipping();   
   
    // rendering scrolls    
    if ( vsVisible) {   
        vscroll->Render();   
    }   
       
    if ( hsVisible ) {   
        hscroll->Render();   
    }   
}   
   
// colors    
void GUIAppListBox::SetBGColor ( DWORD clr ) {   
    bgColor = clr;   
    background.v[0].col = bgColor;   
    background.v[1].col = bgColor;   
    background.v[2].col = bgColor;   
    background.v[3].col = bgColor;   
}   
   
void GUIAppListBox::SetSelectionColor ( DWORD clr ) {   
    selColor = clr;   
    selection.v[0].col = selColor;   
    selection.v[1].col = selColor;   
    selection.v[2].col = selColor;   
    selection.v[3].col = selColor;   
}   
   
void GUIAppListBox::SetFrameColor ( DWORD clr ) {   
}   
   
// selection work    
int GUIAppListBox::SelectionIndex () {   
    return selectIndex;   
}   
   
GUIAppObject * GUIAppListBox::SelectionObject () {   
    return selectObject;   
}   
   
bool GUIAppListBox::SetSelection ( int idx ) {   
    if ( idx < objects.Count() && idx >= 0 ) {   
        selectIndex = idx;   
        selectObject = objects[selectIndex];   
        SetupSelection();   
        FocusSelection();   
        return true;   
    } else {   
        return false;   
    }   
}   
   
int GUIAppListBox::ItemCount () {   
    return objects.Count();   
}   
   
void GUIAppListBox::Update(float dt) {   
    if ( objects.Count()) {   
        objects[selectIndex]->Update(dt);   
    }   
}   
   
void GUIAppListBox::SetupList () {   
    float maxHeight = 0;   
    float maxWidth = 0;   
    for ( int a = 0; a < objects.Count(); a ++ ) {   
        GUIAppObject *obj = objects[a];   
        // moving to 0,0    
        obj->Move(obj->rect.x1, obj->rect.y1);   
        obj->Move(-(rect.x1+sepHeight), -(rect.y1 + maxHeight + sepHeight));   
        maxHeight += obj->GetHeight() + (sepHeight+1);   
        if ( obj->GetWidth() > maxWidth ) {   
            maxWidth = obj->GetWidth();   
        }   
        obj->Selected(false);   
    }   
   
    listWidth = maxWidth;   
    listHeight = maxHeight;   
   
    maxHeight -= GetHeight();   
    maxWidth -= GetWidth();   
   
    clipWidth = (int)GetWidth()-1;   
    clipHeight = (int)GetHeight()-1;   
       
    // scroll states    
    hsVisible = false;   
    vsVisible = false;   
       
    // if scroll is visible    
    if ( listWidth > clipWidth ) {   
        hsVisible = true;   
        float hw = hscroll->GetHeight();   
        clipHeight -= (int)hw;   
        maxHeight += hw;   
    }   
       
    if ( listHeight > clipHeight ) {   
        vsVisible = true;   
        float vw = vscroll->GetWidth();    
        clipWidth -= (int) vw;   
        maxWidth += vw;   
    }   
   
    hscroll->SetLimits(0, maxWidth, 10);   
    vscroll->SetLimits(0, maxHeight, 10 );   
   
    vscroll->SetLenght(GetHeight());   
    hscroll->SetLenght(GetWidth());   
   
    if ( hsVisible && vsVisible ) {   
        vscroll->SetLenght(vscroll->GetHeight()-hscroll->GetHeight());   
        hscroll->SetLenght(hscroll->GetWidth()-vscroll->GetWidth());   
    }   
   
    SetupSelection();   
}   
   
void GUIAppListBox::SetSepSize ( float sz ) {   
    sepHeight = sz;   
}   
   
void GUIAppListBox::SetupSelection() {   
    float x = 0;   
    float y = 0;   
    float w = 0;   
    float h = 0;   
   
    if ( objects.Count()) {   
        for ( int a = 0; a < objects.Count(); a ++ ) {   
            objects[a]->Selected(false);   
        }   
        objects[selectIndex]->Selected(true);   
        x = objects[selectIndex]->rect.x1 - sepHeight;   
        y = objects[selectIndex]->rect.y1 - sepHeight;   
        w = listWidth;   
        h = (objects[selectIndex]->rect.y2 - y) + sepHeight;   
    }   
       
    selection.v[0].x = x;   
    selection.v[0].y = y;   
    selection.v[0].z = 0;   
    selection.v[0].col = selColor;   
    selection.v[0].tx = 0;   
    selection.v[0].ty = 0;   
       
    selection.v[1].x = x + w;   
    selection.v[1].y = y;   
    selection.v[1].z = 0;   
    selection.v[1].col = selColor;   
    selection.v[1].tx = 0;   
    selection.v[1].ty = 0;   
       
    selection.v[2].x = x + w;   
    selection.v[2].y = y + h;   
    selection.v[2].z = 0;   
    selection.v[2].col = selColor;   
    selection.v[2].tx = 0;   
    selection.v[2].ty = 0;   
       
    selection.v[3].x = x;   
    selection.v[3].y = y + h;   
    selection.v[3].z = 0;   
    selection.v[3].col = selColor;   
    selection.v[3].tx = 0;   
    selection.v[3].ty = 0;   
       
    selection.tex = 0;   
    selection.blend = BLEND_ALPHABLEND | BLEND_COLORADD;   
}   
   
void GUIAppListBox::FocusSelection () {   
    if ( objects.Count()) {   
        float sy = objects[selectIndex]->rect.y1 - rect.y1;   
        if ( sy <= sepHeight || sy > (clipHeight-sepHeight) ) {   
            vscroll->SetValue(vscroll->GetValue() + sy);   
            OnEvent(VSCROLL);   
        }   
    }   
}   
   
void GUIAppListBox::Selected ( bool selected ) {   
    if ( selected ) {   
        float x,y;   
        hge->Input_GetMousePos(&x,&y);   
        vscroll->Selected(false);   
        hscroll->Selected(false);   
        if ( vscroll->TestPoint(x,y)) {   
            vscroll->Selected(true);   
            hscroll->Selected(false);   
        }   
        if ( hscroll->TestPoint(x,y)) {   
            hscroll->Selected(true);   
            vscroll->Selected(false);   
        }   
    }   
    if ( objects.Count()) {   
        objects[selectIndex]->Selected(selected);   
    }   
    GUIAppObject::Selected(selected);   
}   
   
void GUIAppListBox::OnEvent ( int id ) {   
    int a = 0;   
    switch ( id ) {   
    case VSCROLL:    
        {   
            float dy = oldVValue - vscroll->GetValue();   
            oldVValue = vscroll->GetValue();   
            for ( a = 0; a < objects.Count(); a ++ ) {   
                objects[a]->Move(0, -dy);   
            }   
            SetupSelection();   
        }   
        break;   
    case HSCROLL:   
        {   
            float dx = oldHValue - hscroll->GetValue();   
            oldHValue = hscroll->GetValue();   
            for ( a = 0; a < objects.Count(); a ++ ) {   
                objects[a]->Move(-dx, 0);   
            }   
            SetupSelection();   
        }   
        break;   
    }   
}   
   
   
bool GUIAppListBox::MouseWheel(int params) {   
    if ( IsSelected()) {   
        if ( hscroll->IsSelected()) {   
            hscroll->MouseWheel(params);   
            return true;   
        }   
        if ( vscroll->IsSelected()) {   
            vscroll->MouseWheel(params);   
            return true;   
        }   
           
        selectIndex -= params;   
           
        if ( selectIndex < 0 ) {   
            selectIndex = 0;   
        }   
           
        if ( selectIndex >= objects.Count()) {   
            selectIndex = objects.Count() - 1;   
        }   
   
        SetupSelection();   
        FocusSelection();   
        return true;   
    } else {   
        return false;   
    }   
}   
   
bool GUIAppListBox::MouseLButton(bool bDown) {   
    float x,y;   
    hge->Input_GetMousePos(&x,&y);   
    if (hscroll->TestPoint(x,y)) {   
        hscroll->Selected(true);   
        hscroll->MouseLButton(bDown);   
        vscroll->Selected(false);   
        if ( objects.Count()) {   
            objects[selectIndex]->Selected(false);   
        }   
        return true;   
    }   
   
    if (vscroll->TestPoint(x,y)) {   
        vscroll->Selected(true);   
        vscroll->MouseLButton(bDown);   
        hscroll->Selected(false);   
        if ( objects.Count()) {   
                objects[selectIndex]->Selected(false);   
        }   
        return true;   
    }   
   
    if ( objects.Count()) {   
        if (objects[selectIndex]->TestPoint(x,y)) {   
            objects[selectIndex]->MouseLButton(bDown);   
            return true;   
        }   
    }   
   
    for ( int a = 0; a < objects.Count(); a ++ ) {   
        hgeRect rect;   
        rect.Set(objects[a]->rect.x1,objects[a]->rect.y1,objects[a]->rect.x1+listWidth,objects[a]->rect.y2);   
        if ( rect.TestPoint(x,y)) {   
            selectIndex = a;   
            SetupSelection();   
            FocusSelection();   
            return true;   
        }   
    }   
   
    return false;   
}   
   
bool GUIAppListBox::MouseMove(float x, float y) {   
    if ( vscroll->IsSelected()) {   
        return vscroll->MouseMove(x,y);   
    }   
    if ( hscroll->IsSelected()) {   
        return hscroll->MouseMove(x,y);   
    }   
   
    if ( objects.Count()) {   
        if (objects[selectIndex]->TestPoint(x,y)) {   
            objects[selectIndex]->MouseMove(x,y);   
            return true;   
        }   
    }   
   
    return false;   
}   
   
void GUIAppListBox::Focus(bool focus) {   
    if ( focus ) {   
        float x,y;   
        hge->Input_GetMousePos(&x,&y);   
        // vscroll    
        if ( vscroll->TestPoint(x,y)) {   
            vscroll->Focus(true);   
        } else {   
            vscroll->Focus(false);   
        }   
        // hscroll    
        if ( hscroll->TestPoint(x,y)) {   
            hscroll->Focus(true);   
        } else {   
            hscroll->Focus(false);   
        }   
           
        if ( objects.Count()) {   
            if ( objects[selectIndex]->TestPoint(x,y)) {   
                objects[selectIndex]->Focus(true);   
            } else {   
                objects[selectIndex]->Focus(false);   
            }   
        }   
    } else {   
        vscroll->Focus(false);   
        hscroll->Focus(false);   
    }   
}   
   
bool GUIAppListBox::KeyClick(int key, int chr) {   
    if ( IsSelected()) {   
        if ( vscroll->IsSelected()) {   
            return vscroll->KeyClick(key,chr);   
        }   
        if ( hscroll->IsSelected()) {   
            return hscroll->KeyClick(key,chr);   
        }   
   
        int oldIndex = selectIndex;   
   
        // setting selection    
        if ( key == HGEK_UP ) {   
            selectIndex --;   
        }   
   
        if ( key == HGEK_DOWN ) {   
            selectIndex ++;   
        }   
   
        if ( key == HGEK_PGUP ) {   
            selectIndex = 0;   
        }   
   
        if ( key == HGEK_PGDN ) {   
            if ( objects.Count()) {   
                selectIndex = objects.Count() - 1;;   
            } else {   
                selectIndex = 0;   
            }   
        }   
   
        if ( selectIndex < 0 ) {   
            if ( objects.Count()) {   
                selectIndex = objects.Count() - 1;;   
            } else {   
                selectIndex = 0;   
            }   
        }   
           
        if ( selectIndex >= objects.Count()) {   
            selectIndex = 0;   
        }   
           
        SetupSelection();   
        FocusSelection();   
   
        if ( oldIndex == selectIndex ) {   
            if ( objects.Count()) {   
                objects[selectIndex]->KeyClick(key,chr);   
            }   
        }   
        return true;   
    } else {   
        return false;   
    }   
}   

