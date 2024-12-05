//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef C_INS_AMMOBOX_H
#define C_INS_AMMOBOX_H
#ifdef _WIN32
#pragma once
#endif

//=========================================================
//=========================================================
#define CINSAmmoBox C_INSAmmoBox

//=========================================================
//=========================================================
class C_INSAmmoBox : public C_BaseAnimating
{
	DECLARE_CLASS( C_INSAmmoBox, C_BaseAnimating );
	DECLARE_CLIENTCLASS( );

public:
	C_INSAmmoBox( );

private:
	int	ObjectCaps( void );

	int ActionType( void ) const;
};

#endif // C_INS_AMMOBOX_H
