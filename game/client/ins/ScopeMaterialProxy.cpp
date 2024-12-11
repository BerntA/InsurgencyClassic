// Base includes required for Material Proxies
#include "cbase.h"
#include "materialsystem/imaterialproxy.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/itexture.h"
#include <keyvalues.h>
#include "mathlib/vmatrix.h"
#include "functionproxy.h"

// Render Targets
#include "rendertexture.h"

// Memory Debugger Thingy
#include "tier0/memdbgon.h"

extern IVEngineClient *engine;


class CScopeMaterialProxy : public IMaterialProxy
{
public:
                 CScopeMaterialProxy();
    virtual     ~CScopeMaterialProxy();

    virtual bool Init( IMaterial *pMaterial, KeyValues *pKeyValues );
    virtual void Release( void );

    virtual void OnBind( void *pC_BaseEntity );

private:
    // Scope "Geometry" hack
    float        m_flScopeGeometry;

    // Hideous, evil hack:
    IMaterialVar *m_pTextureRT;
    IMaterialVar *m_pTextureCrosshair;

    // Stuff from TextureScrollProxy
    IMaterialVar *m_pTextureScrollVar;
	CFloatInput m_TextureScale;
};

CScopeMaterialProxy::CScopeMaterialProxy()
{
    m_pTextureRT = NULL;
    m_pTextureCrosshair = NULL;
    m_pTextureScrollVar = NULL;
}
CScopeMaterialProxy::~CScopeMaterialProxy()
{
}

bool CScopeMaterialProxy::Init( IMaterial *pMaterial, KeyValues *pKeyValues )
{

    // From TextureScroll
    char const* pScrollVarName = pKeyValues->GetString( "textureScrollVar" );
	if( !pScrollVarName )
		return false;

	bool foundVar;
	m_pTextureScrollVar = pMaterial->FindVar( pScrollVarName, &foundVar, false );
	if( !foundVar )
		return false;

	m_TextureScale.Init( pMaterial, pKeyValues, "textureScale", 1.0f );

    // To reset texture.
    m_pTextureRT = pMaterial->FindVar( "$basetexture", &foundVar, true );
    if( !foundVar )
        return false;

    // Scope Geometry: 1 / sin( arctan( radius / ( 2 * length ) )
    m_flScopeGeometry = pKeyValues->GetFloat( "scopeGeometry", 48 );

    return true;
}

void CScopeMaterialProxy::Release( void )
{
    delete this;
}

void CScopeMaterialProxy::OnBind( void *pC_BaseEntity )
{
    float sOffset, tOffset;

    sOffset = 0.0f;
    tOffset = 0.0f;

    // This is a huge hack, it assumes that the scope is 1 foot long with a 1/2 inch radius.
    C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
    if( pPlayer )
    {
        C_BaseEntity *pViewModel = pPlayer->GetViewModel();
        if( pViewModel )
        {
            int iScopeOrigin = pViewModel->LookupAttachment( "scope_pov" );
            int iScopeAlign  = pViewModel->LookupAttachment( "scope_align" );
            if( iScopeAlign && iScopeOrigin )
            {
                QAngle angSink;
                Vector vecOrigin, vecAlign;
                Vector vecWeapon, vecPlayer; 

                // Get weapon angle
                pViewModel->GetAttachment( iScopeOrigin, vecOrigin, angSink );
                pViewModel->GetAttachment( iScopeAlign,  vecAlign, angSink );
                vecWeapon = ( vecAlign - vecOrigin );
                vecWeapon.NormalizeInPlace();

                // Get player angle.
                engine->GetViewAngles( angSink );
                AngleVectors( angSink, &vecPlayer );
                vecPlayer.NormalizeInPlace();
    
                vecAlign = vecWeapon.Cross( vecPlayer );
                sOffset = pow( vecAlign.x, 3 ) * pow( m_flScopeGeometry, 2 );
                tOffset = pow( vecAlign.y, 3 ) * pow( m_flScopeGeometry, 2 );
            }
        }
    }

    m_pTextureRT->SetTextureValue( GetZoomTexture() );

    if (m_pTextureScrollVar->GetType() == MATERIAL_VAR_TYPE_MATRIX)
	{
		VMatrix mat( m_TextureScale.GetFloat(), 0.0f, 0.0f, sOffset,
			0.0f, m_TextureScale.GetFloat(), 0.0f, tOffset,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f );
		m_pTextureScrollVar->SetMatrixValue( mat );
	}
	else
	{
		m_pTextureScrollVar->SetVecValue( sOffset, tOffset, 0.0f );
	} 
}

EXPOSE_INTERFACE( CScopeMaterialProxy, IMaterialProxy, "Scope" IMATERIAL_PROXY_INTERFACE_VERSION );