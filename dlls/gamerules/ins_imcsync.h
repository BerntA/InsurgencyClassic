//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//
//=============================================================================//

#ifndef INS_IMCSYNC_H
#define INS_IMCSYNC_H
#ifdef _WIN32
#pragma once
#endif

//=========================================================
//=========================================================
class CIMCSync : public CBaseEntity
{
public:
	DECLARE_CLASS( CIMCSync, CBaseEntity );
	DECLARE_SERVERCLASS( );

	CIMCSync( );

	static void Create( void );

private:
	int UpdateTransmitState( void );

private:
	CNetworkVar( int, m_iVersion );
	CNetworkVar( bool, m_bOffical );
	CNetworkString( m_szMapName, MAX_MAPNAME_LENGTH );
	CNetworkVar( int, m_iRoundLength );
	CNetworkVar( bool, m_bHasCustomGravity );
};

#endif // INS_IMCSYNC_H