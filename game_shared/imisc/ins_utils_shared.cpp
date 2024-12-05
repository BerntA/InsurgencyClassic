//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "ins_player_shared.h"
#include "ins_utils.h"
#include "ins_gamerules.h"
#include "ins_obj_shared.h"
#include "commander_shared.h"
#include "weapondef.h"

#ifdef CLIENT_DLL

#include "c_playerresource.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
const Vector &UTIL_PlayerViewMins( int iStance )
{
	switch( iStance )
	{
		case STANCE_STAND:
			return VEC_HULL_MIN;

		case STANCE_CROUCH:
			return VEC_DUCK_HULL_MIN;

		case STANCE_PRONE:
			return VEC_PRONE_HULL_MIN;
	}

	Assert( false );

	return VEC_HULL_MIN;
}

//=========================================================
//=========================================================
const Vector &UTIL_PlayerViewMaxs( int iStance )
{
	switch( iStance )
	{
		case STANCE_STAND:
			return VEC_HULL_MAX;

		case STANCE_CROUCH:
			return VEC_DUCK_HULL_MAX;

		case STANCE_PRONE:
			return VEC_PRONE_HULL_MAX;
	}

	Assert( false );

	return VEC_HULL_MAX;
}

//=========================================================
//=========================================================
const Vector &UTIL_PlayerViewMins( const CINSPlayer *pPlayer )
{
	if( pPlayer->IsObserver( ) )
		return VEC_OBS_HULL_MIN;	

	return UTIL_PlayerViewMins( pPlayer->GetCurrentStance( ) );
}

//=========================================================
//=========================================================
const Vector &UTIL_PlayerViewMaxs( const CINSPlayer *pPlayer )
{
	if( pPlayer->IsObserver( ) )
		return VEC_OBS_HULL_MAX;	

	return UTIL_PlayerViewMaxs( pPlayer->GetCurrentStance( ) );
}

//=========================================================
//=========================================================
Vector UTIL_PlayerViewOffset( CINSPlayer *pPlayer, int iStance )
{
	switch( iStance )
	{
		case STANCE_STAND:
		{
			Vector vecView = VEC_VIEW;

			if( pPlayer->GetPlayerFlags( ) & FL_PLAYER_BIPOD )
				vecView.z += pPlayer->ViewOffsetBipod( );

			return vecView;
		}

		case STANCE_CROUCH:
		{
			Vector vecCrouchView = VEC_DUCK_VIEW;

			if( pPlayer->GetPlayerFlags( ) & FL_PLAYER_IRONSIGHTS )
				vecCrouchView.z += PLAYER_VOFFSET_CIRONSIGHT;

			if( pPlayer->GetPlayerFlags( ) & FL_PLAYER_BIPOD )
				vecCrouchView.z += pPlayer->ViewOffsetBipod( );

			return vecCrouchView;
		}

		case STANCE_PRONE:
		{
			Vector vecProneView, vecForward;
			int iProneForward;
			
			vecProneView = VEC_PRONE_VIEW;
			iProneForward = INSRules( )->GetProneForward( );

			QAngle angAngles = pPlayer->EyeAngles( );
			angAngles[ PITCH ] = angAngles[ ROLL ] = 0.0f;

			AngleVectors( angAngles, &vecForward );

			VectorMA( vecProneView, iProneForward, vecForward, vecProneView );

			return vecProneView;
		}
	}

	return VEC_VIEW;
}

//=========================================================
//=========================================================
Vector UTIL_ViewModelStanceOffset( int iStance )
{
	switch( iStance )
	{
		case STANCE_CROUCH:
		{
			return Vector( GetWeaponDef( )->flVMOffsetCrouchX,
				GetWeaponDef( )->flVMOffsetCrouchY,
				GetWeaponDef( )->flVMOffsetCrouchZ );
		}

		case STANCE_PRONE:
		{
			return Vector( GetWeaponDef( )->flVMOffsetProneX,
				GetWeaponDef( )->flVMOffsetProneY,
				GetWeaponDef( )->flVMOffsetProneZ );
		}
	}

	return vec3_origin;
}

