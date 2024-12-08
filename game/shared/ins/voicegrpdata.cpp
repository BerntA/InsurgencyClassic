//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "voicemgr.h"
#include "keyvalues.h"
#include "utlmap.h"

#include "commander_shared.h"
#include "ins_player_shared.h"
#include "ins_utils.h"
#include "play_team_shared.h"

#ifdef CLIENT_DLL

#include "ins_obj_shared.h"
#include "inshud.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
class CListGroup : public CVoiceGroup
{
public:
	CListGroup( );

protected:
	virtual int Count( void ) const = 0;
	virtual bool IsValid( int iID ) const = 0;
	virtual const char *Name( int iID ) const = 0;

	virtual const char *Data( void ) const = 0;
	virtual int Invalid( void ) const = 0;

	bool FindData( KeyValues *pData, int &iID ) const;

private:
	void InitSounds( void );

	int Play( KeyValues *pData ) const;

private:
	CUtlMap< int, int > m_SoundIDs;
};

//=========================================================
//=========================================================
bool ListGroupLess( const int &iLeft, const int &iRight )
{
	return ( iLeft > iRight );
}

//=========================================================
//=========================================================
CListGroup::CListGroup( )
{
	m_SoundIDs.SetLessFunc( ListGroupLess );
}

//=========================================================
//=========================================================
void CListGroup::InitSounds( void )
{
	for( int i = 0; i < Count( ); i++ )
	{
		if( IsValid( i ) )
			m_SoundIDs.Insert( i, InitSound( Name( i ) ) );
	}
}

//=========================================================
//=========================================================
int CListGroup::Play( KeyValues *pData ) const
{
	int iID;

	if( !FindData( pData, iID ) )
		return INVALID_SOUNDGROUP_LOOKUP;

	int iSoundID = m_SoundIDs.Find( iID );

	if( !m_SoundIDs.IsValidIndex( iSoundID ) )
		return INVALID_SOUNDGROUP_LOOKUP;

	return m_SoundIDs[ iSoundID ];
}

//=========================================================
//=========================================================
bool CListGroup::FindData( KeyValues *pData, int &iID ) const
{
	iID = pData->GetInt( Data( ), Invalid( ) );
	return IsValid( iID );
}

//=========================================================
//=========================================================
class CSingleGroup : public CVoiceGroup
{
public:
	CSingleGroup( );

protected:
	virtual const char *Sound( void ) const = 0;

	virtual int Play( KeyValues *pData ) const;

#ifdef CLIENT_DLL

	virtual const char *Text( void ) const { return NULL; }

	virtual bool Text( char *pszBuffer, int iTeamID, KeyValues *pData );

#endif

private:
	void InitSounds( void );

private:
	int m_iSoundID;
};

//=========================================================
//=========================================================
CSingleGroup::CSingleGroup( )
{
	m_iSoundID = INVALID_SOUNDGROUP_LOOKUP;
}

//=========================================================
//=========================================================
void CSingleGroup::InitSounds( void )
{
	const char *pszSound = Sound( );
	Assert( pszSound );

	m_iSoundID = InitSound( pszSound );
}

//=========================================================
//=========================================================
int CSingleGroup::Play( KeyValues *pData ) const
{
	return m_iSoundID;
}

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

bool CSingleGroup::Text( char *pszBuffer, int iTeamID, KeyValues *pData )
{
	const char *pszText = Text( );

	if( !pszText )
		return false;

	Q_strncpy( pszBuffer, pszText, MAX_VOICE_TEXT );

	return true;
}

#endif

//=========================================================
//=========================================================
class COrderGroup : public CListGroup
{
private:
	int SendType( void ) const { return VSENDTYPE_RADIO; }

	const char *Data( void ) const { return "order"; }
};

//=========================================================
// * VOICEGRP_REINFORCE
//=========================================================

//=========================================================
//=========================================================
class CReinforceGroup : public CSingleGroup
{
private:
	int SendType( void ) const { return VSENDTYPE_SCREAM; }

