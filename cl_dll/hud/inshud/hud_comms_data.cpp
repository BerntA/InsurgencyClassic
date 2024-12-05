//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "hud_comms.h"

#include "ins_player_shared.h"
#include "ins_obj_shared.h"
#include "ins_utils.h"
#include "commander_shared.h"
#include "imc_config.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
namespace CommsData {

#define DECLARE_GROUPBASE( className, baseClassName ) \
	typedef baseClassName BaseClass; \
	typedef className ThisClass;

//=========================================================
//=========================================================
class CGroupSingleIcon : public ICommsGroup
{
	DECLARE_GROUPBASE( CGroupSingleIcon, ICommsGroup );

protected:
	virtual const char *Icon( void ) const = 0;

	virtual void SetupGroup( CommsGroup_t &Group )
	{
		Group.m_Icon = m_Icon;
	}

private:
	void SetupIcons( void )
	{
		const char *pszIcon = Icon( );
		Assert( pszIcon );

		LoadIcon( pszIcon, m_Icon );
	}

private:
	CommsTex_t m_Icon;
};

//=========================================================
//=========================================================
class CGroupSingle : public CGroupSingleIcon
{
	DECLARE_GROUPBASE( CGroupSingle, CGroupSingleIcon );

protected:
	virtual const char *Option( void ) const = 0;

	virtual void Selected( void ) const = 0;

private:
	void SetupGroup( CommsGroup_t &Group )
	{
		BaseClass::SetupGroup( Group );

		const char *pszOption = Option( );
		Assert( pszOption );

		Group.AddOption( pszOption, false );
	}

	void Selected( int iID )
	{
		if( iID == 0 )
			Selected( );
	}
};

//=========================================================
//=========================================================
class CGroupCommander
{
protected:
	bool IsVisible( void ) const
	{
		C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer( );
		return ( pPlayer && pPlayer->IsCommander( ) );
	}
};

//=========================================================
//=========================================================
class CReinforceGroup : public CGroupSingle, public CGroupCommander
{
	DECLARE_GROUPBASE( CReinforceGroup, CGroupSingle );

private:
	const char *Icon( void ) const
	{
		return "reinforce";
	}

	const char *Option( void ) const
	{
		return "Call Reinforcements";
	}

	bool IsVisible( void ) const
	{
		return CGroupCommander::IsVisible( );
	}

	void Selected( void ) const
	{
		GetINSHUDHelper( )->ReinforcementCall( );
	}
};

DECLARE_COMGROUP( COMGROUP_REINFORCE, CReinforceGroup );

//=========================================================
//=========================================================
#define MAX_OBJORDERLIST 3

class CObjOrderGroup : public CGroupSingleIcon, public CGroupCommander
{
	DECLARE_GROUPBASE( CObjOrder, CGroupSingleIcon );

private:
	const char *Icon( void ) const
	{
		return "objorder";
	}

	bool IsVisible( void ) const
	{
		return CGroupCommander::IsVisible( );
	}

	void Reset( void )
	{
		m_OrderObj.Purge( );
	}

	void SetupGroup( CommsGroup_t &Group )
	{
		// TODO: need a better way of "selecting" objs to attack

		BaseClass::SetupGroup( Group );

		C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer( );

		if( !pPlayer )
			return;

		// list through all objs
		const CUtlVector< C_INSObjective* > &ObjList = C_INSObjective::GetObjectiveList( );
		char szOption[ 64 ];
		int iObjCount = 0;

		for( int i = 0; i < ObjList.Count( ) && iObjCount < MAX_OBJORDERLIST; i++ )
		{
			C_INSObjective *pObj = ObjList[ i ];
			Assert( pObj );
			
			// ... ensure valid
			if( !pObj || !pObj->IsOrdersAllowed( ) )
				continue;

			// ... ensure it's not the current order
			const CObjOrder *pObjOrder = pPlayer->GetObjOrders( );

			if( pObjOrder )
			{
				if( pObj == pObjOrder->Objective( ) )
					continue;
			}

			// ... find ordertype
			int iOrderType = UTIL_CaculateObjType( pPlayer->GetTeamID( ), pObj );

			if( iOrderType == ORDERTYPE_OBJ_NONE )
				continue;

			// ... add to list
			Q_snprintf( szOption, sizeof( szOption ), "%s Objective %c", g_pszObjOrderTypeNames[ iOrderType ], pObj->GetPhonetischLetter( ) );
			Group.AddOption( szOption, false );

			// ... store away
			m_OrderObj.AddToTail( pObj->GetOrderID( ) );
			iObjCount++;
		}
	}

	void Selected( int iID )
	{
		if( !m_OrderObj.IsValidIndex( iID ) )
			return;

		GetINSHUDHelper( )->ObjectiveOrder( m_OrderObj[ iID ] );
	}

private:
	CUtlVector< int > m_OrderObj;

	CommsTex_t m_Icons[ ALPHABET_SIZE ];
};

DECLARE_COMGROUP( COMGROUP_OBJORDER, CObjOrderGroup );

//=========================================================
//=========================================================
class CUnitOrderGroup : public CGroupSingle, public CGroupCommander
{
	DECLARE_GROUPBASE( CUnitOrder, CGroupSingle );

private:
	const char *Icon( void ) const
	{
		return "unitorder";
	}

	const char *Option( void ) const
	{
		return "Set Waypoint";
	}

	bool IsVisible( void ) const
	{
		return CGroupCommander::IsVisible( );
	}

