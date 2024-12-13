//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Contains entities for implementing/changing game rules dynamically within each BSP.
//
//=============================================================================//

#include "cbase.h"
#include "datamap.h"
#include "gamerules.h"
#include "maprules.h"
#include "player.h"
#include "entitylist.h"
#include "entityoutput.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CRuleEntity : public CBaseEntity
{
public:
	DECLARE_CLASS( CRuleEntity, CBaseEntity );

	void	Spawn( void );

	DECLARE_DATADESC();

	void	SetMaster( string_t iszMaster ) { m_iszMaster = iszMaster; }

protected:
	bool	CanFireForActivator( CBaseEntity *pActivator );

private:
	string_t	m_iszMaster;
};

BEGIN_DATADESC( CRuleEntity )

	DEFINE_KEYFIELD( m_iszMaster, FIELD_STRING, "master" ),

END_DATADESC()

void CRuleEntity::Spawn( void )
{
	SetSolid( SOLID_NONE );
	SetMoveType( MOVETYPE_NONE );
	AddEffects( EF_NODRAW );
}

bool CRuleEntity::CanFireForActivator( CBaseEntity *pActivator )
{
	if ( m_iszMaster != NULL_STRING )
	{
		if ( UTIL_IsMasterTriggered( m_iszMaster, pActivator ) )
			return true;
		else
			return false;
	}
	
	return true;
}

// 
// CRulePointEntity -- base class for all rule "point" entities (not brushes)
//
class CRulePointEntity : public CRuleEntity
{
public:
	DECLARE_CLASS( CRulePointEntity, CRuleEntity );

	int		m_Score;
	void		Spawn( void );
};

void CRulePointEntity::Spawn( void )
{
	BaseClass::Spawn();
	SetModelName( NULL_STRING );
	m_Score = 0;
}

// 
// CRuleBrushEntity -- base class for all rule "brush" entities (not brushes)
// Default behavior is to set up like a trigger, invisible, but keep the model for volume testing
//
class CRuleBrushEntity : public CRuleEntity
{
public:
	DECLARE_CLASS( CRuleBrushEntity, CRuleEntity );

	void		Spawn( void );

private:
};

void CRuleBrushEntity::Spawn( void )
{
	SetModel( STRING( GetModelName() ) );
	BaseClass::Spawn();
}

//
// CGameText / game_text	-- NON-Localized HUD Message (use env_message to display a titles.txt message)
//	Flag: All players					SF_ENVTEXT_ALLPLAYERS
//
#define SF_ENVTEXT_ALLPLAYERS			0x0001

class CGameText : public CRulePointEntity
{
public:
	DECLARE_CLASS( CGameText, CRulePointEntity );

	bool	KeyValue( const char *szKeyName, const char *szValue );

	DECLARE_DATADESC();

	inline	bool	MessageToAll( void ) { return (m_spawnflags & SF_ENVTEXT_ALLPLAYERS); }
	inline	void	MessageSet( const char *pMessage ) { m_iszMessage = AllocPooledString(pMessage); }
	inline	const char *MessageGet( void )	{ return STRING( m_iszMessage ); }

	void InputDisplay( inputdata_t &inputdata );
	void Display( CBaseEntity *pActivator );

	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
	{
		Display( pActivator );
	}

private:

	string_t m_iszMessage;
	hudtextparms_t	m_textParms;
};

LINK_ENTITY_TO_CLASS( game_text, CGameText );

// Save parms as a block.  Will break save/restore if the structure changes, but this entity didn't ship with Half-Life, so
// it can't impact saved Half-Life games.
BEGIN_DATADESC( CGameText )

	DEFINE_KEYFIELD( m_iszMessage, FIELD_STRING, "message" ),
	DEFINE_KEYFIELD( m_textParms.channel, FIELD_INTEGER, "channel" ),
	DEFINE_KEYFIELD( m_textParms.x, FIELD_FLOAT, "x" ),
	DEFINE_KEYFIELD( m_textParms.y, FIELD_FLOAT, "y" ),
	DEFINE_KEYFIELD( m_textParms.effect, FIELD_INTEGER, "effect" ),
	DEFINE_KEYFIELD( m_textParms.fadeinTime, FIELD_FLOAT, "fadein" ),
	DEFINE_KEYFIELD( m_textParms.fadeoutTime, FIELD_FLOAT, "fadeout" ),
	DEFINE_KEYFIELD( m_textParms.holdTime, FIELD_FLOAT, "holdtime" ),
	DEFINE_KEYFIELD( m_textParms.fxTime, FIELD_FLOAT, "fxtime" ),

	// Inputs
	DEFINE_INPUTFUNC( FIELD_VOID, "Display", InputDisplay ),