	const char *Sound( void ) const { return "reinforce"; }

	int Play( KeyValues *pData ) const;

#ifdef CLIENT_DLL

	bool Text( char *pszBuffer, int iTeamID, KeyValues *pData );

	void Action( KeyValues *pData );

#endif
};

DECLARE_VOICEGROUP( VOICEGRP_REINFORCE, CReinforceGroup );

//=========================================================
//=========================================================
int CReinforceGroup::Play( KeyValues *pData ) const
{
	if( pData->GetInt( "rtype" ) != EDRET_DONE )
		return INVALID_SOUNDGROUP_LOOKUP;

	return CSingleGroup::Play( pData );
}

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

bool CReinforceGroup::Text( char *pszBuffer, int iTeamID, KeyValues *pData )
{
	int iEDType = pData->GetInt( "rtype", EDRET_INVALID );

	if( iEDType == EDRET_INVALID )
		return false;

	static const char *pszReturnTypes[ EDRET_COUNT ] = {
		NULL,														// EDRET_INVALID
		"Emergency Reinforcement Called",							// EDRET_DONE
		"Emergency Reinforcements have been Called",				// EDRET_DOING
		"Emergency Reinforcements Depleted",						// EDRET_NONE
		"Emergency Reinforcements not Available at this Time",		// EDRET_NOTATM
		"No Players to Reinforce",									// EDRET_NONEED_PLAYERS
		"Reinforcements are Incoming!"								// EDRET_NONEED_TIME
	};

	Q_strncpy( pszBuffer, pszReturnTypes[ iEDType ], MAX_VOICE_TEXT );

	return true;
}

//=========================================================
//=========================================================
void CReinforceGroup::Action( KeyValues *pData )
{
	if( pData->GetInt( "rtype" ) != EDRET_DONE )
		return;

	GetINSHUDHelper( )->SendEmergencyUpdate( );
}

#endif

//=========================================================
// * VOICEGRP_ORDER_OBJ
//=========================================================

//=========================================================
//=========================================================
class CObjectiveGroup : public COrderGroup
{
private:
#ifdef CLIENT_DLL

	bool Text( char *pszBuffer, int iTeamID, KeyValues *pData );

#endif

	int Count( void ) const { return ORDERTYPE_OBJ_COUNT; }
	bool IsValid( int iID ) const;
	const char *Name( int iID ) const;

	int Invalid( void ) const { return ORDERTYPE_OBJ_NONE; }
};

DECLARE_VOICEGROUP( VOICEGRP_ORDER_OBJ, CObjectiveGroup );

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

bool CObjectiveGroup::Text( char *pszBuffer, int iTeamID, KeyValues *pData )
{
	static const char *pszText[ ORDERTYPE_OBJ_COUNT ] = {
		"",				// ORDERTYPE_OBJ_NONE
		"Attack",		// ORDERTYPE_OBJ_ATTACK
		"Defend"		// ORDERTYPE_OBJ_DEFEND
	};

	int iOrderType, iObjID;
	C_INSObjective *pObjective;

	if( !FindData( pData, iOrderType ) )
		return false;

	iObjID = pData->GetInt( "obj", INVALID_OBJECTIVE );

	if( iObjID == INVALID_OBJECTIVE )
		return false;

	pObjective = C_INSObjective::GetObjective( iObjID );

	if( !pObjective )
		return false;

	Q_snprintf( pszBuffer, MAX_VOICE_TEXT, "%s Objective %s!", pszText[ iOrderType ], pObjective->GetPhonetischName( ) );

	return true;
}

#endif

//=========================================================
//=========================================================
bool CObjectiveGroup::IsValid( int iID ) const
{
	return CObjOrder::IsValidOrder( iID );
}

