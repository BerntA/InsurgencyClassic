//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//=============================================================================//

#ifndef AMMODEF_H
#define AMMODEF_H

#ifdef _WIN32
#pragma once
#endif

#include "weapon_defines.h"
#include "weapon_parse.h"

//=========================================================
//=========================================================
class IAmmoData
{
public:
	virtual void Parse(KeyValues* pData) = 0;

	virtual void Precache(void) {}
	virtual void LevelInit(void) {}
};

//=========================================================
//=========================================================
class IAmmoType
{
public:
	virtual const char* GetHeader(void) const = 0;

	virtual int ConvertID(const char* pszID) const = 0;

	virtual int GetDataCount(void) const = 0;
	virtual IAmmoData* GetData(int iID) = 0;
};

//=========================================================
//=========================================================
class CAmmoDef : public CAutoGameSystem
{
public:
	static CAmmoDef* GetAmmoDef(void);

	IAmmoType* GetAmmoType(int iType) const;

	void Precache(void);

private:
	CAmmoDef();

	bool Init(void);
	bool Load(void);

	void LevelInitPostEntity(void);

private:
	static CAmmoDef m_AmmoDef;

	IAmmoType* m_pAmmoTypes[AMMOTYPE_COUNT];
};

//=========================================================
//=========================================================
extern CAmmoDef* GetAmmoDef(void);

//=========================================================
//=========================================================
typedef IAmmoType* (*AmmoTypeCreator_t)(void);
extern AmmoTypeCreator_t g_AmmoTypeCreators[AMMOTYPE_COUNT];

class CAmmoTypeHelper
{
public:
	CAmmoTypeHelper(int iID, AmmoTypeCreator_t AmmoTypeCreator)
	{
		g_AmmoTypeCreators[iID] = AmmoTypeCreator;
	}
};

#define DECLARE_AMMOTYPE( id, ammotype ) \
	IAmmoType *CreateAmmoType__##id( void ) { \
	return new ammotype; } \
	CAmmoTypeHelper g_AmmoTypeHelper__##id( id, CreateAmmoType__##id );

#endif // AMMODEF_H