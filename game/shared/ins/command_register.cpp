//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//=============================================================================//

#include "cbase.h"
#include "command_register.h"
#include "ins_player_shared.h"
#include "weapon_ballistic_base.h"
#include "ins_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
bool UTIL_IsValidCommandRegister( int iID )
{
	return ( iID >= 0 && iID < CMDREGISTER_COUNT );
}

//=========================================================
//=========================================================
#ifdef GAME_DLL

//=========================================================
//=========================================================
void UTIL_UpdateCommandRegister( CINSPlayer *pPlayer, int iID, int iValue )
{
	if( !pPlayer )
		return;

	if( !UTIL_IsValidCommandRegister( iID ) )
	{
		Assert( false );
		return;
	}

	pPlayer->SetCmdValue( iID, iValue );
}

//=========================================================
//=========================================================
#else

//=========================================================
//=========================================================
void UTIL_SendRegisterUpdate( int iCallbackID, int iValue )
{
	char szUpdate[ 64 ];
	Q_snprintf( szUpdate, sizeof( szUpdate ), "cmdr %i %i", iCallbackID, iValue );

	engine->ServerCmd( szUpdate );
}

//=========================================================
//=========================================================
void UTIL_UpdateCommandRegisters( void )
{
	for( int i = 0; i < CMDREGISTER_COUNT; i++ )
		UTIL_SendRegisterUpdate( i, UTIL_CommandRegisterValue( i ) );
}

//=========================================================
//=========================================================
class CRegisteredConVar : public ConVar
{
public:
	CRegisteredConVar( int iID, const char *pszCommand, const char *pszDefaultValue, const char *pszHelp, int iMaxValue, FnChangeCallback_t Callback );
};

//=========================================================
//=========================================================
CRegisteredConVar *g_pCommandRegisters[ CMDREGISTER_COUNT ];

//=========================================================
//=========================================================
CRegisteredConVar::CRegisteredConVar( int iID, const char *pszCommand, const char *pszDefaultValue, const char *pszHelp, int iMaxValue, FnChangeCallback_t Callback )
	: ConVar( pszCommand, pszDefaultValue, FCVAR_ARCHIVE, pszHelp, true, 0, true, iMaxValue, Callback )
{
	g_pCommandRegisters[ iID ] = this;
}

//=========================================================
//=========================================================
#define DEFINE_CMDREGISTER( cmd, name, defaultvalue, help, maxvalue ) \
	void name##__Callback( IConVar* pConVar, char const* pOldString, float flOldValue ) { \
		C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer( ); \
		if( !pPlayer ) { return; } \
		ConVar* pVar = (ConVar*)pConVar; \
		UTIL_SendRegisterUpdate( cmd, pVar->GetInt( ) ); \
		pPlayer->SetCmdValue( cmd, pVar->GetInt( ) ); \
	} \
	CRegisteredConVar name##( cmd, "cl_" # name, #defaultvalue, #help, maxvalue, name##__Callback );

//=========================================================
//=========================================================
DEFINE_CMDREGISTER( CMDREGISTER_DEATHINFO, deathinfo, 2, "Type of Death Information Shown", DEATHINFOTYPE_COUNT - 1 );
DEFINE_CMDREGISTER( CMDREGISTER_PFIREMODE, pfiremode, 0, "Set Perferred Firemode", FIREMODE_COUNT + 1 );
DEFINE_CMDREGISTER( CMDREGISTER_STANCE, stancemode, 0, "Set your Stance Mode", 1 );
DEFINE_CMDREGISTER( CMDREGISTER_IRONSIGHTHOLD, ironsighthold, 0, "Holding Ironsights instead of a Toggle", 1 );
DEFINE_CMDREGISTER( CMDREGISTER_3DSCOPE, fastscopes, 1, "Toggle 3D Scopes", 1 );
DEFINE_CMDREGISTER( CMDREGISTER_HIDEHINTS, hidehints, 0, "Hide the Hints on the HUD", 1 );
DEFINE_CMDREGISTER( CMDREGISTER_SWITCHDRAW, switchthrow, 1, "Switch to Best Weapon after Redraw", 0 );

//=========================================================
//=========================================================
void UTIL_SetCommandRegister( int iID, int iValue )
{
	if( !UTIL_IsValidCommandRegister( iID ) )
	{
		Assert( false );
		return;
	}

	ConVar *pCommandRegister = UTIL_CommandRegisterCommand( iID );
	pCommandRegister->SetValue( iValue );
}

//=========================================================
//=========================================================
ConVar *UTIL_CommandRegisterCommand( int iID )
{
	Assert( g_pCommandRegisters[ iID ] );
	return g_pCommandRegisters[ iID ];
}

//=========================================================
//=========================================================
int UTIL_CommandRegisterValue( int iID )
{
	ConVar *pCommandRegister = UTIL_CommandRegisterCommand( iID );
	return pCommandRegister->GetInt( );
}

#endif