//=========================================================
//=========================================================
const char *CObjectiveGroup::Name( int iID ) const
{
	static const char *pszCommands[ ORDERTYPE_OBJ_COUNT ] = {
		"",				// ORDERTYPE_OBJ_NONE
		"attack",		// ORDERTYPE_OBJ_ATTACK
		"defend"		// ORDERTYPE_OBJ_DEFEND
	};

	return pszCommands[ iID ];
}

//=========================================================
// * VOICEGRP_ORDER_UNIT
//=========================================================

//=========================================================
//=========================================================
class CUnitGroup : public COrderGroup
{
private:

#ifdef CLIENT_DLL

	bool Text( char *pszBuffer, int iTeamID, KeyValues *pData );

#endif

	int Count( void ) const { return ORDERTYPE_UNIT_COUNT; }
	bool IsValid( int iID ) const;
	const char *Name( int iID ) const;

	int Invalid( void ) const { return ORDERTYPE_UNIT_INVALID; }
};

DECLARE_VOICEGROUP( VOICEGRP_ORDER_UNIT, CUnitGroup );

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

bool CUnitGroup::Text( char *pszBuffer, int iTeamID, KeyValues *pData )
{
	static const char *pszText[ ORDERTYPE_UNIT_COUNT ] = {
		"Secure",		// ORDERTYPE_UNIT_SECURE
		"Move",			// ORDERTYPE_UNIT_MOVE
		"Guard",		// ORDERTYPE_UNIT_GUARD
		"Regroup",		// ORDERTYPE_UNIT_REGROUP
		"Stealth"		// ORDERTYPE_UNIT_STEALTH
	};

	C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer( );

	if( !pPlayer )
		return false;

	int iOrderType;

	if( !FindData( pData, iOrderType ) )
		return false;

	Vector vecPosition = Vector( pData->GetInt( "posx" ), pData->GetInt( "posy" ), pData->GetInt( "posz" ) );
	int iOrientID = UTIL_FindOrientation( pPlayer, vecPosition );

	if( iOrientID == ORIENTATION_INVALID )
		return false;

	Q_snprintf( pszBuffer, MAX_VOICE_TEXT, "%s the Position to the %s", pszText[ iOrderType ], g_pszOrientationNames[ iOrientID ] );

	return true;
}

#endif

//=========================================================
//=========================================================
bool CUnitGroup::IsValid( int iID ) const
{
	return CUnitOrder::IsValidOrder( iID );
}

//=========================================================
//=========================================================
const char *CUnitGroup::Name( int iID ) const
{
	static const char *pszCommands[ ORDERTYPE_UNIT_COUNT ] = {
		"secure",	// ORDERTYPE_UNIT_SECURE
		"move",		// ORDERTYPE_UNIT_MOVE
		"guard",	// ORDERTYPE_UNIT_GUARD
		"regroup",	// ORDERTYPE_UNIT_REGROUP
		"stealth"	// ORDERTYPE_UNIT_STEALTH
	};

	return pszCommands[ iID ];
}

//=========================================================
// * VOICEGRP_ORDER_PLAYER
//=========================================================

//=========================================================
//=========================================================
class CPlayerGroup : public COrderGroup
{
public:
	static const char *Command( int iID );

private:

#ifdef CLIENT_DLL

	bool Text( char *pszBuffer, int iTeamID, KeyValues *pData );

#endif

	int Count( void ) const { return PORDER_COUNT; }
	bool IsValid( int iID ) const;
	const char *Name( int iID ) const;

	int Invalid( void ) const { return INVALID_PORDER; }
};

DECLARE_VOICEGROUP( VOICEGRP_ORDER_PLAYER, CPlayerGroup );

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

bool CPlayerGroup::Text( char *pszBuffer, int iTeamID, KeyValues *pData )
{
	static const char *pszOrders[ PORDER_COUNT ] = {
		"Flank Left!",					// PORDER_FLANKLEFT
		"Flank Right!",					// PORDER_FLANKRIGHT
		"Move!",						// PORDER_MOVING
		"Take Cover!",					// PORDER_TAKECOVER
		"Provide Covering Fire!",		// PORDER_COVERINGFIRE
		"Hold your Fire!",				// PORDER_HOLDFIRE
		"Provide Return Fire!"			// PORDER_RETURNFIRE
	};

	int iOrderID;

	if( !FindData( pData, iOrderID ) )
		return false;

	Q_strncpy( pszBuffer, pszOrders[ iOrderID ], MAX_VOICE_TEXT );

	return true;
}

