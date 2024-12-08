#ifndef _INS_WATERMARK_H_
#define _INS_WATERMARK_H_

class CINSWatermarkPanel : public vgui::Panel {
    DECLARE_CLASS_SIMPLE( CINSWatermarkPanel, vgui::Panel );

public:
    CINSWatermarkPanel();
    ~CINSWatermarkPanel();

    virtual void		OnCommand( const char *command );
    virtual void		Activate();
    virtual void		ApplySchemeSettings( vgui::IScheme *pScheme );
    virtual void		OnThink();

protected:
    virtual void UpdateUserInfo();

private:
    vgui::ImagePanel   *m_pImgINSLogo;
    vgui::Label        *m_pLblVersionInfo;
    vgui::Label        *m_pLblUserInfo;

    wchar_t            *m_szSteamID;
};

#endif