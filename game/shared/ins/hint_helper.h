//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef HINT_HELPER_H
#define HINT_HELPER_H

#ifdef _WIN32
#pragma once
#endif

class CINSPlayer;

//=========================================================
//=========================================================
enum HintTypes_t
{
	HINT_INVALID = -1,

	// control hints
	HINT_SNIPER,
	HINT_IRONSIGHTS,
	HINT_GRENADE,

	// capture hints
	HINT_CAPTURE_CAPTURING,
	HINT_CAPTURE_DEFENDING,
	HINT_CAPTURE_NEP,
	HINT_CAPTURE_BLOCK,
	HINT_CAPTURE_CAPTURED,
	HINT_CAPTURE_CANNOT,

	// other
	HINT_NOCOMMANDER,
	HINT_INITIALORDERS,
	HINT_NKILLCONF,
	HINT_FHIT,
	HINT_TKPUNISH,

	// misc entities
	HINT_SPAWNPROTECTION,
	HINT_SNIPERZONE,
	HINT_MINEFIELD,
	HINT_STANCEALT,

	HINT_COUNT
};

//=========================================================
//=========================================================
#define HINT_MAXLENGTH 64

//=========================================================
//=========================================================
class IHint
{
public:
	virtual const char *Text( void ) const = 0;
	virtual const char *TextAdditional( void ) const { return NULL; }

	virtual const char *Binding( void ) const { return NULL; }

	virtual bool IgnoreClientHide( void ) const { return false; }
	virtual bool ClientOnly( void ) const { return false; }
};

typedef IHint *( *HintCreator_t )( void );
extern HintCreator_t g_HintCreators[ HINT_COUNT ];

#define DECLARE_HINT( id, classname ) \
	IHint *CreateHint__##id( void ) { \
	return new classname; } \
class CHintHelper__##id { \
public: \
	CHintHelper__##id( ) { \
	g_HintCreators[ id ] = CreateHint__##id; } \
} g_HintHelper__##id;

//=========================================================
//=========================================================
class CHintHelperBase
{
public:
	CHintHelperBase( );

	void Init( void );

	static bool IsValidHint( int iHintID );
	IHint *GetHint( int iHintID ) const;

protected:
	IHint *m_pHintData[ HINT_COUNT ];
};

//=========================================================
//=========================================================
#ifdef GAME_DLL

class CHintHelper : public CHintHelperBase, public CAutoGameSystem
{
public:
	void SendHint( CINSPlayer *pPlayer, int iHintID );

private:
	bool Init( void );

	void LevelInitPreEntity( void );

private:
	bool m_bSentHints[ HINT_COUNT ][ MAX_PLAYERS + 1 ];
};

#else

//=========================================================
//=========================================================
class IHintListener
{
public:
	IHintListener( );
	virtual ~IHintListener( );

	virtual void OnHint( int iHintID ) = 0;
};

//=========================================================
//=========================================================
class CHintHelper : public CHintHelperBase, public CAutoGameSystem
{
public:
	CHintHelper( );

	void ShowHint( int iHintID );

	bool Init( void );

private:
	IHintListener *m_pListener;
};

#endif 

//=========================================================
//=========================================================
extern CHintHelper g_HintHelper;

#endif // HINT_HELPER_H