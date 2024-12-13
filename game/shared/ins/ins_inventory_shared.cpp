//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "ins_inventory_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
int CInventoryBlueprint::GetWeaponCount( int iWeaponID ) const
{
	return m_Weapons[ iWeaponID ].Count( );
}

//=========================================================
//=========================================================
const ItemWeaponData_t &CInventoryBlueprint::GetWeapon( int iWeaponID, int iID ) const
{
	return m_Weapons[ iWeaponID ].Element( iID );
}

//=========================================================
//=========================================================
int CInventoryBlueprint::GetEquipmentCount( void )
{
	return m_Equipment.Count( );
}

//=========================================================
//=========================================================
ItemEquipmentData_t &CInventoryBlueprint::GetEquipment( int iID )
{
	return m_Equipment[ iID ];
}

//=========================================================
//=========================================================
void CInventoryBlueprint::AddWeapon( int iID, int iClips, int iAmmoCount )
{
	int iType = WeaponIDToType( iID );

	if( iType != WEAPONTYPE_PRIMARY && iType != WEAPONTYPE_SECONDARY )
		return;

	CUtlVector< ItemWeaponData_t > &Weapons = m_Weapons[ iType ];
	int iIndex = Weapons.AddToTail( );

	if( !Weapons.IsValidIndex( iIndex ) )
		return;

	ItemWeaponData_t &WeaponData = Weapons[ iIndex ];
	WeaponData.m_iID = iID;
	WeaponData.m_iClipCount = iClips;
	WeaponData.m_iAmmoCount = iAmmoCount;
}

//=========================================================
//=========================================================
void CInventoryBlueprint::AddEquipment( int iID, int iCount )
{
	int iIndex = m_Equipment.AddToTail( );

	if( !m_Equipment.IsValidIndex( iIndex ) )
		return;

	ItemEquipmentData_t &EquipmentData = m_Equipment[ iIndex ];
	EquipmentData.m_iID = iID;
	EquipmentData.m_iCount = iCount;	
}
