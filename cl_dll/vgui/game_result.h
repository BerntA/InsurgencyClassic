//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef VGUI_GAMERESULT_H
#define VGUI_GAMERESULT_H
#ifdef _WIN32
#pragma once
#endif

#include "endgame.h"
#include "resultgenerator.h"

class CVictoryImage;
class CResultGenerator;

namespace vgui
{
	class ImagePanel;
};

#define TABNAME_GAMERESULT "Results"

// ------------------------------------------------------------------------------------ //
// Game Result
// ------------------------------------------------------------------------------------ //
class CGameResult : public vgui::EditablePanel
{
private:
	DECLARE_CLASS_SIMPLE(CGameResult, vgui::EditablePanel);

public:
	CGameResult(Panel *pParent, const char *pszPanelName);

	void Reset(void);

	void SetResult(const Result_t &Result);

protected:
	virtual void PerformLayout(void);

	virtual void OnLevelInit(void);

private:
	void UpdateWinner(void);

	void UpdateMap(void);

private:
	bool m_bNeedsUpdate;

	CVictoryImage *m_pVictory;
	vgui::ImagePanel *m_pWinner;

	CResultGenerator *m_pResult;

	bool m_bResultSet;
	Result_t m_Result;
};


#endif // VGUI_GAMERESULT_H
