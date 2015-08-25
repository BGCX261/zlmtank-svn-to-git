#ifndef _SCLOGIN_H
#define _SCLOGIN_H

// window
class LoginWindow : public GUIAppWindow {

	GUIAppButton *button;
	GUIAppEdit *edit1;
	GUIAppEdit *edit2;
	GUIAppLabel *label1;

	hgeFont *font;
	int clickCnt;
public:
	enum {
		GUIAPP_ID_BTN_LOGIN = 10, 
		GUIAPP_ID_EDIT_USERNAME = 20,
		GUIAPP_ID_EDIT_PASSWORD = 40,
		GUIAPP_ID_LABLE_STATUS = 60
	};

	LoginWindow(GUIApp *parentApp, int _id, float x, float y,bool fixed ) ;
    
	virtual void OnEvent (int id);
	virtual void Update(float dt); 
	BOOL SendLoginMsgReq();
	BOOL RecvLoginMsgAck();
};




class scLogin: public scene
{
public:
    GUIApp *m_app;
    LoginWindow *m_wnd;

	
    BOOL init();
    void deinit();
    virtual void PostRender(float d);
};

#endif
