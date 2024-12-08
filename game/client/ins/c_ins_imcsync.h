//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//=============================================================================//

#ifndef C_INS_IMCSYNC_H
#define C_INS_IMCSYNC_H
#ifdef _WIN32
#pragma once
#endif

#include "imc_format.h"

//=========================================================
//=========================================================
class C_IMCSync : public C_BaseEntity
{
	DECLARE_CLASS( C_IMCSync, C_BaseEntity );
	DECLARE_CLIENTCLASS( );

public:
	C_IMCSync( );
	virtual ~C_IMCSync( );

	int GetVersion( void ) const { return m_iVersion; }
	bool IsOffical( void ) const { return m_bOffical; }
	int GetRoundLength( void ) const { return m_iRoundLength; }
	bool HasCustomGravity( void ) const { return m_bHasCustomGravity; }

private:
	int m_iVersion;
	bool m_bOffical;
	int m_iRoundLength;
	bool m_bHasCustomGravity;
};

//=========================================================
//=========================================================
extern C_IMCSync *IMCSync( void );

#endif // C_INS_IMCSYNC_H
