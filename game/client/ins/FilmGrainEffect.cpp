//=========       Copyright � Reperio Studios 2024 @ Bernt Andreas Eide!       ============//
//
// Purpose: Film Grain Screen Space Effect.
//
//========================================================================================//

#include "cbase.h"
#include "ScreenSpaceEffects.h"
#include "GlobalRenderEffects.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar ins_fx_filmgrain("ins_fx_filmgrain", "0", FCVAR_ARCHIVE, "Enable or Disable film grain.", true, 0, true, 1);

class CFilmGrainEffect : public IScreenSpaceEffect
{
public:
	CFilmGrainEffect(void) {}

	void Init(void);
	void Shutdown(void);
	void SetParameters(KeyValues* params);
	void Enable(bool bEnable);
	bool IsEnabled() { return ins_fx_filmgrain.GetBool(); }
	void Render(int x, int y, int w, int h);

private:
	CMaterialReference m_FilmGrainMaterial;
};

ADD_SCREENSPACE_EFFECT(CFilmGrainEffect, filmgrain);

void CFilmGrainEffect::Init(void)
{
	m_FilmGrainMaterial.Init(materials->FindMaterial("effects/filmgrain", TEXTURE_GROUP_CLIENT_EFFECTS));
}

void CFilmGrainEffect::Shutdown(void)
{
	m_FilmGrainMaterial.Shutdown();
}

void CFilmGrainEffect::SetParameters(KeyValues* params)
{
}

void CFilmGrainEffect::Enable(bool bEnable)
{
}

void CFilmGrainEffect::Render(int x, int y, int w, int h)
{
	if (!IsEnabled())
		return;

	RenderMaterialOverlay(m_FilmGrainMaterial, x, y, w, h);
}