#endif

//=========================================================
//=========================================================
const char *CPlayerGroup::Command( int iID )
{
	static const char *pszCommands[ PORDER_COUNT ] = {
		"flankleft",	// PORDER_FLANKLEFT
		"flankright",	// PORDER_FLANKRIGHT
		"moving",		// PORDER_MOVING
		"takecover",	// PORDER_TAKECOVER
		"coveringfire",	// PORDER_COVERINGFIRE
		"holdfire",		// PORDER_HOLDFIRE
		"returnfire"	// PORDER_RETURNFIRE
	};

	return pszCommands[ iID ];
}

//=========================================================
//=========================================================
bool CPlayerGroup::IsValid( int iID ) const
{
	return UTIL_ValidPlayerOrder( iID );
}

//=========================================================
//=========================================================
const char *CPlayerGroup::Name( int iID ) const
{
	return Command( iID );
}

//=========================================================
// * VOICEGRP_ORDER_PLAYER_RESPONSES
//=========================================================

//=========================================================
//=========================================================
class CPlayerResponsesGroup : public CVoiceGroup
{
public:
	CPlayerResponsesGroup( );

private:
	void InitSounds( void );

	int SendType( void ) const { return VSENDTYPE_RADIO; }

	int Play( KeyValues *pData ) const;

#ifdef CLIENT_DLL

	bool Text( char *pszBuffer, int iTeamID, KeyValues *pData );

#endif

private:
	int m_iSoundIDs[ PORDER_COUNT ][ PORESPONSE_COUNT ];
};

DECLARE_VOICEGROUP( VOICEGRP_ORDER_PLAYER_RESPONSES, CPlayerResponsesGroup );

//=========================================================
//=========================================================
CPlayerResponsesGroup::CPlayerResponsesGroup( )
{
	memset( m_iSoundIDs, INVALID_SOUNDGROUP_LOOKUP, sizeof( m_iSoundIDs ) );
}

//=========================================================
//=========================================================
void CPlayerResponsesGroup::InitSounds( void )
{
	char szBuffer[ MAX_VOICE_SOUNDSCRIPT ];

	static const char *pszPlayerOrderResponses[ PORESPONSE_COUNT ] = {
		"doing",	// PORESPONSE_DOING
		"done",		// PORESPONSE_DONE
		"cannot"	// PORESPONSE_CANNOT
	};

	for( int i = 0; i < PORDER_COUNT; i++ )
	{
		for( int j = 0; j < PORESPONSE_COUNT; j++ )
		{
			Q_snprintf( szBuffer, MAX_VOICE_SOUNDSCRIPT, "%s.%s", pszPlayerOrderResponses[ j ], CPlayerGroup::Command( i ) );

			m_iSoundIDs[ i ][ j ] = InitSound( szBuffer );
		}
	}
}

//=========================================================
//=========================================================
int CPlayerResponsesGroup::Play( KeyValues *pData ) const
{
	int iOrderType = pData->GetInt( "order", INVALID_PORDER );

	if( !UTIL_ValidPlayerOrder( iOrderType ) )
		return INVALID_SOUNDGROUP_LOOKUP;

	int iResponseType = pData->GetInt( "response", INVALID_PORESPONSE );

	if( !UTIL_ValidPlayerOrderResponse( iResponseType ) )
		return INVALID_SOUNDGROUP_LOOKUP;

	return m_iSoundIDs[ iOrderType ][ iResponseType ];
}

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

