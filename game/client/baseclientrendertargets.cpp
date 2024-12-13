//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation for CBaseClientRenderTargets class.
//			Provides Init functions for common render textures used by the engine.
//			Mod makers can inherit from this class, and call the Create functions for
//			only the render textures the want for their mod.
//=============================================================================//

#include "cbase.h"
#include "baseclientrendertargets.h"						// header	
#include "materialsystem/imaterialsystemhardwareconfig.h"	// Hardware config checks
#include "tier0/icommandline.h"

ITexture* CBaseClientRenderTargets::CreateWaterReflectionTexture(IMaterialSystem* pMaterialSystem, int iSize)
{
	return pMaterialSystem->CreateNamedRenderTargetTextureEx2(
		"_rt_WaterReflection",
		iSize, iSize, RT_SIZE_PICMIP,
		pMaterialSystem->GetBackBufferFormat(),
		MATERIAL_RT_DEPTH_SHARED,
		TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT,
		CREATERENDERTARGETFLAGS_HDR);
}

ITexture* CBaseClientRenderTargets::CreateWaterRefractionTexture(IMaterialSystem* pMaterialSystem, int iSize)
{
	return pMaterialSystem->CreateNamedRenderTargetTextureEx2(
		"_rt_WaterRefraction",
		iSize, iSize, RT_SIZE_PICMIP,
		// This is different than reflection because it has to have alpha for fog factor.
		IMAGE_FORMAT_RGBA8888,
		MATERIAL_RT_DEPTH_SHARED,
		TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT,
		CREATERENDERTARGETFLAGS_HDR);
}

ITexture* CBaseClientRenderTargets::CreateCameraTexture(IMaterialSystem* pMaterialSystem, int iSize)
{
	return pMaterialSystem->CreateNamedRenderTargetTextureEx2(
		"_rt_Camera",
		iSize, iSize, RT_SIZE_DEFAULT,
		pMaterialSystem->GetBackBufferFormat(),
		MATERIAL_RT_DEPTH_SHARED,
		0,
		CREATERENDERTARGETFLAGS_HDR);
}

//-----------------------------------------------------------------------------
// Purpose: Called by the engine in material system init and shutdown.
//			Clients should override this in their inherited version, but the base
//			is to init all standard render targets for use.
// Input  : pMaterialSystem - the engine's material system (our singleton is not yet inited at the time this is called)
//			pHardwareConfig - the user hardware config, useful for conditional render target setup
//-----------------------------------------------------------------------------
void CBaseClientRenderTargets::InitClientRenderTargets(IMaterialSystem* pMaterialSystem, IMaterialSystemHardwareConfig* pHardwareConfig, int iWaterTextureSize, int iCameraTextureSize)
{
	// Water effects
	m_WaterReflectionTexture.Init(CreateWaterReflectionTexture(pMaterialSystem, iWaterTextureSize));
	m_WaterRefractionTexture.Init(CreateWaterRefractionTexture(pMaterialSystem, iWaterTextureSize));

	// Monitors
	m_CameraTexture.Init(CreateCameraTexture(pMaterialSystem, iCameraTextureSize));
}

//-----------------------------------------------------------------------------
// Purpose: Shut down each CTextureReference we created in InitClientRenderTargets.
//			Called by the engine in material system shutdown.
// Input  :  - 
//-----------------------------------------------------------------------------
void CBaseClientRenderTargets::ShutdownClientRenderTargets()
{
	// Water effects
	m_WaterReflectionTexture.Shutdown();
	m_WaterRefractionTexture.Shutdown();

	// Monitors
	m_CameraTexture.Shutdown();
}

class CTNERenderTargets : public CBaseClientRenderTargets
{
	DECLARE_CLASS_GAMEROOT(CTNERenderTargets, CBaseClientRenderTargets);

public:
	virtual void InitClientRenderTargets(IMaterialSystem* pMaterialSystem, IMaterialSystemHardwareConfig* pHardwareConfig);
	virtual void ShutdownClientRenderTargets();

	ITexture* CreateScopeTexture(IMaterialSystem* pMaterialSystem);

private:
	CTextureReference               m_ScopeTexture;
};

ITexture* CTNERenderTargets::CreateScopeTexture(IMaterialSystem* pMaterialSystem)
{
	return pMaterialSystem->CreateNamedRenderTargetTextureEx2(
		"_rt_Scope",
		1024, 1024, RT_SIZE_OFFSCREEN,
		pMaterialSystem->GetBackBufferFormat(),
		MATERIAL_RT_DEPTH_SHARED,
		TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT,
		CREATERENDERTARGETFLAGS_HDR);
}

//-----------------------------------------------------------------------------
// Purpose: Called by the engine in material system init and shutdown.
//                      Clients should override this in their inherited version, but the base
//                      is to init all standard render targets for use.
// Input  : pMaterialSystem - the engine's material system (our singleton is not yet inited at the time this is called)
//                      pHardwareConfig - the user hardware config, useful for conditional render target setup
//-----------------------------------------------------------------------------
void CTNERenderTargets::InitClientRenderTargets(IMaterialSystem* pMaterialSystem, IMaterialSystemHardwareConfig* pHardwareConfig)
{
	m_ScopeTexture.Init(CreateScopeTexture(pMaterialSystem));

	// Water effects & camera from the base class (standard HL2 targets) 
	BaseClass::InitClientRenderTargets(pMaterialSystem, pHardwareConfig);
}

//-----------------------------------------------------------------------------
// Purpose: Shut down each CTextureReference we created in InitClientRenderTargets.
//                      Called by the engine in material system shutdown.
// Input  :  - 
//-----------------------------------------------------------------------------
void CTNERenderTargets::ShutdownClientRenderTargets()
{
	m_ScopeTexture.Shutdown();

	// Clean up standard HL2 RTs (camera and water) 
	BaseClass::ShutdownClientRenderTargets();
}

static CTNERenderTargets g_TNERenderTargets;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CTNERenderTargets, IClientRenderTargets, CLIENTRENDERTARGETS_INTERFACE_VERSION, g_TNERenderTargets);