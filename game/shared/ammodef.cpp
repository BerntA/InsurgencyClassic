//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//=============================================================================//

#include "cbase.h"
#include "ammodef.h"
#include "keyvalues.h"
#include "filesystem.h"
#include "weapon_defines.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
#define PATH_AMMODATA "scripts/weapons/ammodata"

//=========================================================
//=========================================================
AmmoTypeCreator_t g_AmmoTypeCreators[AMMOTYPE_COUNT];

CAmmoDef CAmmoDef::m_AmmoDef;

//=========================================================
//=========================================================
CAmmoDef::CAmmoDef()
{
	memset(m_pAmmoTypes, NULL, sizeof(m_pAmmoTypes));
}

//=========================================================
//=========================================================
bool CAmmoDef::Init(void)
{
	// load the types
	for (int i = 0; i < AMMOTYPE_COUNT; i++)
	{
		AmmoTypeCreator_t Creator = g_AmmoTypeCreators[i];
		Assert(Creator);

		m_pAmmoTypes[i] = (Creator)();
	}

	// load it
	if (!Load())
	{
		Assert(false);
		Warning("CAmmoDef::GetAmmoDef, Unable to Load Ammo Definitions\n");
	}

	return true;
}

//=========================================================
//=========================================================
bool CAmmoDef::Load(void)
{
	// load from file
	KeyValues* pAmmoData = ReadEncryptedKVFile(::filesystem, PATH_AMMODATA, GetEncryptionKey());

	if (!pAmmoData)
		return false;

	// load each part
	for (KeyValues* pDataType = pAmmoData->GetFirstSubKey(); pDataType; pDataType = pDataType->GetNextKey())
	{
		const char* pszDataType = pDataType->GetName();

		if (!pszDataType || pszDataType[0] == '\0')
			continue;

		for (int i = 0; i < AMMOTYPE_COUNT; i++)
		{
			IAmmoType* pAmmoType = m_pAmmoTypes[i];

			if (Q_strcmp(pszDataType, pAmmoType->GetHeader()) != 0)
				continue;

			for (KeyValues* pDataPart = pDataType->GetFirstSubKey(); pDataPart; pDataPart = pDataPart->GetNextKey())
			{
				const char* pszName = pDataPart->GetName();

				if (!pszName || pszName[0] == '\0')
					continue;

				int iDataID = pAmmoType->ConvertID(pszName);

				if (iDataID == INVALID_AMMODATA)
					continue;

				IAmmoData* pData = pAmmoType->GetData(iDataID);
				Assert(pData);

				pData->Parse(pDataPart);
			}
		}
	}

	pAmmoData->deleteThis();

	return true;
}

//=========================================================
//=========================================================
IAmmoType* CAmmoDef::GetAmmoType(int iType) const
{
	return m_pAmmoTypes[iType];
}

//=========================================================
//=========================================================
void CAmmoDef::Precache(void)
{
	for (int i = 0; i < AMMOTYPE_COUNT; i++)
	{
		IAmmoType* pAmmoType = m_pAmmoTypes[i];
		Assert(pAmmoType);

		for (int j = 0; j < pAmmoType->GetDataCount(); j++)
		{
			// precache ammo
			IAmmoData* pAmmoData = pAmmoType->GetData(j);
			Assert(pAmmoData);

			pAmmoData->Precache();
		}
	}
}

//=========================================================
//=========================================================
void CAmmoDef::LevelInitPostEntity(void)
{
	for (int i = 0; i < AMMOTYPE_COUNT; i++)
	{
		IAmmoType* pAmmoType = m_pAmmoTypes[i];
		Assert(pAmmoType);

		for (int j = 0; j < pAmmoType->GetDataCount(); j++)
		{
			IAmmoData* pAmmoData = pAmmoType->GetData(j);
			Assert(pAmmoData);

			pAmmoData->LevelInit();
		}
	}
}

//=========================================================
//=========================================================
CAmmoDef* CAmmoDef::GetAmmoDef(void)
{
	return &m_AmmoDef;
}

//=========================================================
//=========================================================
CAmmoDef* GetAmmoDef(void)
{
	return CAmmoDef::GetAmmoDef();
}