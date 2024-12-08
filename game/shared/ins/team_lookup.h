//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Lookup tables for the Team management. Contains all the details for a specific team
//
// $NoKeywords: $
//=============================================================================//

#ifndef TEAM_LOOKUP_H
#define TEAM_LOOKUP_H
#ifdef _WIN32
#pragma once
#endif

#include "imc_format.h"
#include "playercust.h"
#include "commander_shared.h"
#include "weapon_defines.h"
#include "ins_inventory_shared.h"

//=========================================================
//=========================================================
class KeyValues;

typedef char IMCString[ 256 ];

#define MODEL_STRING_LENGTH 128

class CPlayerClass;
class CWeaponRollout;

class CPlayerModelData
{
public:
	CPlayerModelData( );
	~CPlayerModelData( );

	void SetPath( const char *pszModel );
	void SetupCustomization( void );

	const char *GetPath( void ) const { return m_szPath; }
	modelcustomization_t &GetCustomization( void ) { return m_Customization; }

private:
	char m_szPath[ MODEL_STRING_LENGTH ];
	modelcustomization_t m_Customization;
};

enum TeamType_t
{
	TEAMTYPE_CONVENTIONAL = 0,
	TEAMTYPE_UNCONVENTIONAL,
	TEAMTYPE_COUNT
};

// NOTE: CIMCConfig::PreFinalise takes advantage of the order

enum ClassTypes_t
{
	CLASSTYPE_INVALID = -1,
	CLASSTYPE_NORMAL = 0,
	CLASSTYPE_COMMANDER,
	CLASSTYPE_MEDIC,
	CLASSTYPE_COUNT
};

struct DefinedPlayerClass_t
{
	DefinedPlayerClass_t( const char *pszName, int iType )
	{
		m_pszName = pszName;
		m_iType = iType;
	}

	const char *m_pszName;
	int m_iType;
};

//=========================================================
//=========================================================
struct RankNames_t
{
	RankNames_t( );

	const char *m_pszName;
	const char *m_pszShort;
};

//=========================================================
//=========================================================
class CTeamLookup
{
public:
	CTeamLookup( int iTeamClass );
	~CTeamLookup( );

	void Precache( void );
	void SetupCustomization( void );

	void SetType( int iType ) { m_iType = iType; }
	void SetName( const char *pszName ) { m_pszName = pszName; }
	void AddAdditionalName( const char *pszAdditionalName );
	void SetFullName( const char *pszFullName ) { m_pszFullName = pszFullName; }
	void SetFileName( const char *pszFileName ) { m_pszFileName = pszFileName; }
	void SetModel( const char *pszModel );
	void SetRankName( int iID, const char *pszName, const char *pszShort );

	bool AddClass( const DefinedPlayerClass_t &DefinedPlayerClass );
	void LoadClass( const DefinedPlayerClass_t &DefinedPlayerClass, KeyValues *pClass );

	void SetDefaultWeapon( int iWeapon ) { m_iDefaultWeapon = iWeapon; }

	void SetForeignLanguage( bool bState ) { m_bForeignLanguage = bState; }

	inline int GetType( void ) const { return m_iType; }
	inline const char *GetName( void ) const { return m_pszName; }
	inline const char *GetFullName( void ) const { return m_pszFullName; }
	inline const char *GetFileName( void ) const { return m_pszFileName; }
	CPlayerModelData *GetModelData( void ) { return &m_PlayerModelData; }

	const char *GetRandomName( void ) const;

	const char *GetFullRankName( int iID ) const;
	const char *GetRankName( int iID ) const;

	int GetDefaultWeapon( void ) const { return m_iDefaultWeapon; }

	bool UsesForeignLanguage( void ) const { return m_bForeignLanguage; }

	float GetWeightOffset( void ) const;

	bool IsValidClass(int iClass);
	CPlayerClass *GetClass(int iClass);
	int GetClassCount(void) { return m_Classes.Count(); }
	int FindClass(const char *pszClassName);

	void InitCommanderClass(void);
	int GetCommanderClass(void) const { return m_iCommanderClassID; }

private:
	int m_iType;

	const char *m_pszName;
	const char *m_pszFullName;
	const char *m_pszFileName;
	CUtlVector< const char* > m_AdditionalNames;

	RankNames_t m_RankNames[ RANK_COUNT ];

	CUtlVector< CPlayerClass* > m_Classes;
	int m_iCommanderClassID;

	CPlayerModelData m_PlayerModelData;

	int m_iDefaultWeapon;

	bool m_bForeignLanguage;
};

//=========================================================
//=========================================================
class CPlayerInventoryBlueprint : public CInventoryBlueprint
{
#ifdef GAME_DLL

public:
	void Write( CINSPlayer *pPlayer ) const;

private:
	void WriteWeapon( CINSPlayer *pPlayer, int iType ) const;

#endif
};

//=========================================================
//=========================================================
#define PLAYERCLASS_DESCRIPTION_LENGTH 2048

enum AmmoRolloutType_t
{
	AMMOTYPE_NONE = -1,
	AMMOTYPE_AMMO = 0,
	AMMOTYPE_CLIPS,
	MAX_AMMOTYPES
};