//=========================================================
//=========================================================
Vector UTIL_ViewModelMovingOffset( int iStance )
{
	switch( iStance )
	{
		case STANCE_STAND:
		{
			return Vector( GetWeaponDef( )->flVMOffsetStandMoveX,
				GetWeaponDef( )->flVMOffsetStandMoveY,
				GetWeaponDef( )->flVMOffsetStandMoveZ );
		}

		case STANCE_CROUCH:
		{
			return Vector( GetWeaponDef( )->flVMOffsetCrouchMoveX,
				GetWeaponDef( )->flVMOffsetCrouchMoveY,
				GetWeaponDef( )->flVMOffsetCrouchMoveZ );
		}
	}

	return vec3_origin;
}

//=========================================================
//=========================================================
int UTIL_GetNextStance( int iStanceFrom, int iStanceTo )
{
	int iStanceDiff = iStanceFrom - iStanceTo;

	if( abs( iStanceDiff ) > 1 )
		return iStanceFrom + ( ( iStanceDiff < 0 ) ? 1 : -1 );

	return iStanceTo;
}


//=========================================================
//=========================================================
float UTIL_StanceTransitionTime( int iStanceFrom, int iStanceTo )
{
	switch( iStanceFrom )
	{
		case STANCE_STAND:
		{
			return STIME_STAND_TO_CROUCH;
		}

		case STANCE_CROUCH:
		{
			if( iStanceTo == STANCE_STAND )
			{
				return STIME_CROUCH_TO_STAND;
			}
			else
			{
				Assert( iStanceTo == STANCE_PRONE );
				return STIME_CROUCH_TO_PRONE;
			}

			break;
		}

		case STANCE_PRONE:
		{
			return STIME_PRONE_TO_CROUCH;
		}
	}

	return 0.0f;
}

//=========================================================
//=========================================================
float UTIL_ProneThreshold( int iStance )
{
	switch( iStance )
	{
		case STANCE_STAND:
			return PTIME_STAND;

		case STANCE_CROUCH:
			return PTIME_CROUCH;
	}

	return 0.0f;
}

//=========================================================
//=========================================================
bool UTIL_IsMoving( const Vector &vecVelocity )
{
	return ( ( fabs( vecVelocity.x ) >= 1.0f ) || ( fabs( vecVelocity.y ) >= 1.0f ) );
}

//=========================================================
//=========================================================
bool UTIL_IsMoving( const Vector &vecVelocity, int iMinSpeed )
{
	return ( Vector2DLength( vecVelocity.AsVector2D( ) ) >= iMinSpeed );
}

//=========================================================
//=========================================================
float UTIL_3DCaculateDistance( const CBaseEntity *pEntity, CBasePlayer *pPlayer )
{
	return UTIL_3DCaculateDistance( pEntity->GetAbsOrigin( ), pPlayer );
}

float UTIL_2DCaculateDistance( const CBaseEntity *pEntity, CBasePlayer *pPlayer )
{
	return UTIL_2DCaculateDistance( pEntity->GetAbsOrigin( ), pPlayer );
}

float UTIL_3DCaculateDistance( const Vector &vecOrigin, CBasePlayer *pPlayer )
{
	Vector vecPlayerOrigin = pPlayer->GetAbsOrigin( );

	float flX = vecOrigin.x - vecPlayerOrigin.x;
	float flY = vecOrigin.y - vecPlayerOrigin.y;
	float flZ = vecOrigin.z - vecPlayerOrigin.z;

	return abs( sqrt( ( flX * flX ) + ( flY * flY ) + ( flZ * flZ ) ) );
}

float UTIL_2DCaculateDistance( const Vector &vecOrigin, CBasePlayer *pPlayer )
{
	Vector vecPlayerOrigin = pPlayer->GetAbsOrigin( );

	float flX = vecOrigin.x - vecPlayerOrigin.x;
	float flY = vecOrigin.y - vecPlayerOrigin.y;

	return abs( sqrt( ( flX * flX ) + ( flY * flY ) ) );
}

//=========================================================
//=========================================================
bool UTIL_ValidObjectiveID( int iID )
{
	return ( iID >= 0 && iID < MAX_OBJECTIVES );
}

//=========================================================
//=========================================================
int UTIL_CaculateObjType( int iTeamID, int iObjID )
{
	CINSObjective *pObjective = NULL;

#ifdef GAME_DLL

	pObjective = INSRules( )->GetObjective( iObjID );

#else

	pObjective = C_INSObjective::GetObjective( iObjID );

#endif

	return UTIL_CaculateObjType( iTeamID, pObjective );
}

//=========================================================
//=========================================================
int UTIL_CaculateObjType( int iTeamID, const CINSObjective *pObjective )
{
	if( !pObjective )
		return ORDERTYPE_OBJ_NONE;

	return ( ( pObjective->GetCapturedTeam( ) != iTeamID ) ? ORDERTYPE_OBJ_ATTACK : ORDERTYPE_OBJ_DEFEND );
}

