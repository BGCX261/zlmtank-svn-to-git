//#include "def.h"  
#include "guiwindow.h"  
   
void GUIAppWindow::SetupSize(float x, float y, float w, float h ) {   
    rect.x1 = x;   
    rect.x2 = x + w;   
    rect.y1 = y;   
    rect.y2 = y + h;   
       
    windowBg.v[0].x = x;   
    windowBg.v[0].y = y;   
    windowBg.v[0].z = 0;   
    windowBg.v[0].col = bgColor;   
    windowBg.v[0].tx = 0;   
    windowBg.v[0].ty = 0;   
       
    windowBg.v[1].x = x + w;   
    windowBg.v[1].y = y;   
    windowBg.v[1].z = 0;   
    windowBg.v[1].col = bgColor;   
    windowBg.v[1].tx = 0;   
    windowBg.v[1].ty = 0;   
       
    windowBg.v[2].x = x + w;   
    windowBg.v[2].y = y + h;   
    windowBg.v[2].z = 0;   
    windowBg.v[2].col = bgColor;   
    windowBg.v[2].tx = 0;   
    windowBg.v[2].ty = 0;   
       
    windowBg.v[3].x = x;   
    windowBg.v[3].y = y + h;   
    windowBg.v[3].z = 0;   
    windowBg.v[3].col = bgColor;   
    windowBg.v[3].tx = 0;   
    windowBg.v[3].ty = 0;   
       
    windowBg.tex = 0;   
    windowBg.blend = BLEND_ALPHABLEND | BLEND_COLORMUL | BLEND_ZWRITE;   
    curSelection = -1;   
}   
   
void GUIAppWindow::SetBGColor ( DWORD clr ) {   
    bgColor = clr;   
    windowBg.v[0].col = bgColor;   
    windowBg.v[1].col = bgColor;   
    windowBg.v[2].col = bgColor;   
    windowBg.v[3].col = bgColor;   
}   
   
void GUIAppWindow::SetFrameColor ( DWORD clr ) {   
    frameColor = clr;   
}   
   
GUIAppWindow::GUIAppWindow( GUIApp *parent, int _id, float x, float y, float w, float h ) : GUIAppObject() {   
    parentapp = parent;   
    bStatic = false;   
    bVisible = true;   
    bEnabled = true;   
    id=_id;   
    isSelected = false;   
    bgColor = ARGB(0xaf,0xA0,0xA0,0xA0);   
    frameColor = ARGB(0xff,0xFF,0xFF,0xFF);   
    SetupSize(x,y,w,h);   
    wWidth = w;   
    wHeight = h;   
    ox = 0;   
    oy = 0;   
   
}   
   
void GUIAppWindow::SelectNext() {   
    int a, start = 0;   
    if ( curSelection >= 0 ) {   
        start = curSelection;   
    }   
       
    if (curSelection == objects.Count()-1) {   
        start = 0;   
    }   
   
    for ( a = start; a < objects.Count(); a ++ ) {   
        objects[a]->Selected(false);   
    }   
   
    for ( a = start; a < objects.Count(); a ++ ) {   
        if ( !objects[a]->bStatic && objects[a]->bVisible && a != curSelection ) {   
            int id = objects[a]->id;   
            objects[a]->Selected(true);   
            objects[a]->Focus(true);   
            curSelection = a;   
            break;   
        }   
    }   
}   
   
void GUIAppWindow::AddCtrl ( GUIAppObject *obj ) {   
    // moving to window context    
    obj->Move(-rect.x1, -rect.y1);   
    objects.Add(obj);   
   
    // adding objkect into parent GUI    
    obj->SetParent(this);   
   
    // make selection    
    if ( curSelection < 0 && !obj->bStatic ) {   
        curSelection = objects.Count() -1 ;   
    }   
}   
   
void GUIAppWindow::Render() {   
    hge->Gfx_RenderQuad(&windowBg);   
    hge->Gfx_RenderLine(rect.x1, rect.y1, rect.x2, rect.y1, frameColor);   
    hge->Gfx_RenderLine(rect.x2, rect.y1, rect.x2, rect.y2, frameColor);   
    hge->Gfx_RenderLine(rect.x2, rect.y2, rect.x1, rect.y2, frameColor);   
    hge->Gfx_RenderLine(rect.x1, rect.y2, rect.x1, rect.y1, frameColor);   
   
    for ( int a = 0; a < objects.Count(); a ++ ) {   
        if ( objects[a]->bVisible ) {   
            objects[a]->Render();   
        }   
    }   
}   
   
void GUIAppWindow::Move ( float dx, float dy ) {   
    SetupSize(rect.x1-dx, rect.y1-dy,wWidth,wHeight);   
}   
   
