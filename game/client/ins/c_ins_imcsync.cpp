//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Team management class. Contains all the details for a specific team
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "c_ins_imcsync.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================

// PNOTE: was going to use OnDataChanged created handle
// to actually mod the IMCConfig values - but HasCustomGrav
// is a special case and I want the error checking!

//=========================================================
//=========================================================
IMPLEMENT_CLIENTCLASS_DT( C_IMCSync, DT_IMCSync, CIMCSync )

	RecvPropInt( RECVINFO( m_iVersion ) ),
	RecvPropBool( RECVINFO( m_bOffical ) ),
	RecvPropInt( RECVINFO( m_iRoundLength ) ),
	RecvPropBool( RECVINFO( m_bHasCustomGravity ) ),
	
END_RECV_TABLE( )

//=========================================================
//=========================================================
C_IMCSync *g_pIMCSync = NULL;

C_IMCSync::C_IMCSync( )
{
	Assert( !g_pIMCSync );
	g_pIMCSync = this;
}

//=========================================================
//=========================================================
C_IMCSync::~C_IMCSync( )
{
	g_pIMCSync = NULL;
}

//=========================================================
//=========================================================
C_IMCSync *IMCSync( void )
{
	Assert( g_pIMCSync );
	return g_pIMCSync;
}