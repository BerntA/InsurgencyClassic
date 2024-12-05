//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "c_ins_playerlocaldata.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
BEGIN_RECV_TABLE_NOBASE( CINSPlayerLocalData, DT_INSLocal )

	RecvPropBool( RECVINFO( m_bAllowTeamChange ) ),

	RecvPropFloat( RECVINFO( m_flWeightFactor ) ),

	RecvPropFloat( RECVINFO( m_flLean ) ),

	RecvPropEHandle( RECVINFO( m_BandagingPlayer ) ),
	RecvPropFloat( RECVINFO( m_flBandageThreshold ) )

END_RECV_TABLE( )

//=========================================================
//=========================================================
BEGIN_PREDICTION_DATA_NO_BASE( CINSPlayerLocalData )

	DEFINE_PRED_FIELD( m_flLean, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),

	DEFINE_PRED_FIELD( m_BandagingPlayer, FIELD_EHANDLE, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_flBandageThreshold, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),

END_PREDICTION_DATA( )

//=========================================================
//=========================================================
CINSPlayerLocalData::CINSPlayerLocalData( )
{

}