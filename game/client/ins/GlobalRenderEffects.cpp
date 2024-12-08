//=========       Copyright © Reperio Studios 2016 @ Bernt Andreas Eide!       ============//
//
// Purpose: Handle/Store Shared Effect Data
//
//========================================================================================//

#include "cbase.h"
#include "GlobalRenderEffects.h"
#include "view.h"
#include "view_scene.h"
#include "viewrender.h"
#include "c_playerresource.h"

void CGlobalRenderEffects::Initialize()
{
	if (m_bInitialized)
		return;

	m_bInitialized = true;

	m_MatBloodOverlay = materials->FindMaterial("effects/blood_overlay_1", TEXTURE_GROUP_CLIENT_EFFECTS);
	m_MatBloodOverlay->IncrementReferenceCount();

	m_MatPerkOverlay = materials->FindMaterial("effects/com_shield003a", TEXTURE_GROUP_CLIENT_EFFECTS);
	m_MatPerkOverlay->IncrementReferenceCount();

	m_MatSpawnProtectionOverlay = materials->FindMaterial("effects/spawnprotection_overlay", TEXTURE_GROUP_CLIENT_EFFECTS);
	m_MatSpawnProtectionOverlay->IncrementReferenceCount();

	m_MatCloakOverlay = materials->FindMaterial("effects/cloak_overlay", TEXTURE_GROUP_CLIENT_EFFECTS);
	m_MatCloakOverlay->IncrementReferenceCount();

	m_MatBleedOverlay = materials->FindMaterial("effects/bleed_overlay", TEXTURE_GROUP_CLIENT_EFFECTS);
	m_MatBleedOverlay->IncrementReferenceCount();

	m_MatBurnOverlay = materials->FindMaterial("effects/burn_overlay", TEXTURE_GROUP_CLIENT_EFFECTS);
	m_MatBurnOverlay->IncrementReferenceCount();

	m_MatIceOverlay = materials->FindMaterial("effects/frozen_overlay", TEXTURE_GROUP_CLIENT_EFFECTS);
	m_MatIceOverlay->IncrementReferenceCount();
}

void CGlobalRenderEffects::Shutdown()
{
	if (!m_bInitialized)
		return;

	m_bInitialized = false;

	m_MatBloodOverlay->DecrementReferenceCount();
	m_MatBloodOverlay = NULL;

	m_MatPerkOverlay->DecrementReferenceCount();
	m_MatPerkOverlay = NULL;

	m_MatSpawnProtectionOverlay->DecrementReferenceCount();
	m_MatSpawnProtectionOverlay = NULL;

	m_MatCloakOverlay->DecrementReferenceCount();
	m_MatCloakOverlay = NULL;

	m_MatBleedOverlay->DecrementReferenceCount();
	m_MatBleedOverlay = NULL;

	m_MatBurnOverlay->DecrementReferenceCount();
	m_MatBurnOverlay = NULL;

	m_MatIceOverlay->DecrementReferenceCount();
	m_MatIceOverlay = NULL;
}

static CGlobalRenderEffects g_GlobalRenderFX;
CGlobalRenderEffects* GlobalRenderEffects = &g_GlobalRenderFX;

void RenderMaterialOverlay(IMaterial* texture, int x, int y, int w, int h)
{
	if (texture == NULL)
		return;

	if (texture->NeedsFullFrameBufferTexture())
		DrawScreenEffectMaterial(texture, x, y, w, h);
	else if (texture->NeedsPowerOfTwoFrameBufferTexture())
	{
		UpdateRefractTexture(x, y, w, h, true);

		// Now draw the entire screen using the material...
		CMatRenderContextPtr pRenderContext(materials);
		ITexture* pTexture = GetPowerOfTwoFrameBufferTexture();
		int sw = pTexture->GetActualWidth();
		int sh = pTexture->GetActualHeight();
		// Note - don't offset by x,y - already done by the viewport.
		pRenderContext->DrawScreenSpaceRectangle(
			texture,
			0, 0, w, h,
			0, 0, sw - 1, sh - 1, sw, sh
		);
	}
	else
	{
		byte color[4] = { 255, 255, 255, 255 };
		render->ViewDrawFade(color, texture);
	}
}