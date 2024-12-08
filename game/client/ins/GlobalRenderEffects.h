//=========       Copyright © Reperio Studios 2016 @ Bernt Andreas Eide!       ============//
//
// Purpose: Handle/Store Shared Effect Data
//
//========================================================================================//

#ifndef C_GLOBAL_RENDER_FX_H
#define C_GLOBAL_RENDER_FX_H

#ifdef _WIN32
#pragma once
#endif

#include "model_types.h"
#include "filesystem.h"
#include "baseclientrendertargets.h"
#include "materialsystem/imaterialsystem.h"
#include "rendertexture.h"

class IMaterialSystem;
class IMaterialSystemHardwareConfig;

class CGlobalRenderEffects
{
public:
	void Initialize();
	void Shutdown();

	IMaterial* GetBloodOverlay(void) { return m_MatBloodOverlay; }
	IMaterial* GetPerkOverlay(void) { return m_MatPerkOverlay; }
	IMaterial* GetSpawnProtectionOverlay(void) { return m_MatSpawnProtectionOverlay; }
	IMaterial* GetCloakOverlay(void) { return m_MatCloakOverlay; }

	IMaterial* GetBleedOverlay(void) { return m_MatBleedOverlay; }
	IMaterial* GetBurnOverlay(void) { return m_MatBurnOverlay; }
	IMaterial* GetFrozenOverlay(void) { return m_MatIceOverlay; }

private:
	IMaterial* m_MatBloodOverlay;
	IMaterial* m_MatPerkOverlay;
	IMaterial* m_MatSpawnProtectionOverlay;
	IMaterial* m_MatCloakOverlay;

	IMaterial* m_MatBleedOverlay;
	IMaterial* m_MatBurnOverlay;
	IMaterial* m_MatIceOverlay;

	bool m_bInitialized;
};

extern CGlobalRenderEffects* GlobalRenderEffects;
extern void RenderMaterialOverlay(IMaterial* texture, int x, int y, int w, int h);

#endif // C_GLOBAL_RENDER_FX_H