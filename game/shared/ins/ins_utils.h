//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// $NoKeywords: $
//=============================================================================//

#ifndef INS_UTIL_H
#define INS_UTIL_H
#pragma once

#include "imc_format.h"
#include "ins_player_shared.h"
#include "ins_coloredstring.h"
#include "utlmap.h"
#include "ins_shared.h"

#ifdef GAME_DLL

#include "hint_helper.h"

#endif

//=========================================================
//=========================================================
const Vector &UTIL_PlayerViewMins( int iStance );
const Vector &UTIL_PlayerViewMaxs( int iStance );
const Vector &UTIL_PlayerViewMins( const CINSPlayer *pPlayer );
const Vector &UTIL_PlayerViewMaxs( const CINSPlayer *pPlayer );
Vector UTIL_PlayerViewOffset( CINSPlayer *pPlayer, int iStance );

Vector UTIL_ViewModelStanceOffset( int iStance );
Vector UTIL_ViewModelMovingOffset( int iStance );

int UTIL_GetNextStance( int iStanceFrom, int iStanceTo );
float UTIL_StanceTransitionTime( int iStanceFrom, int iStanceTo );
float UTIL_ProneThreshold( int iStance );

//=========================================================
//=========================================================
bool UTIL_IsMoving( const Vector &vecVelocity );
bool UTIL_IsMoving( const Vector &vecVelocity, int iMinSpeed );

//=========================================================
//=========================================================
extern float UTIL_3DCaculateDistance( const CBaseEntity *pEntity, CBasePlayer *pPlayer );
extern float UTIL_2DCaculateDistance( const CBaseEntity *pEntity, CBasePlayer *pPlayer );

extern float UTIL_3DCaculateDistance( const Vector &vecOrigin, CBasePlayer *pPlayer );
extern float UTIL_2DCaculateDistance( const Vector &vecOrigin, CBasePlayer *pPlayer );

//=========================================================
//=========================================================
extern bool UTIL_ValidObjectiveID( int iID );

//=========================================================
//=========================================================
extern int UTIL_CaculateObjType( int iTeamID, int iObjID );
extern int UTIL_CaculateObjType( int iTeamID, const CINSObjective *pObjective );
extern int UTIL_CaculateObjType( const CINSPlayer *pPlayer, const CINSObjective *pObjective );

//=========================================================
//=========================================================
#ifdef GAME_DLL

#define INS_EXPLOSION_DEFAULTSOUND "BaseGrenade.Explode"

extern void UTIL_CreateExplosion( const Vector &vecOrigin, CBaseEntity *pAttacker, CBaseEntity *pInflictor, int iDamage, int iDamageRadius, int iExtraDamageFlags, const char *pszSound = NULL, trace_t *pCheckTrace = NULL, Vector *pReported = NULL );

extern void UTIL_SendHint( CINSPlayer *pPlayer, int iHintID );
extern const char *UTIL_FindPlaceName( const Vector &vecPos );

extern 

//=========================================================
//=========================================================
#else

extern const char *g_pszPhoneticAlphabet[ ALPHABET_SIZE ];

#endif

//=========================================================
//=========================================================
void RadiusDamage( const CTakeDamageInfo &info, const Vector &vecSrc, float flRadius, int iClassIgnore, CBaseEntity *pEntityIgnore, bool bSendConcussion );

//=========================================================
//=========================================================
#ifdef GAME_DLL

extern int UTIL_WeaponStatisticType( int iHitGroup );

#endif

//=========================================================
//=========================================================
extern char *UTIL_HandleChatString( char *pszString, bool &bThirdPerson );
extern char *UTIL_CleanChatString( char *pszString );

extern void UTIL_ParseChatMessage( CColoredString &String, int iSenderID, int iType, bool bThirdPerson, const char *pszMessage );

//=========================================================
//=========================================================
#ifdef GAME_DLL

extern void UTIL_SendKeyValues( KeyValues *pData );

#else

extern void UTIL_ReadKeyValues( bf_read &msg, KeyValues *pData );

#endif

//=========================================================
//=========================================================
extern bool UTIL_ValidStatusType( int iID );

struct StatusProgressiveData_t
{
	const char *pszSound, *pszProgressiveText, *pszText;
};

class IStatusProgressive
{
public:
	IStatusProgressive( );

	const StatusProgressiveData_t *Find( int iID ) const;
	const CUtlMap< int, StatusProgressiveData_t > &Data( void ) const { return m_Data; }

protected:
	void InitSound( const char *pszSound, const char *pszProgressive, const char *pszText );
	void AddSound( int iID, const char *pszSound, const char *pszProgressive, const char *pszText );

private:
	CUtlMap< int, StatusProgressiveData_t > m_Data;
};

extern const IStatusProgressive *UTIL_StatusProgressive( int iType );
extern const StatusProgressiveData_t *UTIL_StatusProgressiveData( int iType, int iID );

//=========================================================
//=========================================================
#ifdef _DEBUG

extern void UTIL_AddLineList( const Vector &vecStart, const Vector &vecEnd, const Color &LineColor );
extern void UTIL_DrawLineList( void );
extern void UTIL_ClearLineList( void );

#endif

//=========================================================
//=========================================================
extern void UTIL_FastRotate( float flAngle, float &flX, float &flY );

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

extern bool UTIL_IsVentRunning( void );

#endif

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

extern void UTIL_RemoveCommandFlags( const ConCommandBase *pCommand, int iFlags );

#endif

//=========================================================
//=========================================================
extern void UTIL_PrintCheatMessage( CBasePlayer *pPlayer );

//=========================================================
//=========================================================
#ifdef GAME_DLL

Vector UTIL_SpawnPositionOffset( CBaseEntity *pEntity );

#endif

//=========================================================
//=========================================================
enum Orientation_t
{
	ORIENTATION_INVALID = -1,
	ORIENTATION_NORTH = 0,
	ORIENTATION_NORTHEAST,
	ORIENTATION_EAST,
	ORIENTATION_SOUTHEAST,
	ORIENTATION_SOUTH,
	ORIENTATION_SOUTHWEST,
	ORIENTATION_WEST,
	ORIENTATION_NORTHWEST,
	ORIENTATION_COUNT
};

//=========================================================
//=========================================================
extern int UTIL_FindOrientation( CBaseEntity *pEntity, Vector &vecReferance );

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

extern const char *g_pszOrientationNames[ ORIENTATION_COUNT ];

extern const char *g_pszFireModeNames[ FIREMODE_COUNT ];

extern char *ConvertCRtoNL( char *str );
extern wchar_t *ConvertCRtoNL( wchar_t *str );
extern void StripEndNewlineFromString( char *str );
extern void StripEndNewlineFromString( wchar_t *str );

extern void UTIL_ParseMapName( char *pszBuffer, int iLength );

typedef CUtlVector< wchar_t > TextStream_t;
extern int UTIL_ComputeStringWidth( vgui::HFont &hFont, TextStream_t &TextStream );

#endif

//=========================================================
//=========================================================
extern bool UTIL_FindInList( const char **pStrings, const char *pszToFind );
extern bool UTIL_FindPrefixInList( const char **pStrings, const char *pszToFind );

#endif // INS_UTIL_H