void GUIAppWindow::MoveObjects(float dx, float dy ) {   
    for ( int a = 0; a < objects.Count(); a ++ ) {   
        objects[a]->Move(dx,dy);   
    }   
}   
   
void GUIAppWindow::Update(float dt) {   
    for ( int a = 0; a < objects.Count(); a ++ ) {   
        objects[a]->Update(dt);   
    }   
}   
   
bool GUIAppWindow::MouseMove(float x, float y) {   
    if ( bVisible ) {   
        for ( int a = 0; a < objects.Count(); a ++ ) {   
            objects[a]->MouseMove(x+rect.x1,y+rect.y1);   
        }   
        CheckObjectFocus();   
        if ( isSelected ) {   
            float dx = ox - x;   
            float dy = oy - y;   
            Move(dx,dy);   
            MoveObjects (dx,dy);   
            return false;   
        } else {   
            ox = x;   
            oy = y;   
            return false;   
        }   
    } else {   
        return false;   
    }   
}   
   
void GUIAppWindow::SelectObject ( int id ) {   
    for ( int a = 0; a < objects.Count(); a ++ ) {   
        if (objects[a]->id == id ) {   
            objects[a]->Selected(true);   
        } else {   
            objects[a]->Selected(false);   
        }   
    }   
}   
   
void GUIAppWindow::OnTop() {   
    float inc = 0.001f;   
    windowBg.v[0].z += inc;   
    windowBg.v[1].z += inc;   
    windowBg.v[2].z += inc;   
    windowBg.v[3].z += inc;   
}   
   
bool GUIAppWindow::MouseLButton(bool bDown) {   
    if ( bVisible ) {   
        OnTop ();   
        isSelected = bDown;   
        float x,y;   
        hge->Input_GetMousePos(&x,&y);   
        for ( int a = 0; a < objects.Count(); a ++ ) {   
            if (objects[a]->TestPoint(x,y)) {   
                if (bDown) {   
                    SelectObject(objects[a]->id);   
                    curSelection = a;   
                }   
                objects[a]->MouseLButton(bDown);   
                if (!objects[a]->bStatic) {   
                    isSelected = false;   
                }   
            }   
        }   
        CheckObjectFocus();   
    }   
    return false;   
}   
   
bool GUIAppWindow::MouseRButton(bool bDown) {   
    if ( bVisible ) {   
        OnTop ();   
        isSelected = bDown;   
        float x,y;   
        hge->Input_GetMousePos(&x,&y);   
        for ( int a = 0; a < objects.Count(); a ++ ) {   
            if (objects[a]->TestPoint(x,y)) {   
                if (bDown) {   
                    SelectObject(objects[a]->id);   
                    curSelection = a;   
                }   
                objects[a]->MouseRButton(bDown);   
                if (!objects[a]->bStatic) {   
                    isSelected = false;   
                }   
            }   
        }   
        CheckObjectFocus();   
    }   
    return false;   
}   
   
void GUIAppWindow::CheckObjectFocus () {   
    float x,y;   
    hge->Input_GetMousePos(&x,&y);   
    for ( int a = 0; a < objects.Count(); a ++ ) {   
        if (objects[a]->TestPoint(x,y)) {   
            objects[a]->Focus(true);   
        } else {   
            objects[a]->Focus(false);   
        }   
    }   
}   
   
void GUIAppWindow::Focus(bool bFocused) {   
    if ( bVisible ) {   
        CheckObjectFocus();   
    }   
}   
   
void GUIAppWindow::MouseOver(bool bOver) {   
    if ( bVisible ) {   
        float x,y;   
        hge->Input_GetMousePos(&x,&y);   
        for ( int a = 0; a < objects.Count(); a ++ ) {   
            if (!objects[a]->TestPoint(x,y)) {   
                objects[a]->MouseOver(bOver);   
            }   
        }   
    }   
}   
   
bool GUIAppWindow::MouseWheel(int nNotches) {   
    if ( bVisible ) {   
        for ( int a = 0; a < objects.Count(); a ++ ) {   
            objects[a]->MouseWheel(nNotches);   
        }   
    }   
    return false;   
}   
   
bool GUIAppWindow::KeyClick(int key, int chr) {   
    if ( bVisible ) {   
        for ( int a = 0; a < objects.Count(); a ++ ) {   
            objects[a]->KeyClick(key,chr);   
        }   
        if ( key == HGEK_TAB ) {   
            SelectNext();   
        }   
    }   
    return false;   
} 
void GUIAppWindow::OnEvent ( int obj_id )
{

}