bool CPlayerResponsesGroup::Text( char *pszBuffer, int iTeamID, KeyValues *pData )
{
	static const char *pszPlayerResponse[ PORDER_COUNT ][ PORESPONSE_COUNT ] = 
	{
		{ { "Flanking Left!" }, { "In Position!" }, { "Can't get there!" } },					// PORDER_FLANKLEFT
		{ { "Flanking Right!" }, { "In Position!" }, { "Can't get there!" } },					// PORDER_FLANKRIGHT
		{ { "Moving!" }, { "In Position!" }, { "That's suicide!" } },							// PORDER_MOVING
		{ { "Looking for cover!" }, { "In Position!" }, { "I'm not moving!" } },				// PORDER_TAKECOVER
		{ { "Covering fire!" }, { "We have them pinned!" }, { "Can't do that right now!" } },	// PORDER_COVERINGFIRE
		{ { "" }, { "" }, { "" } },																// PORDER_HOLDFIRE
		{ { "" }, { "" }, { "" } },																// PORDER_RETURNFIRE
	};

	int iOrderType = pData->GetInt( "order", INVALID_PORDER );

	if( !UTIL_ValidPlayerOrder( iOrderType ) )
		return false;

	int iResponseType = pData->GetInt( "response", INVALID_PORESPONSE );

	if( !UTIL_ValidPlayerOrderResponse( iResponseType ) )
		return false;

	Q_strncpy( pszBuffer, pszPlayerResponse[ iOrderType ][ iResponseType ], MAX_VOICE_TEXT );

	return true;
}

#endif

//=========================================================
// * VOICEGRP_PLAYER_ACTION
//=========================================================

//=========================================================
//=========================================================
class CActionGroup : public COrderGroup
{
private:
	int SendType( void ) const { return VSENDTYPE_SHOUT; }

	int Count( void ) const { return PACTION_COUNT; }
	bool IsValid( int iID ) const;
	const char *Name( int iID ) const;

	const char *Data( void ) const { return "action"; }
	int Invalid( void ) const { return INVALID_PACTION; }
};

DECLARE_VOICEGROUP( VOICEGRP_PLAYER_ACTION, CActionGroup );

//=========================================================
//=========================================================
bool CActionGroup::IsValid( int iID ) const
{
	return CINSPlayer::IsValidAction( iID );
}

//=========================================================
//=========================================================
const char *CActionGroup::Name( int iID ) const
{
	static const char *pszCommands[ PACTION_COUNT ] = {
		"reloading",	// PACTION_RELOADING
		"hit",			// PACTION_HIT
		"outofammo",	// PACTION_OUTOFAMMO
		"fragout",		// PACTION_FRAGOUT
		"mandown"		// PACTION_MANDOWN
	};

	return pszCommands[ iID ];
}

//=========================================================
// * VOICEGRP_PLAYER_STATUS
//=========================================================

//=========================================================
//=========================================================
class CStatusGroup : public CVoiceGroup
{
public:
	CStatusGroup( );

private:
	void InitSounds( void );

	int SendType( void ) const { return VSENDTYPE_RADIO; }

	int Play( KeyValues *pData ) const;

#ifdef CLIENT_DLL

	bool Text( char *pszBuffer, int iTeamID, KeyValues *pData );

#endif

private:
	bool FindData( KeyValues *pData, int &iType, int &iID ) const;

private:
	CUtlMap< int, int > m_SoundIDs[ PSTATUSTYPE_COUNT ];
};

DECLARE_VOICEGROUP( VOICEGRP_PLAYER_STATUS, CStatusGroup );

//=========================================================
//=========================================================
CStatusGroup::CStatusGroup( )
{
	for( int i = 0; i < PSTATUSTYPE_COUNT; i++ )
		m_SoundIDs[ i ].SetLessFunc( ListGroupLess );
}

//=========================================================
//=========================================================
void CStatusGroup::InitSounds( void )
{
	for( int i = 0; i < PSTATUSTYPE_COUNT; i++ )
	{
		const CUtlMap< int, StatusProgressiveData_t > &Data = UTIL_StatusProgressive( i )->Data( );

		for( int j = Data.FirstInorder( ); j != Data.InvalidIndex( ); j = Data.NextInorder( j ) )
			m_SoundIDs[ i ].Insert( j, InitSound( Data[ j ].pszSound ) );
	}
}

