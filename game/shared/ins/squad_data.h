//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Shared Squad Header
//
// $NoKeywords: $
//=============================================================================//

#ifndef SQUAD_DATA_H
#define SQUAD_DATA_H
#ifdef _WIN32
#pragma once
#endif

//=========================================================
//=========================================================
typedef int EncodedSquadData_t;

class SquadData_t
{
#ifdef GAME_DLL

	friend class CPlayTeam;

#endif

public:
	SquadData_t( );
	SquadData_t( int iSquadID, int iSlotID );

	bool operator==( const SquadData_t &SquadData_t ) const;
	SquadData_t &operator=( const SquadData_t &SquadData );

	void Reset( void );

	bool IsValid( void ) const;
	
	int GetSquadID( void ) const { return m_iSquadID; }
	int GetSlotID( void ) const { return m_iSlotID; }

	void ParseEncodedInfo( EncodedSquadData_t EncodedSquadInfo );
	EncodedSquadData_t GetEncodedData( void ) const;

private:
	int m_iSquadID;
	int m_iSlotID;
};

#endif // SQUAD_DATA_H
