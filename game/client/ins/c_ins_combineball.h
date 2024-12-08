//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef C_INS_COMBINEBALL_H
#define C_INS_COMBINEBALL_H

#ifdef _WIN32
#pragma once
#endif

//=========================================================
//=========================================================
class C_INSCombineBall : public C_BaseAnimating
{
	DECLARE_CLASS( C_INSCombineBall, C_BaseAnimating );
	DECLARE_CLIENTCLASS( );

public:
	C_INSCombineBall( );

private:
	RenderGroup_t GetRenderGroup( void );

	void OnDataChanged( DataUpdateType_t UpdateType );
	int DrawModel( int iFlags );

	void DrawMotionBlur( void );
	void DrawFlicker( void );
	bool InitMaterials( void );

private:
	Vector m_vecLastOrigin;
	bool m_bEmit;
	float m_flRadius;

	IMaterial *m_pFlickerMaterial;
	IMaterial *m_pBodyMaterial;
	IMaterial *m_pBlurMaterial;
};

#endif // C_INS_COMBINEBALL_H