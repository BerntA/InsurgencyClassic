#include <game/client/iviewport.h>
#include <vgui_controls/panel.h>
#include <vgui_controls/Button.h>

#ifndef INS_PANEL_H
#define INS_PANEL_H

class CINSPanel : public vgui::Panel, public IViewPortPanel
{
	DECLARE_CLASS_SIMPLE(CINSPanel, Panel);
	
	const char* m_pszImage;
	const char* m_pszImageArmed;
	const char* m_pszImageDepressed;
	const char* m_pszImageDisabled;
	
	int m_iImage;
	int m_iImageArmed;
	int m_iImageDepressed;
	int m_iImageDisabled;

	vgui::Button* m_pNext;

protected:
	char m_szBackground[MAX_PATH];
	int m_iBackground;

	int m_iX;
	int m_iY;
	int m_iS1;
	int m_iS2;

	bool m_bDrawBG;

public:
	enum { SZ_SQSIZE=1024 };

	CINSPanel(Panel *parent, const char *panelName, const char* pszINSVGUIname, bool bPopup=true);

	virtual void PerformLayout();

	virtual void Activate();

	virtual const char *GetName( void );
	virtual void SetData(KeyValues *data);
	virtual void Reset( void );
	virtual void Update( void );
	virtual bool NeedsUpdate( void );
	virtual bool HasInputElements( void );

	virtual void ShowPanel( bool state );

	virtual vgui::VPANEL GetVPanel( void );
	virtual bool IsVisible();
	virtual void SetParent( vgui::VPANEL parent );

	void EnableNextButton( bool bEnable );

	virtual void PaintBackground();

	inline int IntegerScale(int iSize) { return (iSize*m_iS1)>>m_iS2; };
	void SetScaledBounds(Panel* pPanel, int X, int Y, int W, int H);
};

#define CONTROL_SIZE(name,x,y,w,h) \
	enum { \
		SZ_##name##_OFFSET_X=x, \
		SZ_##name##_OFFSET_Y=y, \
		SZ_##name##_SIZE_X=w, \
		SZ_##name##_SIZE_Y=h \
	}

#define SCALE_CONTROL(panel,name) SetScaledBounds(panel,SZ_##name##_OFFSET_X,SZ_##name##_OFFSET_Y,SZ_##name##_SIZE_X,SZ_##name##_SIZE_Y)

#endif //INS_PANEL_H