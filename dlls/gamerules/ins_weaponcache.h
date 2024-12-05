//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef INS_WEAPONCACHE_H
#define INS_WEAPONCACHE_H
#ifdef _WIN32
#pragma once
#endif

#include "props.h"

//=========================================================
//=========================================================
class CINSObjective;
class CINSPlayer;
class CIMCWeaponCache;

//=========================================================
//=========================================================
class CWeaponCacheBlueprint : public CInventoryBlueprint
{
};

//=========================================================
//=========================================================
class CINSWeaponCache : public CBaseProp
{
	DECLARE_CLASS( CINSWeaponCache, CBaseProp );
	DECLARE_DATADESC( );
	DECLARE_SERVERCLASS( );

public:
	CINSWeaponCache( );

	void Setup( CIMCWeaponCache *pWeaponCache );

	int GetID( void ) const { return m_iID; }

	bool HasIMCParent( void ) const;
	CIMCWeaponCache *GetIMCParent( void ) const { return m_pParent; }

	void SetObjParent( CINSObjective *pParent );
	bool HasObjParent( void ) const;
	CINSObjective *GetObjParent( void ) const { return m_pObjParent; }

	int GetFlags( void ) const;
	int GetTeam( void );

	void Create( void );
	void Remove( void );
	void Restock( void );

private:
	void Spawn( void );

	bool CreateVPhysics( void );

	void SetState( int iState );
	void UpdateState( void );

	void TraceAttack( const CTakeDamageInfo &info, const Vector &dir, trace_t *ptr );
	void Destory( CINSPlayer *pPlayer );
	void Break( void );

private:
	int m_iID;

	CIMCWeaponCache *m_pParent;

	CINSObjective *m_pObjParent;
	CNetworkVar( int, m_iState );
};

#endif // INS_WEAPONCACHE_H