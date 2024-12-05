//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// $NoKeywords: $
//
//=============================================================================//

#include "cbase.h"
#include "ins_weaponcache_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
IMPLEMENT_NETWORKCLASS_ALIASED( INSWeaponCache, DT_INSWeaponCache )

BEGIN_NETWORK_TABLE( CINSWeaponCache, DT_INSWeaponCache )

#ifdef GAME_DLL

	SendPropInt( SENDINFO( m_iState ), WCACHESTATE_BITS ),

#else

	RecvPropInt( RECVINFO( m_iState ) ),

#endif

END_NETWORK_TABLE( )