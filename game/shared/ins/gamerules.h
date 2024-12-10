//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef GAMERULES_H
#define GAMERULES_H
#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL

#include "c_baseentity.h"

#define CGameRules C_GameRules
#define CGameRulesProxy C_GameRulesProxy

#else

#include "baseentity.h"

#endif

#include "igamesystem.h"
#include "gamerules_register.h"

//=========================================================
//=========================================================
class CBasePlayer;

//=========================================================
//=========================================================
enum
{
	GR_FRIEND = 0,
	GR_ENEMY
};

//=========================================================
//=========================================================
class CGameRulesProxy : public CBaseEntity
{
public:
	DECLARE_CLASS( CGameRulesProxy, CBaseEntity );
	DECLARE_NETWORKCLASS();

	CGameRulesProxy();
	~CGameRulesProxy();

	virtual int UpdateTransmitState( void );
	static void NotifyNetworkStateChanged( void );

private:
	static CGameRulesProxy *s_pGameRulesProxy;
};

//=========================================================
//=========================================================
abstract_class CGameRules : public CAutoGameSystemPerFrame
{
	DECLARE_CLASS_GAMEROOT( CGameRules, CAutoGameSystemPerFrame );

	virtual char const *Name( void ) { return "CGameRules"; }

public:
	CGameRules(void);
	virtual ~CGameRules( void );

	// General
	virtual void Precache( void ) = 0;

	virtual bool ShouldCollide( int collisionGroup0, int collisionGroup1 );

	inline void NetworkStateChanged( void )
	{
		// forward the call to the entity that will send the data.
		CGameRulesProxy::NotifyNetworkStateChanged( );
	}

	inline void NetworkStateChanged( void *pVar )
	{
		// forward the call to the entity that will send the data.
		CGameRulesProxy::NotifyNetworkStateChanged( );
	}

	virtual const CViewVectors *GetViewVectors( void ) const;

	// Player Management
	virtual bool FPlayerCanTakeDamage( CBasePlayer *pPlayer, CBaseEntity *pAttacker ) = 0;
	virtual void PlayerThink( CBasePlayer *pPlayer ) = 0;

	virtual int DefaultFOV( void ) const = 0;
	virtual int DefaultWeaponFOV( void ) const = 0;

#ifdef GAME_DLL

public:

	// General
	virtual void LevelInitPreEntity( void );
	virtual void LevelShutdown( void ) { }

	virtual void FrameUpdatePostEntityThink( void );
	virtual void Think( void ) = 0;
	virtual void EndGameFrame( void );

	// Change Level
	virtual void ChangeLevel( void );
	virtual void GetNextLevelName( char *pszNextMap, int iBufSize );

	// Player Management
	virtual bool ClientConnected( edict_t *pEntity, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen ) = 0;
	virtual bool ClientCommand(CBasePlayer* pBasePlayer, const CCommand& args) = 0;
	virtual void ClientSettingsChanged( CBasePlayer *pPlayer ) = 0;
	virtual void ClientDisconnected( edict_t *pClient ) = 0;
	
	virtual void PlayerSpawn( CBasePlayer *pPlayer ) = 0;
	virtual CBaseEntity *GetPlayerSpawnSpot( CBasePlayer *pPlayer ) = 0;
	virtual void UpdatePlayerData( CBasePlayer *pPlayer ) { }

	virtual void PlayerKilled( CBasePlayer *pVictim, const CTakeDamageInfo &info ) = 0;

	virtual int PlayerRelationship( CBasePlayer *pPlayer, CBaseEntity *pTarget ) = 0;
	virtual bool PlayerCanCommunicate( CBasePlayer *pListener, CBasePlayer *pSpeaker ) = 0;

	// Misc
	virtual bool CanEntityBeUsePushed( CBaseEntity *pEnt ) { return true; }

#endif
};

//=========================================================
//=========================================================
extern CGameRules *g_pGameRules;

inline CGameRules *GameRules( void )
{
	return g_pGameRules;
}

//=========================================================
//=========================================================
#ifdef GAME_DLL

void InstallGameRules( void );
void RegisterUserMessages( void );

#endif

//=========================================================
//=========================================================
void RadiusDamage( const CTakeDamageInfo &info, const Vector &vecSrc, float flRadius, int iClassIgnore, CBaseEntity *pEntityIgnore );

#endif // GAMERULES_H