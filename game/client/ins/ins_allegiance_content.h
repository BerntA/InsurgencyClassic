// Insurgency Team (C) 2007
// First revision

#ifndef INS_ALLEGIANCE_CONTENT_H
#define INS_ALLEGIANCE_CONTENT_H

#ifdef _WIN32
#pragma once
#endif

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

#endif // INS_ALLEGIANCE_CONTENT_H