//=========================================================
//=========================================================
int CStatusGroup::Play( KeyValues *pData ) const
{
	int iType, iID;

	if( !FindData( pData, iType, iID ) )
		return INVALID_SOUNDGROUP_LOOKUP;

	const CUtlMap< int, int > &SoundIDs = m_SoundIDs[ iType ];

	int iSoundID = SoundIDs.Find( iID );

	if( !SoundIDs.IsValidIndex( iSoundID ) )
		return INVALID_SOUNDGROUP_LOOKUP;

	return SoundIDs[ iSoundID ];
}

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

bool CStatusGroup::Text( char *pszBuffer, int iTeamID, KeyValues *pData )
{
	int iType, iID;

	if( !FindData( pData, iType, iID ) )
		return false;

	const StatusProgressiveData_t *pStatusData = UTIL_StatusProgressiveData( iType, iID );

	if( !pStatusData )
		return false;

	Q_strncpy( pszBuffer, pStatusData->pszProgressiveText, MAX_VOICE_TEXT );

	return true;
}

#endif

//=========================================================
//=========================================================
bool CStatusGroup::FindData( KeyValues *pData, int &iType, int &iID ) const
{
	iType = pData->GetInt( "type", INVALID_PSTATUSTYPE );
	iID = pData->GetInt( "id", INVALID_PSTATUSID );

	return ( iType != INVALID_PSTATUSTYPE && iID != INVALID_PSTATUSID );
}

//=========================================================
// * VOICEGRP_PLAYER_HELP
//=========================================================

//=========================================================
//=========================================================
class CHelpGroup : public CSingleGroup
{
private:
	int SendType( void ) const { return VSENDTYPE_SCREAM; }

	const char *Sound( void ) const { return "help"; }

#ifdef CLIENT_DLL

	bool Text( char *pszBuffer, int iTeamID, KeyValues *pData );

#endif
};

DECLARE_VOICEGROUP( VOICEGRP_PLAYER_HELP, CHelpGroup );

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

bool CHelpGroup::Text( char *pszBuffer, int iTeamID, KeyValues *pData )
{
	C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer( );

	if( !pPlayer )
		return false;

	Q_strncpy( pszBuffer, "Need Assistance", MAX_VOICE_TEXT );

	CINSObjective *pObjective = pPlayer->GetCurrentObj( );

	if( pObjective )
	{
		char szObjBuffer[ MAX_VOICE_TEXT ];
		Q_snprintf( szObjBuffer, sizeof( szObjBuffer ), " at Objective %s", pObjective->GetPhonetischName( ) );

		Q_strncat( pszBuffer, szObjBuffer, MAX_VOICE_TEXT );
	}
	else if( pData )
	{
		const char *pszArea = pData->GetString( "area" );

		if( !pszArea )
		{
			char szAreaBuffer[ MAX_VOICE_TEXT ];
			Q_snprintf( szAreaBuffer, sizeof( szAreaBuffer ), " at the %s", pszArea );

			Q_strncat( pszBuffer, szAreaBuffer, MAX_VOICE_TEXT );
		}
	}

	Q_strncat( pszBuffer, "!", MAX_VOICE_TEXT );

	return true;
}

#endif

//=========================================================
// * VOICEGRP_PLAYER_HELP
//=========================================================

//=========================================================
//=========================================================
class CMoveoutGroup : public CSingleGroup
{
private:
	int SendType( void ) const { return VSENDTYPE_RADIO; }

	const char *Sound( void ) const { return "moveout"; }

#ifdef CLIENT_DLL

	const char *Text( void ) const { return NULL; }

#endif
};

DECLARE_VOICEGROUP( VOICEGRP_MOVEOUT, CMoveoutGroup );