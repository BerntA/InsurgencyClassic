//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "ins_profilemanager.h"
#include "imc_config.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
LINK_ENTITY_TO_CLASS( ins_profilemanager, CINSProfileManager );

BEGIN_DATADESC( CINSProfileManager )

	DEFINE_OUTPUT( m_OnProfile0, "OnProfile0" ),
	DEFINE_OUTPUT( m_OnProfile1, "OnProfile1" ),
	DEFINE_OUTPUT( m_OnProfile2, "OnProfile2" ),
	DEFINE_OUTPUT( m_OnProfile3, "OnProfile3" ),
	DEFINE_OUTPUT( m_OnProfile4, "OnProfile4" ),
	DEFINE_OUTPUT( m_OnProfile5, "OnProfile5" ),
	DEFINE_OUTPUT( m_OnProfile6, "OnProfile6" ),
	DEFINE_OUTPUT( m_OnProfile7, "OnProfile7" ),

END_DATADESC()

//=========================================================
//=========================================================
void CINSProfileManager::FireIMCOutput( void )
{
	if( !IMCConfig( ) )
		return;

	switch( IMCConfig( )->GetProfileID( ) )
	{
		case 0:
			m_OnProfile0.FireOutput( NULL, this );
			break;

		case 1:
			m_OnProfile1.FireOutput( NULL, this );
			break;

		case 2:
			m_OnProfile2.FireOutput( NULL, this );
			break;

		case 3:
			m_OnProfile3.FireOutput( NULL, this );
			break;

		case 4:
			m_OnProfile4.FireOutput( NULL, this );
			break;

		case 5:
			m_OnProfile5.FireOutput( NULL, this );
			break;

		case 6:
			m_OnProfile6.FireOutput( NULL, this );
			break;

		case 7:
			m_OnProfile7.FireOutput( NULL, this );
			break;
	}
}
