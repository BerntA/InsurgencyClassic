//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef INS_PROFILEMANAGER_H
#define INS_PROFILEMANAGER_H
#ifdef _WIN32
#pragma once
#endif

//=========================================================
//=========================================================
class CINSProfileManager : public CBaseEntity
{
public:
	DECLARE_CLASS( CINSProfileManager, CBaseEntity );
	DECLARE_DATADESC( );

	void FireIMCOutput( void );

private:
	COutputEvent m_OnProfile0, m_OnProfile1, m_OnProfile2, m_OnProfile3;
	COutputEvent m_OnProfile4, m_OnProfile5, m_OnProfile6, m_OnProfile7;
};

#endif // INS_PROFILEMANAGER_H
