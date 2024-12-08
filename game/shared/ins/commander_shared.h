//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//=============================================================================//

#ifndef INS_COMMANDER_SHARED_H
#define INS_COMMANDER_SHARED_H
#ifdef _WIN32
#pragma once
#endif

#include "ins_obj_shared.h"

//=========================================================
//=========================================================
#define COMMANDER_SLOT 0

//=========================================================
//=========================================================

// PNOTE: the preprocessor names are based off the US names
enum RankSystem_t
{
	INVALID_RANK = -1,
	RANK_PRIVATE = 0,
	RANK_LCORPORAL,
	RANK_CORPORAL,
	RANK_SERGEANT,
	RANK_LIEUTENANT,
	RANK_COUNT
};

#define RANK_COMMANDER RANK_LIEUTENANT

#ifdef GAME_DLL

DECLARE_STRING_LOOKUP_CONSTANTS( int, ranks )
#define LookupRank( k, j ) STRING_LOOKUP( ranks, k, j )

#endif

typedef int RankBoundaries_t[ RANK_COUNT ];

extern bool UTIL_ValidRank( int iRankID );

#ifdef CLIENT_DLL

extern const char *g_pszRankIcons[ RANK_COUNT ];

#endif

//=========================================================
//=========================================================
enum ObjOrderType_t
{
	ORDERTYPE_OBJ_NONE = 0,
	ORDERTYPE_OBJ_ATTACK,
	ORDERTYPE_OBJ_DEFEND,
	ORDERTYPE_OBJ_COUNT
};

#ifdef CLIENT_DLL

extern const char *g_pszObjOrderTypeNames[ ORDERTYPE_OBJ_COUNT ];

#endif

//=========================================================
//=========================================================
class CObjOrder
{
public:
	CObjOrder( );

#ifdef GAME_DLL

	void Init( void );
	void Init( CINSObjective *pObjective );

#else

	void Init( bf_read &msg );

#endif

	static bool IsValidOrder( int iOrderType );

	void Reset( void );

#ifdef GAME_DLL

	void Send( IRecipientFilter &filter ) const;

#endif

	bool HasOrders( void ) const;
	bool HasOrder( CINSObjective *pObjective ) const;

	inline CINSObjective *Objective( void ) const { return m_pObjective; }

#ifdef CLIENT_DLL

	int OrderType( void ) const;

#endif

public:
	CINSObjective *m_pObjective;
};

//=========================================================
//=========================================================
enum UnitOrderType_t
{
	ORDERTYPE_UNIT_INVALID = -1,
	ORDERTYPE_UNIT_SECURE = 0,
	ORDERTYPE_UNIT_MOVE,
	ORDERTYPE_UNIT_GUARD,
	ORDERTYPE_UNIT_REGROUP,
	ORDERTYPE_UNIT_STEALTH,
	ORDERTYPE_UNIT_COUNT
};

#ifdef CLIENT_DLL

extern const char *g_pszUnitOrderTypeNames[ ORDERTYPE_UNIT_COUNT ];

#endif

//=========================================================
//=========================================================
class CUnitOrder
{
public:
	CUnitOrder( );

#ifdef GAME_DLL

	static void Precache( void );

	void Init( int iOrderType, const Vector &vecPosition );

#else

	void Init( bf_read &msg );

#endif

	static bool IsValidOrder( int iOrderType );

	void Reset( void );

#ifdef GAME_DLL

	void Send( IRecipientFilter &filter );

#endif

	int OrderType( void ) const { return m_iOrderType; }
	const Vector &Position( void ) const { return m_vecPosition; }

	bool HasOrders( void ) const;

	static const char *ModelName( int iID );

private:
	int m_iOrderType;
	Vector m_vecPosition;
};

//=========================================================
//=========================================================
enum PlayerOrders_t
{
	INVALID_PORDER = -1,
	PORDER_FLANKLEFT,
	PORDER_FLANKRIGHT,
	PORDER_MOVING,
	PORDER_TAKECOVER,
	PORDER_COVERINGFIRE,
	PORDER_HOLDFIRE,
	PORDER_RETURNFIRE,
	PORDER_COUNT
};

#define MAX_PORDER_BITS 3

enum PlayerOrderResponses_t
{
	INVALID_PORESPONSE = -1,
	PORESPONSE_DOING,
	PORESPONSE_DONE,
	PORESPONSE_CANNOT,
	PORESPONSE_COUNT
};

extern bool UTIL_ValidPlayerOrder( int iType );
extern bool UTIL_ValidPlayerOrderResponse( int iType );

#endif // INS_COMMANDER_SHARED_H
