//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "pain_helper.h"
#include "c_ins_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
class CPainTypeLength : public CPainType
{
public:
	CPainTypeLength::CPainTypeLength( )
	{
		m_flEndTime = m_flInvLength = 0.0f;
	}

protected:
	void SetLength( float flLength )
	{
		m_flEndTime = gpGlobals->curtime + flLength;
		m_flInvLength = 1 / flLength;
	}

private:
	bool ShouldUpdate( void ) const
	{
		return ( m_flEndTime > gpGlobals->curtime );
	}

	void Update( void )
	{
		float flFactor = ( ( m_flEndTime - gpGlobals->curtime ) * m_flInvLength );

		SetAlphaLevel( flFactor * GetAlphaFactor( ) );
		SetBlurLevel( flFactor * GetBlurFactor( ) );
		SetHazeLevel( flFactor * GetHazeFactor( ) );
	}

	virtual float GetAlphaFactor( void ) const = 0;
	virtual float GetBlurFactor( void ) const = 0;
	virtual float GetHazeFactor( void ) const = 0;
	virtual bool UsingMotionBlur( void ) const { return false; }

private:
	float m_flEndTime;
	float m_flInvLength;
};

//=========================================================
//=========================================================
class CPainTypeDmg : public CPainTypeLength
{
private:
	void DoInit( void )
	{
		int iDamage = 0;

		if( GetParameter( "damage", iDamage ) )
			SetLength( clamp( iDamage * GetDmgFactor( ), GetTimeMin( ), GetTimeMax( ) ) );
	}

protected:
	virtual float GetDmgFactor( void ) const = 0;
	virtual float GetTimeMin( void ) const = 0;
	virtual float GetTimeMax( void ) const = 0;
};

//=========================================================
//=========================================================
class CPainTypePMinor : public CPainTypeDmg
{
private:
	float GetDmgFactor( void ) const { return 0.25f; }
	float GetTimeMin( void ) const { return 0.5f; }
	float GetTimeMax( void ) const { return 1.0f; }

	float GetAlphaFactor( void ) const { return 0.5f; }
	float GetBlurFactor( void ) const { return 0.3f; }
	float GetHazeFactor( void ) const { return 0.0f; }
};

DEFINE_PAINTYPE( PAINTYPE_PMINOR, CPainTypePMinor );

//=========================================================
//=========================================================
class CPainTypePMajor : public CPainTypeDmg
{
private:
	float GetDmgFactor( void ) const { return 0.25f; }
	float GetTimeMin( void ) const { return 0.75f; }
	float GetTimeMax( void ) const { return 2.0f; }

	float GetAlphaFactor( void ) const { return 0.5f; }
	float GetBlurFactor( void ) const { return 0.5f; }
	float GetHazeFactor( void ) const { return 0.1f; }
	bool GetUseMotionBlur(void) const { return true; }
};

DEFINE_PAINTYPE( PAINTYPE_PMAJOR, CPainTypePMajor );

//=========================================================
//=========================================================
#define CMINOR_LENGTH 4.0f

class CPainTypeCMinor : public CPainTypeLength
{
public:
	virtual void DoInit( void )
	{
		SetLength( CMINOR_LENGTH );
	}

protected:
	float GetAlphaFactor(void) const { return 0.25f; }
	float GetBlurFactor(void) const { return 0.25f; }
	float GetHazeFactor(void) const { return 0.0f; }
};

DEFINE_PAINTYPE( PAINTYPE_CMINOR, CPainTypeCMinor );

//=========================================================
//=========================================================
#define LMINOR_LENGTH 2.0f

class CPainTypeCLight : public CPainTypeCMinor
{
public:
	void DoInit( void )
	{
		SetLength( LMINOR_LENGTH );
	}
};

DEFINE_PAINTYPE( PAINTYPE_CLIGHT, CPainTypeCLight );

//=========================================================
//=========================================================
class CPainTypeCMajor : public CPainTypeDmg
{
protected:
	float GetDmgFactor( void ) const { return 1.75f; }
	float GetTimeMin( void ) const { return 0.75f; }
	float GetTimeMax( void ) const { return 2.25f; }

	float GetAlphaFactor(void) const { return 0.0f; }
	float GetBlurFactor(void) const { return 0.75f; }
	float GetHazeFactor(void) const { return 0.0f; }
	bool UsingMotionBlur(void) const { return true; }
};

DEFINE_PAINTYPE( PAINTYPE_CMAJOR, CPainTypeCMajor );

//=========================================================
//=========================================================
class CPainTypeDeath : public CPainType
{
protected:
	bool ShouldUpdate( void ) const
	{
		return true;
	}

	void Update( void )
	{
		CPainType::Update( );

		SetAlphaLevel( 0.5f );
		SetBlurLevel( 0.5f );
	}

	bool UsingMotionBlur( void ) const { return true; }
};

DEFINE_PAINTYPE( PAINTYPE_DEATH, CPainTypeDeath );

//=========================================================
//=========================================================
#define BWHIZ_LENGTH 0.25f

class CPainTypeBulletWhiz : public CPainTypeLength
{
public:
	void DoInit( void )
	{
		SetLength( BWHIZ_LENGTH );
	}

protected:
	float GetAlphaFactor(void) const { return 0.25f; }
	float GetBlurFactor(void) const { return 0.0f; }
	float GetHazeFactor(void) const { return 0.0f; }
};

DEFINE_PAINTYPE( PAINTYPE_BWHIZ, CPainTypeBulletWhiz );
