//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// $NoKeywords: $
//=============================================================================//

#ifndef INS_INVENTORY_SHARED_H
#define INS_INVENTORY_SHARED_H

#ifdef _WIN32
#pragma once
#endif

#include "weapon_defines.h"

//=========================================================
//=========================================================

// PNOTE: the base inventory code does not store any item data
// it only provides an abstract interface to transfer it around

//=========================================================
//=========================================================
struct ItemWeaponData_t
{
	int m_iID;
	int m_iClipCount;
	int m_iAmmoCount;
};

struct ItemEquipmentData_t
{
	int m_iID;
	int m_iCount;
};

//=========================================================
//=========================================================
class CInventoryBlueprint
{
public:
	void AddWeapon( int iID, int iClipCount, int iAmmoCount );
	void AddEquipment( int iID, int iCount );

	int GetWeaponCount( int iWeaponType ) const;
	const ItemWeaponData_t &GetWeapon( int iWeaponType, int iID ) const;

	int GetEquipmentCount( void );
	ItemEquipmentData_t &GetEquipment( int iID );

protected:
	CUtlVector< ItemWeaponData_t > m_Weapons[ WEAPONTYPE_MAJOR_COUNT ];
	CUtlVector< ItemEquipmentData_t > m_Equipment;
};

//=========================================================
//=========================================================
class IInventoryManager
{
public:

#ifdef GAME_DLL

	virtual bool AddWeapon( int iWeaponID, int iClipCount, int iAmmoCount ) = 0;
	
#endif

};

#endif // INS_INVENTORY_SHARED_H