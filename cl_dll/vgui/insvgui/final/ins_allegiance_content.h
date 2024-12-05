// Insurgency Team (C) 2007
// First revision

#pragma once

Panel *CreateMiniscoreBoardPanel( int iTeam, Panel *pParent, const char *pszPanelName );

class SquadButton : public ImageButton
{
public:
	DECLARE_CLASS_SIMPLE(SquadButton, ImageButton);

	SquadButton( const char* pszImage, const char* pszImageArmed, const char* pszImageDepressed, const char* pszImageDisabled, Panel *parent, const char *panelName, Panel *pActionSignalTarget=NULL, const char *pCmd=NULL );

public:

	virtual void OnCursorEntered( void );
	virtual void OnCursorExited( void );

	virtual void SetParentPanels( Panel *pScoreBoards[2], Panel *pSituation, Panel *pDescriptionPanel );

private:

	void SetVisibleContent( bool bState );

	Panel *m_pScoreboards[2]; // To hide them

	Panel *m_pSituationPanel;
	Panel *m_pDescriptionPanel;

};