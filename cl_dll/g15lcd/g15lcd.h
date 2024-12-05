//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef G15LCD_INTERFACE_H
#define G15LCD_INTERFACE_H
#ifdef _WIN32
#pragma once
#endif

//=========================================================
//=========================================================
class CG15LCDInterface : public CAutoGameSystem
{
public:
	CG15LCDInterface( );

	bool IsInit( void ) const { return m_bInit; }

private:
	bool Init( void );
	void Shutdown( void );

private:
	bool m_bInit;
};

//=========================================================
//=========================================================
extern CG15LCDInterface g_G15LCDInterface;

#endif // TRACKIR_INTERFACE_H