END_DATADESC()

bool CGameText::KeyValue( const char *szKeyName, const char *szValue )
{
	if (FStrEq(szKeyName, "color"))
	{
		int color[4];
		UTIL_StringToIntArray( color, 4, szValue );
		m_textParms.r1 = color[0];
		m_textParms.g1 = color[1];
		m_textParms.b1 = color[2];
		m_textParms.a1 = color[3];
	}
	else if (FStrEq(szKeyName, "color2"))
	{
		int color[4];
		UTIL_StringToIntArray( color, 4, szValue );
		m_textParms.r2 = color[0];
		m_textParms.g2 = color[1];
		m_textParms.b2 = color[2];
		m_textParms.a2 = color[3];
	}
	else
		return BaseClass::KeyValue( szKeyName, szValue );

	return true;
}

void CGameText::InputDisplay( inputdata_t &inputdata )
{
	Display( inputdata.pActivator );
}

void CGameText::Display( CBaseEntity *pActivator )
{
	if (!CanFireForActivator(pActivator))
		return;

	// also send to all if we haven't got a specific activator player to send to 
	if (MessageToAll() || !pActivator || !pActivator->IsPlayer())
	{
		UTIL_HudMessageAll( m_textParms, MessageGet() );
	}
	else
	{
		UTIL_HudMessage(ToBasePlayer(pActivator), m_textParms, MessageGet());
	}
}

//
// CGamePlayerZone / game_player_zone -- players in the zone fire my target when I'm fired
//
// Needs master?
class CGamePlayerZone : public CRuleBrushEntity
{
public:
	DECLARE_CLASS( CGamePlayerZone, CRuleBrushEntity );
	void InputCountPlayersInZone( inputdata_t &inputdata );

	DECLARE_DATADESC();

private:

	COutputEvent m_OnPlayerInZone;
	COutputEvent m_OnPlayerOutZone;

	COutputInt m_PlayersInCount;
	COutputInt m_PlayersOutCount;
};

LINK_ENTITY_TO_CLASS( game_zone_player, CGamePlayerZone );
BEGIN_DATADESC( CGamePlayerZone )

	// Inputs
	DEFINE_INPUTFUNC(FIELD_VOID, "CountPlayersInZone", InputCountPlayersInZone),

	// Outputs
	DEFINE_OUTPUT(m_OnPlayerInZone, "OnPlayerInZone"),
	DEFINE_OUTPUT(m_OnPlayerOutZone, "OnPlayerOutZone"),
	DEFINE_OUTPUT(m_PlayersInCount, "PlayersInCount"),
	DEFINE_OUTPUT(m_PlayersOutCount, "PlayersOutCount"),

END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Counts all the players in the zone. Fires one output per player
//			in the zone, one output per player out of the zone, and outputs
//			with the total counts of players in and out of the zone.
//-----------------------------------------------------------------------------
void CGamePlayerZone::InputCountPlayersInZone(inputdata_t& inputdata)
{
	int playersInCount = 0;
	int playersOutCount = 0;

	if (!CanFireForActivator(inputdata.pActivator))
		return;

	CBasePlayer* pPlayer = NULL;

	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		pPlayer = UTIL_PlayerByIndex(i);
		if (pPlayer)
		{
			trace_t		trace;

			UTIL_TraceModel(
				pPlayer->GetAbsOrigin(),
				pPlayer->GetAbsOrigin(),
				pPlayer->GetPlayerMins(),
				pPlayer->GetPlayerMaxs(),
				this,
				COLLISION_GROUP_NONE,
				&trace
			);

			if (trace.startsolid)
			{
				playersInCount++;
				m_OnPlayerInZone.FireOutput(pPlayer, this);
			}
			else
			{
				playersOutCount++;
				m_OnPlayerOutZone.FireOutput(pPlayer, this);
			}
		}
	}

	m_PlayersInCount.Set(playersInCount, inputdata.pActivator, this);
	m_PlayersOutCount.Set(playersOutCount, inputdata.pActivator, this);
}