typedef int LayoutCustomisation_t[ WEAPONTYPE_MAJOR_COUNT ];

class CPlayerClass
{
public:
	CPlayerClass();
	~CPlayerClass();

	void Precache(void);
	void SetupCustomization(void);

	void SetName(const char *pszName);
	void SetFileName(const char *pszName);
	void SetModel(const char *pszName);
	void SetDescription(const char *pszDescription) { m_pszDescription = pszDescription; }
	void SetType(int iType) { m_iType = iType; }

	const char *GetName(void) { return m_szName; }	
	const char *GetFileName(void) { return m_szFileName; }
	CPlayerModelData *GetModelData( void ) { return m_pPlayerModelData; }
	const char *GetDescription(void) const { return m_pszDescription; }
	int GetType(void) const { return m_iType; }

	const CPlayerInventoryBlueprint &GetBlueprint( void ) const { return m_Blueprint; }

	void ParseInventory( KeyValues *pInventoryData );
	void ParseWeapons( KeyValues *pWeaponData, int iWeaponType );
	void ParseEquipment( KeyValues *pAmmoData );

private:
	IMCString m_szName, m_szFileName;
	CPlayerModelData *m_pPlayerModelData;
	const char *m_pszDescription;
	int m_iType;

	CPlayerInventoryBlueprint m_Blueprint;
};

//=========================================================
//=========================================================
class CPlayerClassHelper
{
public:
	int GetClassCount( void ) const { return m_Classes.Count( ); }
	const DefinedPlayerClass_t &GetClass( int iID ) const { return m_Classes[ iID ]; }

protected:
	CUtlVector< DefinedPlayerClass_t > m_Classes;
};

extern CPlayerClassHelper *g_TeamClassLookup[ MAX_LOOKUP_TEAMS ];

#define START_PLAYERCLASS_LOOKUP( teamtype ) \
	namespace PlayerClass__##teamtype { \
	class CPlayerClassHelper__##teamtype##__Helper : public CPlayerClassHelper { \
	public: \
		CPlayerClassHelper__##teamtype##__Helper( ) { \
			g_TeamClassLookup[ teamtype ] = this;
		
#define DEFINE_PLAYERCLASS( name, type ) \
	m_Classes.AddToTail( DefinedPlayerClass_t( name, type ) );

#define END_PLAYERCLASS_LOOKUP( ) \
		} \
	} g_PlayerClass; \
	};

typedef void ( *AddTeam_t )( CTeamLookup *pTeam );
extern AddTeam_t g_CreateTeamLookup[ MAX_LOOKUP_TEAMS ];

class CCreateTeamLookup
{
public:
	CCreateTeamLookup( AddTeam_t AddTeam, int iTeamType )
	{
		g_CreateTeamLookup[ iTeamType ] = AddTeam;
	}
};

#define START_TEAM_DEFINE( teamtype ) \
	void Add_##teamtype( CTeamLookup *pTeam ); \
	CCreateTeamLookup g_CreateTeamLookup__##teamtype##__Helper( Add_##teamtype, teamtype ); \
	void Add_##teamtype( CTeamLookup *pTeam ) {

#define END_TEAM_DEFINE( ) }

#define DEFINE_TYPE( type ) pTeam->SetType( type );
#define DEFINE_NAME( name ) pTeam->SetName( name );
#define DEFINE_ADDITIONALNAME( addname ) pTeam->AddAdditionalName( addname );
#define DEFINE_FULLNAME( fullname ) pTeam->SetFullName( fullname );
#define DEFINE_FILENAME( filename ) pTeam->SetFileName( filename );
#define DEFINE_MODEL( model ) pTeam->SetModel( model );
#define DEFINE_DEFAULTWEAPON( model ) pTeam->SetDefaultWeapon( model );
#define DEFINE_FOREIGNLANGUAGE( ) pTeam->SetForeignLanguage( true );
#define DEFINE_RANK( i, fullrank, rank ) pTeam->SetRankName( i, fullrank, rank );
#define DEFINE_RANK_PRIVATE( fullrank, rank ) DEFINE_RANK( RANK_PRIVATE, fullrank, rank )
#define DEFINE_RANK_LCORPORAL( fullrank, rank ) DEFINE_RANK( RANK_LCORPORAL, fullrank, rank )
#define DEFINE_RANK_CORPORAL( fullrank, rank ) DEFINE_RANK( RANK_CORPORAL, fullrank, rank )
#define DEFINE_RANK_SERGEANT( fullrank, rank ) DEFINE_RANK( RANK_SERGEANT, fullrank, rank )
#define DEFINE_RANK_LIEUTENANT( fullrank, rank ) DEFINE_RANK( RANK_LIEUTENANT, fullrank, rank )

//=========================================================
//=========================================================
extern CTeamLookup *g_pTeamLookup[MAX_LOOKUP_TEAMS];
extern void SetupTeamLookup(void);
extern void SetupTeamLookup_VidInit(void);
extern void ClearTeamLookup(void);
extern CTeamLookup *GetTeamLookup(int iTeamType);

#endif // TEAM_LOOKUP_H