//=========================================================
//=========================================================
int UTIL_CaculateObjType( const CINSPlayer *pPlayer, const CINSObjective *pObjective )
{
	if( !pPlayer )
		return ORDERTYPE_OBJ_NONE;

	return UTIL_CaculateObjType( pPlayer->GetTeamID( ), pObjective );
}

//=========================================================
//=========================================================
IColoredString::IColoredString( )
{
	Reset( );
}

//=========================================================
//=========================================================
void IColoredString::Reset( )
{
	m_szString[ 0 ] = '\0';
}

//=========================================================
//=========================================================
int IColoredString::AddString( const char *pszMessage )
{
	if( !pszMessage )
		return 0;

	int iLength = Q_strlen( pszMessage );

	if( iLength == 0 )
		return 0;

	Q_strncat( m_szString, pszMessage, sizeof( m_szString ), COPY_ALL_CHARACTERS );

	return iLength;
}

//=========================================================
//=========================================================
#define THIRDPERSON_PREFIX "/me "
#define THIRDPERSON_PREFIX_LENGTH 4

//=========================================================
//=========================================================
char *UTIL_HandleChatString( char *pszString, bool &bThirdPerson )
{
	if( !pszString )
		return NULL;

	// strip all bad chars
	char *pszCleanedString = UTIL_CleanChatString( pszString );

	if( !pszCleanedString )
		return NULL;

	// handle third person
	bThirdPerson = ( Q_strlen( pszString ) > THIRDPERSON_PREFIX_LENGTH && ( Q_strncmp( pszCleanedString, THIRDPERSON_PREFIX, THIRDPERSON_PREFIX_LENGTH ) == 0 ) );

	if( bThirdPerson )
		pszCleanedString += THIRDPERSON_PREFIX_LENGTH;

	return pszCleanedString;
}

//=========================================================
//=========================================================
char *UTIL_CleanChatString( char *pszString )
{
	// invalid if NULL or empty
	if ( !pszString || *pszString == '\0' )
		return NULL;

	// cut the message off
	int iLength = Q_strlen( pszString );

	if( iLength > MAX_CHATMSG_LENGTH )
	{
		pszString[ MAX_CHATMSG_LENGTH - 1 ] = '\0';

	#ifdef CLIENT_DLL

		iLength = MAX_CHATMSG_LENGTH;

	#endif
	}

#ifdef CLIENT_DLL

	// strip any trailing '\n'
	if( pszString[ iLength - 1 ] == '\n' )
		pszString[ iLength - 1 ] = '\0';

	// strip leading '\n' characters (or notify/color signifies)
	while( *pszString && ( *pszString == '\n' || *pszString == 1 || *pszString == 2 ) )
		pszString++;

	// ensure valid string
	if( *pszString == '\0' )
		return NULL;

#endif

	return pszString;
}

//=========================================================
//=========================================================
void UTIL_ParseChatMessage( CColoredString &String, int iSenderID, int iType, bool bThirdPerson, const char *pszMessage )
{
	if( iType == SAYTYPE_SERVER )
	{
		String.Add( "*** ", CLOOKUP_SERVER );
		String.Add( pszMessage, CLOOKUP_SERVER );

		return;
	}

#ifdef GAME_DLL

	CBasePlayer *pPlayer = UTIL_PlayerByIndex( iSenderID );

#endif

	int iTeamColorID = CLOOKUP_INVALID;

#ifdef CLIENT_DLL

	iTeamColorID = UTIL_TeamColorLookup( PlayerResource( )->GetTeamID( iSenderID ) );

#endif

	bool bAlive;

#ifdef GAME_DLL

	bAlive = pPlayer->IsAlive( );

#else

	bAlive = PlayerResource( )->IsAlive( iSenderID );

#endif

	if( !bAlive )
		String.Add( "*DEAD* " );

	switch( iType )
	{
		case SAYTYPE_TEAM:
		{
			String.Add( "(Team) ", iTeamColorID );
			break;
		}

		case SAYTYPE_SQUAD:
		{
			String.Add( "(Squad) ", iTeamColorID );
			break;
		}
	}

	if( bThirdPerson )
		String.Add( "* ", CLOOKUP_THIRDPERSON );

	const char *pszPlayerName = NULL;

#ifdef GAME_DLL

	pszPlayerName = pPlayer->GetPlayerName( );

#else

	pszPlayerName = PlayerResource( )->GetPlayerName( iSenderID );

#endif

	String.Add( pszPlayerName, iTeamColorID );

	if( bThirdPerson )
	{
		String.Add( " " );
		String.Add( pszMessage, CLOOKUP_THIRDPERSON );
	}
	else
	{
		String.Add( ": ", iTeamColorID );
		String.Add( pszMessage );
	}
}

