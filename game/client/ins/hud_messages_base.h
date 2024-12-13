//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef HUD_BMESSAGES_H
#define HUD_BMESSAGES_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/editablepanel.h>
#include <vgui_controls/textentry.h>
#include "vgui_basepanel.h"

#include "hudelement.h"

#include "ins_richtext.h"
#include "ins_utils.h"

//=========================================================
//=========================================================
namespace vgui
{
	class IScheme;
	class Label;
};

class CHudMessagesLine;

//=========================================================
//=========================================================
#define DEFAULT_LINE_COUNT 8
#define DEFAULT_LINE_SHOWTIME 6.0f
#define DEFAULT_LINE_FADETIME 1.0f

//=========================================================
//=========================================================
class CMessagesColorLookup : public CColorLookup
{
public:
	void FindColor( int iID, Color &StringColor ) const;
};

//=========================================================
//=========================================================
class CHudMessagesBase : public CHudElement, public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CHudMessagesBase, vgui::EditablePanel );

public:
	CHudMessagesBase( const char *pszElementName, const char *pszPanelName );

	virtual void Init( void );
	void LevelInit( const char *pszNewMap );
	void LevelShutdown( void );

	void Reset( void );
	void Clear( void );

	const CColorLookup &ColorLookup( void ) const { return m_ColorLookup; }

	vgui::HFont Font( void ) const { return m_hFont; }

	void IncrementLineCounter( void ) { m_iLineCounter++; }
	int GetLineCounter( void ) const { return m_iLineCounter; }

	virtual float LineShowTime( void ) const { return DEFAULT_LINE_SHOWTIME; }
	virtual float LineFadeTime( void ) const { return DEFAULT_LINE_FADETIME; }

	virtual bool CalculateLineWidth( void ) const { return false; }

protected:
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

	virtual void Update( void );

	void Print( CColoredString &Message );

	virtual CHudMessagesLine *CreateNewLine( void );
	virtual int LineCount( void ) { return DEFAULT_LINE_COUNT; }

	virtual const char *FontName( void ) const = 0;
	virtual bool FontScaled( void ) const { return false; }

	virtual bool DrawBackwards( void ) const { return true; }
	virtual bool LineRightAlign( void ) const { return false; }

private:
	void OnTick( void );

	bool UseTraverseSizing( void ) { return true; }

	bool UpdateLine( CHudMessagesLine *pLine, int iCurrentY );
	void ExpireOldestLine( void );
	CHudMessagesLine *FindUnusedLine( void );

	int FindLineHeight( const CHudMessagesLine *pLine ) const;

protected:
	CUtlVector< CHudMessagesLine* > m_ChatLines;

	int m_iLineCounter;

	CMessagesColorLookup m_ColorLookup;

	vgui::HFont	m_hFont;
	int	m_iFontHeight;
};

//=========================================================
//=========================================================
class CHudMessagesLine : public CINSRichText
{
	DECLARE_CLASS_SIMPLE( CHudMessagesLine, CINSRichText )

public:
	CHudMessagesLine( CHudMessagesBase *pParent );

	virtual void Init( void ) { }

	void ApplySchemeSettings( vgui::IScheme *pScheme );
	void PerformLayout( void );

	void Print( CColoredString &String );

	int GetCount( void ) { return m_iCount; }
	float GetStartTime( void ) { return m_flStartTime; }
	float GetEndTime( void ) { return m_flExpireTime; }
	int LineWidth( void ) const { return m_iLineWidth; }

	bool IsReadyToExpire( void );
	void Expire( void );

	void PerformFadeout( void );

	virtual int GetHeightForce( void ) const { return 0; }
	virtual int GetYGap( void ) const { return 0; }

private:
	CHudMessagesLine( const CHudMessagesLine & );

protected:
	CHudMessagesBase *m_pParent;

	int m_iLineWidth;

private:
	int m_iCount;
	float m_flStartTime, m_flExpireTime;
};

#endif // HUD_BMESSAGES_H
