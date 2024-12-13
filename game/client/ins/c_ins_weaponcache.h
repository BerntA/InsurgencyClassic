//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//
//=============================================================================//

#ifndef C_INS_WEAPONCACHE_H
#define C_INS_WEAPONCACHE_H
#ifdef _WIN32
#pragma once
#endif

#include "ins_weaponcache_shared.h"

//=========================================================
//=========================================================
class C_INSWeaponCache : public C_BaseAnimating
{
	DECLARE_CLASS( C_INSWeaponCache, C_BaseAnimating );
	DECLARE_CLIENTCLASS( );

public:
	int ActionType( void ) const;

private:
	int m_iState;
};

#endif // C_INS_WEAPONCACHE_H