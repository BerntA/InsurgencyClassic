//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Shared Squad Header
//
// $NoKeywords: $
//=============================================================================//

#ifndef SQUAD_SHARED_H
#define SQUAD_SHARED_H
#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL

#define CINSSquad C_INSSquad

#endif

//=========================================================
//=========================================================
#include "imc_format.h"
#include "commander_shared.h"
#include "squad_data.h"
#include "ins_player_shared.h"

//=========================================================
//=========================================================
class CTeamLookup;
class CPlayerClass;

#ifdef GAME_DLL
// How many seconds a blacklist should last
#define CMDR_BLACKLIST_LENGTH 120.0f
#define CMDR_RESPONSE_LENGTH 30.0f
#define CMDR_PROMO_TICK 0.2f
#define CMDR_PROMO_OVERFLOW 30.f

class CPlayTeam;
struct IMCSquadData_t;

#endif

//=========================================================
//=========================================================
class CINSSquad : public CBaseEntity
{
public:
	DECLARE_CLASS( CINSSquad, CBaseEntity );
	DECLARE_NETWORKCLASS( );

public:

#ifdef GAME_DLL

	DECLARE_DATADESC();

	CINSSquad( );

	// General
	void Init( int iID, int iTeamID, IMCSquadData_t *pSquadData );

	int UpdateTransmitState( void );

	// Status Management
	void SendStatusUpdate( void );

	// Slot Management
	bool IsValidSquadData( const SquadData_t &SquadData );

	void ResetSlots( void );

	void AddPlayer( CINSPlayer *pPlayer, const SquadData_t &SquadData );
	void RemovePlayer( CINSPlayer *pPlayer );

	// Status Management
	void SetEnabled( bool bState );
	void ResetEnabled( void );

	// Spawn Management
	bool SpawnPlayersValid( void );
	void SpawnPlayers( int iType, CUtlVector< CINSPlayer* > &PlayersSpawned );

	// Server-side commander control
	void KickCommander( void );
	void SelectNewCommander( void );
	void PromotionResponse( CINSPlayer* pPlayer, bool bAccept );
	void CmdrPromotionThink( void );

	// Order Management
	void ResetOrders( void );

	void ResetObjOrders( void );
	bool AssignObjOrders( CINSObjective *pObjective );
	const CObjOrder &GetObjOrders( void ) const { return m_ObjOrders; }
	bool HasObjOrders( void ) const;
	bool IsFollowingAttackOrders( CINSObjective *pObjective ) const;
	void NotifyInvalidObjOrders( void );

	void ResetUnitOrders( void );
	bool AssignUnitOrders( int iOrderType, const Vector &vecPosition );
	const CUnitOrder &GetUnitOrders( void ) const { return m_UnitOrders; }
	bool HasUnitOrders( void ) const;

	void UpdateOrders( CINSPlayer *pPlayer );

	void ResetPlayerOrders( void );
	void AssignPlayerOrders( int iOrderType );

#ifdef TESTING

	void ForcePlayerOrders( int iOrderType );

#endif

	// Misc
	static CINSSquad *Create( void );

	static bool IsValidSlotData( int iTeam, int iSlotID );

	int GetID( void ) const { return m_iID; }
	CPlayTeam *GetParentTeam( void ) const;

#ifdef TESTING

	void PrintSquad( void );

#endif

#else

	// Name Management
	static const char *GetDefaultName( int iSquad );

	// Player Management
	int GetPlayerID( int iSlotID ) const;
	virtual const char *CanSpawn( const CINSPlayer *pPlayer ) const { return NULL; }

	

#endif

	// Score Management
	int GetScore( void );

	// Name Management
	const char *GetName( void ) const { return m_szName; }

	// Slot Management
	bool HasFreeSlot( void ) const;

	bool IsSlotEmpty( int iSlotID ) const;
	CPlayerClass *GetClass( int iSlotID ) const;
	int GetClassID( int iSlotID ) const;

	int CountFreeSlots( void ) const;
	int CountOccupiedSlots( void ) const;
	bool HasPlayers( void ) const;

	// Player Management
	CINSPlayer *GetPlayer( int iSlotID ) const;

	bool HasCommander( void ) const;
	CINSPlayer *GetCommander( void );

	// Status Management
	bool IsEnabled( void ) const;

	// Misc
	static bool IsValidSquad( int iSquadID );
	static bool IsValidSlotID( int iSlotID );

protected:

	CTeamLookup *GetTeamLookup( void ) const;

private:

#ifdef GAME_DLL

	// Order Management
	void SetPlayerOrders( int iOrderType );

#endif

protected:

#ifdef GAME_DLL

	// the ID of the squad
	int m_iID;

	// this is used for when objectives are disabled
	int m_iSpawnObjective;

	// Used for the black list
	struct BlackListEntry_t 
	{
		CINSPlayer*	pPlayer;
		float		fReleaseTime;
	};

	// Kicked commanders go in here for a set amount of time
	CUtlVector<BlackListEntry_t> m_CmdrBlackList;

	// Used to store "most eligible" commander candidate
	CINSPlayer* pMostEligible;

	// Used to control the state of the promotion system within the think function
	bool bPromoResponse;
	float fPromoTimeout;
	float fPromoAccum;
#endif

	// information that defines the squad
	CNetworkVar( int, m_iParentTeam );

	CNetworkString( m_szName, MAX_SQUADNAME_LENGTH );

	CNetworkArray( int, m_iSlots, MAX_SQUAD_SLOTS );
	CNetworkArray( int, m_iPlayerSlots, MAX_SQUAD_SLOTS );

	CNetworkVar( bool, m_bEnabled );

#ifdef GAME_DLL

	// orders
	CObjOrder m_ObjOrders;
	CUnitOrder m_UnitOrders;

#endif
};

#endif // SQUAD_SHARED_H