//=========================================================
//=========================================================
bool UTIL_ValidStatusType( int iID )
{
	return ( iID >= 0 && iID < PSTATUSTYPE_COUNT );
}

//=========================================================
//=========================================================
typedef IStatusProgressive *( *StatusProgressiveCreator_t )( void );
StatusProgressiveCreator_t g_StatusProgressiveHelpers[ PSTATUSTYPE_COUNT ];

class CStatusProgressiveHelper
{
public:
	CStatusProgressiveHelper( int iID, StatusProgressiveCreator_t StatusProgressiveCreator )
	{
		g_StatusProgressiveHelpers[ iID ] = StatusProgressiveCreator;
	}
};

#define DEFINE_STATUSPROGRESSIVE( id, classname ) \
	IStatusProgressive *CreateStatusProgressive__##id( void ) { \
	return new classname; } \
	CStatusProgressiveHelper g_CommsGroupHelper__##id( id, CreateStatusProgressive__##id );

//=========================================================
//=========================================================
bool StatusProgressiveDataLess( const int &iLeft, const int &iRight )
{
	return ( iLeft > iRight );
}

//=========================================================
//=========================================================
IStatusProgressive::IStatusProgressive( )
{
	m_Data.SetLessFunc( StatusProgressiveDataLess );
}

//=========================================================
//=========================================================
const StatusProgressiveData_t *IStatusProgressive::Find( int iID ) const
{
	int iDataID = m_Data.Find( iID );

	if( !m_Data.IsValidIndex( iDataID ) )
		return NULL;

	return &m_Data[ iDataID ];
}

//=========================================================
//=========================================================
void IStatusProgressive::InitSound( const char *pszSound, const char *pszProgressive, const char *pszText )
{
	Assert( m_Data.Count( ) == 0 );
	AddSound( 0, pszSound, pszProgressive, pszText );
}

//=========================================================
//=========================================================
void IStatusProgressive::AddSound( int iID, const char *pszSound, const char *pszProgressiveText, const char *pszText )
{
	Assert( pszSound && pszText );

	int iDataID = m_Data.Insert( iID ); 
	Assert( m_Data.IsValidIndex( iDataID ) );

	StatusProgressiveData_t &Data = m_Data[ iDataID ];
	Data.pszSound = pszSound;
	Data.pszProgressiveText = pszProgressiveText;
	Data.pszText = pszText;
}

