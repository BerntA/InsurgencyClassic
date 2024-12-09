//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// $NoKeywords: $
//
//=============================================================================//

#include "cbase.h"
#include "definedsquads.h"
#include "team_lookup.h"
#include "filesystem.h"
#include "ins_squad_shared.h"
#include "imc_loader_k.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
bool LoadDefinedSlot(const char *pszClassName, int iTeamID, int iSlotID, SlotData_t &Data);

//=========================================================
//=========================================================
bool LoadDefinedSquad(const char *pszFileName, int iTeamID, char *pszName, SlotData_t &Data)
{
	CTeamLookup *pTeamLookup = GetTeamLookup(iTeamID);

	if(!pTeamLookup)
		return false;

	char szFilePath[256];
	Q_snprintf(szFilePath, sizeof(szFilePath), "scripts/dsquads/%s_%s", pTeamLookup->GetFileName(), pszFileName);

	KeyValues *pSquadData = ReadEncryptedKVFile( ::filesystem, szFilePath, GetEncryptionKey( ) );

	if( !pSquadData )
		return false;

	if( LoadDefinedSquadData( pSquadData, iTeamID, pszName, Data ) )
		return true;

	pSquadData->deleteThis( );

	return false;
}

//=========================================================
//=========================================================
bool LoadDefinedSquadData( KeyValues *pSquad, int iTeamID, char *pszSquadName, SlotData_t &Data )
{
	if( !pSquad )
		return false;

	// load name
	const char *pszLoadedSquadName = pSquad->GetString( "name", NULL );

	if( !pszLoadedSquadName )
		return false;

	Q_strncpy( pszSquadName, pszLoadedSquadName, MAX_SQUADNAME_LENGTH );

	// find the team type and squad data
	CTeamLookup *pTeam = GetTeamLookup( iTeamID );
	KeyValues *pSlotData = pSquad->FindKey( SLOT_DATA );

	if( !pTeam || !pSlotData )
		return false;

	KeyValues *pSlot = pSlotData->GetFirstSubKey( );

	for( ; pSlot; pSlot = pSlot->GetNextKey( ) )
	{
		const char *pszName = pSlot->GetName( );

		for( int i = 1; i < MAX_SQUAD_SLOTS; i++ )
		{
			if( Q_stricmp( pszName, g_pszSquadSlotNames[ i ] ) != 0 )
				continue;

			LoadDefinedSlot( pSlot->GetString( ), iTeamID, i, Data );
			break;
		}
	}

	return true;
}

//=========================================================
//=========================================================
bool LoadDefinedSlot(const char *pszClassName, int iTeamID, int iSlotID, SlotData_t &Data)
{
	if(!pszClassName || iSlotID == INVALID_SLOT)
		return false;

	CTeamLookup *pTeamLookup = GetTeamLookup(iTeamID);

	if(!pTeamLookup)
		return false;

	int iClassID = pTeamLookup->FindClass(pszClassName);

	if(iClassID == INVALID_CLASS)
	{
		AssertMsg(false, "CTeamLookup::FindClass failed in LoadDefinedSlot");
		return false;
	}

	// set the class
	Data[iSlotID] = iClassID;

	return true;
}
