//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef C_INS_GAMERULES_OBJ_H
#define C_INS_GAMERULES_OBJ_H
#ifdef _WIN32
#pragma once
#endif

#include "imc_format.h"

//=========================================================
//=========================================================
class C_INSObjective : public C_BaseEntity
{
	DECLARE_CLASS( C_INSObjective, C_BaseEntity );
	DECLARE_CLIENTCLASS( );

public:
	C_INSObjective( );
	~C_INSObjective( );

	static void LevelInit( void );
	static C_INSObjective *GetObjective( int iID );

	static const CUtlVector< C_INSObjective* > &GetObjectiveList( void );

	void OnDataChanged( DataUpdateType_t Type );

	void SetName( const char *pszName );
	void SetPhonetic( int iPhonetic ) { m_iPhonetic = iPhonetic; }
	void SetColor( const Color clrColor );
	void SetPlayersRequired( int iCount ) { m_iPlayersRequired = iCount; }
	void SetPlayersCapturing( int iCount ) { m_iPlayersCapturing = iCount; }
	void SetCaptureProgress( int iValue ) { m_iCaptureProgress = iValue; }

	const Vector &GetMarker( void ) const { return m_vecOrigin; }
	int GetOrderID( void ) const { return m_iOrderID; }
	int GetRadius( void ) const { return m_iRadius; }
	int GetChaseRadius( void ) const { return m_iRadius; }
	const char *GetName( void ) const { return m_szName; }
	int GetPhonetic( void ) const { return m_iPhonetic; }
	const char *GetPhonetischName( void ) const;
	char GetPhonetischLetter( void ) const;
	const Color &GetColor( void ) const { return m_clrColor; }
	const Color &GetWhitedColor( void ) const { return m_clrWhitedColor; }
	int GetCapturedTeam( void ) const { return m_iCapturedTeam; }
	int GetCaptureTeam(void) const { return m_iCaptureTeam; }
	int GetPlayersRequired( void ) const { return m_iPlayersRequired; }
	int GetPlayersCapturing( void ) const { return m_iPlayersCapturing; }
	bool IsCapturing( void ) const;
	int GetCaptureProgress( void ) const { return m_iCaptureProgress; }
	bool IsOrdersAllowed( void ) const { return m_bOrdersAllowed; }

	bool IsPlayerInside( C_BasePlayer *pPlayer );

	const Vector &GetChaseCamOrigin( void ) { return GetMarker( ); }
	int	GetChaseCamDistance( void ) { return GetChaseRadius( ); }

private:
	Vector m_vecOrigin;
	int m_iOrderID;
	int m_iRadius;
	int m_iChaseRadius;
	char m_szName[ MAX_OBJNAME_LENGTH ];
	int m_iPhonetic;
	Color m_clrColor;
	Color m_clrWhitedColor;

	int m_iCapturedTeam;
	int m_iCaptureTeam;

	int m_iPlayersRequired;
	int m_iPlayersCapturing;

	int m_iCaptureProgress;

	bool m_bOrdersAllowed;
};

#endif // C_INS_GAMERULES_OBJ_H