//=========================================================
//=========================================================
namespace StatusProgressive {

//=========================================================
//=========================================================
class CObjOrderStatus : public IStatusProgressive
{
public:
	CObjOrderStatus( )
	{
		static const char *pszObjSounds[ ORDERTYPE_OBJ_COUNT ] = {
			"standingby",	// ORDERTYPE_OBJ_NONE
			"attacking",	// ORDERTYPE_OBJ_ATTACK
			"defending"		// ORDERTYPE_OBJ_DEFEND
		};

		static const char *pszObjProgressiveText[ ORDERTYPE_OBJ_COUNT ] = {
			"Standing By",		// ORDERTYPE_OBJ_NONE
			"I'm Attacking",	// ORDERTYPE_OBJ_ATTACK
			"I'm Defending"		// ORDERTYPE_OBJ_DEFEND
		};

		static const char *pszObjText[ ORDERTYPE_OBJ_COUNT ] = {
			"Standing By",		// ORDERTYPE_OBJ_NONE
			"Attacking",		// ORDERTYPE_OBJ_ATTACK
			"Defending"			// ORDERTYPE_OBJ_DEFEND
		};

		for( int i = 0; i < ORDERTYPE_OBJ_COUNT; i++ )
			AddSound( i, pszObjSounds[ i ], pszObjProgressiveText[ i ], pszObjText[ i ] );
	}
};

DEFINE_STATUSPROGRESSIVE( PSTATUSTYPE_ORDER_OBJ, CObjOrderStatus );

//=========================================================
//=========================================================
class CUnitOrderStatus : public IStatusProgressive
{
public:
	CUnitOrderStatus( )
	{
		static const char *pszUnitSounds[ ORDERTYPE_UNIT_COUNT ] = {
			"securing",		// ORDERTYPE_UNIT_SECURE
			"moving",		// ORDERTYPE_UNIT_MOVE
			"guarding",		// ORDERTYPE_UNIT_GUARD
			"regrouping",	// ORDERTYPE_UNIT_REGROUP
			"stealthing"	// ORDERTYPE_UNIT_STEALTH
		};

		static const char *pszUnitProgressiveText[ ORDERTYPE_UNIT_COUNT ] = {
			"I'm Securing",		// ORDERTYPE_UNIT_SECURE
			"I'm Moving",		// ORDERTYPE_UNIT_MOVE
			"I'm Guarding",		// ORDERTYPE_UNIT_GUARD
			"I'm Regrouping",	// ORDERTYPE_UNIT_REGROUP
			"I'm Stealthing"	// ORDERTYPE_UNIT_STEALTH
		};

		static const char *pszUnitText[ ORDERTYPE_UNIT_COUNT ] = {
			"Securing",		// ORDERTYPE_UNIT_SECURE
			"Moving",		// ORDERTYPE_UNIT_MOVE
			"Guarding",		// ORDERTYPE_UNIT_GUARD
			"Regrouping",	// ORDERTYPE_UNIT_REGROUP
			"Stealthing"	// ORDERTYPE_UNIT_STEALTH
		};

		for( int i = 0; i < ORDERTYPE_UNIT_COUNT; i++ )
			AddSound( i, pszUnitSounds[ i ], pszUnitProgressiveText[ i ], pszUnitText[ i ] );
	}
};

DEFINE_STATUSPROGRESSIVE( PSTATUSTYPE_ORDER_UNIT, CUnitOrderStatus );

//=========================================================
//=========================================================
class CPlayerOrderStatus : public IStatusProgressive
{
public:
	CPlayerOrderStatus( )
	{
		static const char *pszPlayerSounds[ PORDER_COUNT ] = {
			"flankingleft",		// PORDER_FLANKLEFT
			"flankingright",	// PORDER_FLANKRIGHT
			"moving",			// PORDER_MOVING
			"takingcover",		// PORDER_TAKECOVER
			"coveringfire",		// PORDER_COVERINGFIRE
			"holdfire",			// PORDER_HOLDFIRE
			"returingfire"		// PORDER_RETURNFIRE
		};

		static const char *pszProgressivePlayerText[ PORDER_COUNT ] = {
			"I'm Flanking",		// PORDER_FLANKLEFT
			"I'm Flanking",		// PORDER_FLANKRIGHT
			"I'm Moving",		// PORDER_MOVING
			"Taking Cover",		// PORDER_TAKECOVER
			"Covering Fire",	// PORDER_COVERINGFIRE
			"Holding Fire",		// PORDER_HOLDFIRE
			"Returing Fire"		// PORDER_RETURNFIRE
		};

		static const char *pszPlayerText[ PORDER_COUNT ] = {
			"Flanking",			// PORDER_FLANKLEFT
			"Flanking",			// PORDER_FLANKRIGHT
			"Moving",			// PORDER_MOVING
			"Taking Cover",		// PORDER_TAKECOVER
			"Covering Fire"		// PORDER_COVERINGFIRE
			"Holding Fire"		// PORDER_HOLDFIRE
			"Returing Fire"		// PORDER_RETURNFIRE
		};

		for( int i = 0; i < ORDERTYPE_OBJ_COUNT; i++ )
			AddSound( i, pszPlayerSounds[ i ], pszProgressivePlayerText[ i ], pszPlayerText[ i ] );
	}
};

DEFINE_STATUSPROGRESSIVE( PSTATUSTYPE_ORDER_PLAYER, CPlayerOrderStatus );

//=========================================================
//=========================================================
class CObjectiveStatus : public IStatusProgressive
{
public:
	CObjectiveStatus( )
	{
		static const char *pszPlayerSounds[ PSTATUS_OBJ_COUNT ] = {
			"capturing",		// PSTATUS_OBJ_CAPTURING
			"defending"			// PSTATUS_OBJ_DEFENDING
		};

		static const char *pszProgressivePlayerText[ PSTATUS_OBJ_COUNT ] = {
			"I'm Capturing",	// PSTATUS_OBJ_CAPTURING
			"I'm Defending"		// PSTATUS_OBJ_DEFENDING
		};

		static const char *pszPlayerText[ PSTATUS_OBJ_COUNT ] = {
			"Capturing",		// PSTATUS_OBJ_CAPTURING
			"Defending"			// PSTATUS_OBJ_DEFENDING
		};

		for( int i = 0; i < PSTATUS_OBJ_COUNT; i++ )
			AddSound( i, pszPlayerSounds[ i ], pszProgressivePlayerText[ i ], pszPlayerText[ i ] );
	}
};

DEFINE_STATUSPROGRESSIVE( PSTATUSTYPE_OBJECTIVE, CObjectiveStatus );

};

