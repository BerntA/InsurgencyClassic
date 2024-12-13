//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//

#ifndef PAINHELPER_H
#define PAINHELPER_H
#ifdef _WIN32
#pragma once
#endif

#include "pain_helper_shared.h"

//=========================================================
//=========================================================
class CPainElements
{
public:
	virtual void Reset( void );

	void SetAlphaLevel( float flLevel ) { m_flAlphaLevel = flLevel; }
	void SetBlurLevel( float flLevel ) { m_flBlurLevel = flLevel; }
	void SetHazeLevel( float flLevel ) { m_flHazeLevel = flLevel; }
	void SetMotionBlur( bool bState ) { m_bUseMotionBlur = bState; }

	float GetAlphaLevel( void ) const { return m_flAlphaLevel; }
	float GetBlurLevel( void ) const { return m_flBlurLevel; }
	float GetHazeLevel( void ) const { return m_flHazeLevel; }
	bool UsingMotionBlur( void ) const { return m_bUseMotionBlur; }

public:
	float m_flAlphaLevel;
	float m_flBlurLevel;
	float m_flHazeLevel;
	bool m_bUseMotionBlur;
};

//=========================================================
//=========================================================
// TODO: if we ever need a parameter system with different
// data types - make it kinda like ConVar's (with containers etc)

class CPainType : public CPainElements
{
public:
	CPainType( );

	void Reset( void );

	void SetParameter( const char *pszName, int iValue );

	bool IsFresh( void ) const { return m_bFresh; }
	void Init( void );

	virtual bool ShouldUpdate( void ) const = 0;
	virtual void Update( void );

protected:
	bool GetParameter( const char *pszName, int &iValue );

	virtual void DoInit( void ) { }

private:
	bool m_bFresh;

	CUtlMap< const char *, int > m_Parameters;	
};

//=========================================================
//=========================================================
typedef CPainType *( *PainTypeHelper_t )( void );

extern PainTypeHelper_t g_PainHelpers[ MAX_PAINTYPES ];

#define DEFINE_PAINTYPE( id, classname ) \
	CPainType *CreatePainType__##id( void ) { \
	return new classname;  } \
	class PainTypeHelper_##classname { \
	public: \
		PainTypeHelper_##classname( ) {	\
			g_PainHelpers[ id ] = CreatePainType__##id; \
		} \
	}; \
	PainTypeHelper_##classname g_PainTypeHelper__##classname;

//=========================================================
//=========================================================
class CPainHelper : public CPainElements, public CAutoGameSystem
{
public:
	bool Init( void );
	void Shutdown( void );

	CPainType *CreatePain( int iTypeID );
	
	bool AttemptUpdate( void );
	void Update( void );

	void FudgeValues( void );

private:
	bool CanDrawPain( void );

private:
	CPainType *m_pPainTypes[ MAX_PAINTYPES ];

	CUtlVector< int > m_Active;
};

//=========================================================
//=========================================================
extern CPainHelper g_PainHelper;

#endif // PAINHELPER_H