	void Selected( void ) const
	{
		GetINSHUDHelper( )->UnitOrderStart( );
	}
};

DECLARE_COMGROUP( COMGROUP_UNITORDER, CUnitOrderGroup );

//=========================================================
//=========================================================
class CPlayerOrderGroup : public CGroupSingleIcon, public CGroupCommander
{
	DECLARE_GROUPBASE( CPlayerOrderGroup, CGroupSingleIcon );

private:
	const char *Icon( void ) const
	{
		return "porder";
	}

	bool IsVisible( void ) const
	{
		return CGroupCommander::IsVisible( );
	}

	void SetupGroup( CommsGroup_t &Group )
	{
		static const char *pszOrders[ PORDER_COUNT ] = {
			"Flank Left",					// PORDER_FLANKLEFT
			"Flank Right",					// PORDER_FLANKRIGHT
			"Move",							// PORDER_MOVING
			"Take Cover",					// PORDER_TAKECOVER
			"Covering Fire",				// PORDER_COVERINGFIRE
			"Hold Fire",					// PORDER_HOLDFIRE
			"Return Fire"					// PORDER_RETURNFIRE
		};

		BaseClass::SetupGroup( Group );

		for( int i = 0; i < PORDER_COUNT; i++ )
			Group.AddOption( pszOrders[ i ], true );
	}

	void Selected( int iID )
	{
		if( !UTIL_ValidPlayerOrder( iID ) )
			return;

		GetINSHUDHelper( )->PlayerOrder( iID );
	}
};

DECLARE_COMGROUP( COMGROUP_PORDER, CPlayerOrderGroup );

//=========================================================
//=========================================================
class CPlayerOrderResponseGroup : public CGroupSingleIcon, public CGroupCommander
{
	DECLARE_GROUPBASE( CPlayerOrderResponseGroup, CGroupSingleIcon );

private:
	const char *Icon( void ) const
	{
		return "porder";
	}

	bool IsVisible( void ) const
	{
		return ( PlayerOrder( ) != INVALID_PORDER );
	}

	void SetupGroup( CommsGroup_t &Group )
	{
		static const char *pszPlayerResponse[ PORDER_COUNT ][ PORESPONSE_COUNT ] = 
		{
			{ { "Flanking Left" }, { "In Position" }, { "Cannot Flank" } },			// PORDER_FLANKLEFT
			{ { "Flanking Right" }, { "In Position" }, { "Cannot Flank" } },		// PORDER_FLANKRIGHT
			{ { "Moving" }, { "In Position" }, { "That's Suicide" } },				// PORDER_MOVING
			{ { "Looking" }, { "In Position" }, { "Cannot Move" } },				// PORDER_TAKECOVER
			{ { "Covering Fire" }, { "Pinned" }, { "Cannot Provide" } },			// PORDER_COVERINGFIRE
			{ { "Holding Fire" }, { "Fire Held" }, { "Cannot Hold" } },				// PORDER_HOLDFIRE
			{ { "Returning Fire" }, { "Fire Returned" }, { "Cannot Return" } }		// PORDER_RETURNFIRE
		};

		BaseClass::SetupGroup( Group );

		// find player order
		int iPlayerOrder = PlayerOrder( );

		if( iPlayerOrder == INVALID_PORDER )
			return;

		// add them
		for( int i = 0; i < PORESPONSE_COUNT; i++ )
			Group.AddOption( pszPlayerResponse[ iPlayerOrder ][ i ], true );
	}

	void Selected( int iID )
	{
		if( !UTIL_ValidPlayerOrderResponse( iID ) )
			return;

		GetINSHUDHelper( )->PlayerOrderResponse( iID );
	}

	int PlayerOrder( void ) const
	{
		int iPlayerOrder = INVALID_PORDER;

		C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer( );

		if( pPlayer && !pPlayer->IsCommander( ) )
			iPlayerOrder = pPlayer->GetPlayerOrder( );

		return iPlayerOrder;
	}
};

DECLARE_COMGROUP( COMGROUP_POREPONSE, CPlayerOrderResponseGroup );

//=========================================================
//=========================================================
class CPlayerStatusGroup : public CGroupSingleIcon
{
	DECLARE_GROUPBASE( CPlayerStatusGroup, CGroupSingleIcon );

private:
	const char *Icon( void ) const
	{
		return "status";
	}

	void SetupGroup( CommsGroup_t &Group )
	{
		BaseClass::SetupGroup( Group );

		const char *pszText = NULL;

		C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer( );

		if( pPlayer )
		{
			int iStatusType, iStatusID;

			if( pPlayer->GetStatus( iStatusType, iStatusID ) )
			{
				const StatusProgressiveData_t *pStatusData = UTIL_StatusProgressiveData( iStatusType, iStatusID );

				if( pStatusData )
					pszText = pStatusData->pszText;
			}
		}

		Group.AddOption( pszText, false );
	}

	void Selected( int iID )
	{
		GetINSHUDHelper( )->StatusBroadcast( );
	}
};

DECLARE_COMGROUP( COMGROUP_STATUS, CPlayerStatusGroup );

//=========================================================
//=========================================================
class CNeedHelpGroup : public CGroupSingle, public CGroupCommander
{
	DECLARE_GROUPBASE( CUnitOrder, CGroupSingle );

private:
	const char *Icon( void ) const
	{
		return "help";
	}

	const char *Option( void ) const
	{
		return "Need Help!";
	}

	void Selected( void ) const
	{
		GetINSHUDHelper( )->PlayerHelp( );
	}
};

DECLARE_COMGROUP( COMGROUP_NEEDHELP, CNeedHelpGroup );

};