//=========================================================
//=========================================================
class CStatusProgressiveMgr : public CAutoGameSystem
{
public:
	bool Init( void );

	inline IStatusProgressive *Type( int iID ) const;

private:
	IStatusProgressive *m_pStatusProgress[ PSTATUSTYPE_COUNT ];
};

CStatusProgressiveMgr g_StatusProgressiveMgr;

//=========================================================
//=========================================================
bool CStatusProgressiveMgr::Init( void )
{
	for( int i = 0; i < PSTATUSTYPE_COUNT; i++ )
	{
		StatusProgressiveCreator_t Creator = g_StatusProgressiveHelpers[ i ];
		Assert( Creator );

		m_pStatusProgress[ i ] = ( Creator )( );
	}

	return true;
}

//=========================================================
//=========================================================
IStatusProgressive *CStatusProgressiveMgr::Type( int iID ) const
{
	Assert( UTIL_ValidStatusType( iID ) );
	Assert( m_pStatusProgress[ iID ] );

	return m_pStatusProgress[ iID ];
}

//=========================================================
//=========================================================
const IStatusProgressive *UTIL_StatusProgressive( int iType )
{
	return g_StatusProgressiveMgr.Type( iType );
}

//=========================================================
//=========================================================
const StatusProgressiveData_t *UTIL_StatusProgressiveData( int iType, int iID )
{
	const IStatusProgressive *pStatusProgressive = UTIL_StatusProgressive( iType );

	if( !pStatusProgressive )
		return NULL;

	return pStatusProgressive->Find( iID );
}

//=========================================================
//=========================================================
void UTIL_FastRotate( float flAngle, float &flX, float &flY )
{
	float flSinA = sin( flAngle );
	float flCosA = cos( flAngle );
	float flYT = flY;
	flY = flX * flSinA;
	flX *= flCosA;
	flSinA *= flYT;
	flCosA *= flYT;
	flX -= flSinA;
	flY += flCosA;
}

//=========================================================
//=========================================================
void UTIL_PrintCheatMessage( CBasePlayer *pPlayer )
{
	ClientPrint( pPlayer, HUD_PRINTCONSOLE, "Can't use cheat cvars in multiplayer, unless the server has sv_cheats set to 1.\n" );
}

//=========================================================
//=========================================================
int UTIL_FindOrientation( CBaseEntity *pEntity, Vector &vecReferance )
{
	Assert( pEntity );

	if( !pEntity )
		return ORIENTATION_INVALID;

	Vector vecOffset = pEntity->GetAbsOrigin( ) - vecReferance;

	QAngle angTemp;
	VectorAngles( vecOffset, angTemp );

	angTemp[ YAW ] -= 25.0f;

	if( angTemp[ YAW ] < 0.0f )
		angTemp[ YAW ] = 360 + angTemp[ YAW ];

	return floor( angTemp[ YAW ] / 45.0f );
}

//=========================================================
//=========================================================
const unsigned char *GetEncryptionKey( void )
{
	return ( const unsigned char * )"W4Di4LQc";
}

//=========================================================
//=========================================================
bool UTIL_FindInList( const char **pStrings, const char *pszToFind )
{
	int i = 0;

	while( pStrings[ i ] != NULL )
	{
		if( Q_stricmp( pStrings[ i ], pszToFind ) == 0 )
			return true;

		i++;
	}

	return false;
}

//=========================================================
//=========================================================
bool UTIL_FindPrefixInList( const char **pStrings, const char *pszToFind )
{
	int i = 0;

	while( pStrings[ i ] != NULL )
	{
		const char *pszPrefix = pStrings[ i ];

		int iPrefixLength = Q_strlen( pszPrefix );

		if( Q_strnicmp( pszPrefix, pszToFind, iPrefixLength ) == 0 )
			return true;

		i++;
	}

	return false;
}