//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef INS_RECIPIENTFILTER_H
#define INS_RECIPIENTFILTER_H
#ifdef _WIN32
#pragma once
#endif

#include "recipientfilter.h"

//=========================================================
//=========================================================
class CINSPlayer;
class CINSSquad;
class CPlayTeam;

//=========================================================
//=========================================================
class CReliableRecipientFilter : public CRecipientFilter
{
public:
	CReliableRecipientFilter( );
};

//=========================================================
//=========================================================
class CReliablePlayTeamRecipientFilter : public CReliableRecipientFilter
{
public:
	CReliablePlayTeamRecipientFilter( );
};

//=========================================================
//=========================================================
class CReliablePlayerRecipientFilter : public CReliableRecipientFilter
{
public:
	CReliablePlayerRecipientFilter( CINSPlayer *pPlayer );
};

//=========================================================
//=========================================================
class CReliableSquadRecipientFilter : public CReliableRecipientFilter
{
public:
	CReliableSquadRecipientFilter( CINSSquad *pSquad );
};

//=========================================================
//=========================================================
class CUnitRecipientFilter : public CReliableRecipientFilter
{
public:
	CUnitRecipientFilter( ) { }
	CUnitRecipientFilter( CINSPlayer *pPlayer );

	void Setup( CINSPlayer *pPlayer );
};

#endif // INS_RECIPIENTFILTER_H