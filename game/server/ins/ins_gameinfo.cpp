//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "ins_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
class CINSGameInfo : public CBaseEntity, public IGameEventListener2
{
	DECLARE_CLASS( CINSGameInfo, CBaseEntity );
	DECLARE_DATADESC( );

private:
	void Activate( void );

	void FireGameEvent( IGameEvent *pEvent );

	void InputForceWin( inputdata_t &inputdata );

private:
	COutputEvent m_OnRoundReset;
};

//=========================================================
//=========================================================
LINK_ENTITY_TO_CLASS( ins_gameinfo, CINSGameInfo );

//=========================================================
//=========================================================
BEGIN_DATADESC( CINSGameInfo )

	DEFINE_OUTPUT( m_OnRoundReset, "OnRoundReset" ),

	DEFINE_INPUTFUNC( FIELD_INTEGER, "ForceWin", InputForceWin ),

END_DATADESC( )

//=========================================================
//=========================================================
void CINSGameInfo::Activate( void )
{
	BaseClass::Activate( );

	gameeventmanager->AddListener( this, "round_reset", true );
}

//=========================================================
//=========================================================
void CINSGameInfo::FireGameEvent( IGameEvent *pEvent )
{
	m_OnRoundReset.FireOutput( NULL, this );
}

//=========================================================
//=========================================================
void CINSGameInfo::InputForceWin( inputdata_t &inputdata )
{
	CINSRules *pRules = INSRules( );

	if( !pRules || !pRules->IsModeRunning( ) )
		return;

	int iTeamID = inputdata.value.Int( );

	if( iTeamID == 0 )
	{
		CBaseEntity *pActivator = inputdata.pActivator;

		if( pActivator && pActivator->IsPlayer( ) )
			iTeamID = ToINSPlayer( pActivator )->GetTeamID( );
	}

	pRules->RunningMode( )->RoundWon( inputdata.value.Int( ), ENDGAME_WINCONDITION_CUSTOM, inputdata.pActivator, true );
}