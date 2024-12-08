#include "cbase.h"
#include "insvgui.h"
#include "team_lookup.h"
#include "insvgui_customizehelper.h"
#include "animation.h"

class CCustomizeHelper : public ICustomizeHelper
{
private:
	char* m_pszTmp;
	int m_iTeam;
	int m_iClass;
	CPlayerClass* m_pClass;
	CPlayerModelData* m_pModelData;
	modelcustomization_t* m_pCustomization;
	studiohdr_t *m_pStuHdr;

	int m_iWeapon;
	int m_nSkin;
	int m_nBody;

	char* AllocString(int iSize);

	int GetSelectedWeapon();
	int GetSelectedSkin();
	int GetSelectedBodygroup(int iGroup);

public:
	CCustomizeHelper();
	~CCustomizeHelper();

	const char* GetModelName(int iTeam, int iClass);
	void LoadModel(int iTeam, int iClass);

	void SaveModel();
	void CloseModel();

	int GetModelIndex();

	int GetCustomizeCount();
	const char* GetCustomizeAttachment(int iCust);
	const char* GetCustomizeImage(int iCust);
	const char* GetCustomizeName(int iCust);

	int GetCustomizeOptionCount(int iCust);
	const char* GetCustomizeOptionName(int iCust, int iOption);
	const char* GetCustomizeOptionDesc(int iCust, int iOption);

	int GetSelectedCustomizeOption(int iCust);
	void SetSelectedCustomizeOption(int iCust, int iOption);

	int GetSelectedRealWeaponID();
	int GetSelectedRealSkin();
	int GetSelectedRealBody();
};

ICustomizeHelper* CINSVGUIHelper::GetCustomizeHelper()
{
	static CCustomizeHelper custHelper;
	return &custHelper;
}

CCustomizeHelper::CCustomizeHelper()
{
	m_pszTmp=NULL;
	m_pCustomization=NULL;
}

CCustomizeHelper::~CCustomizeHelper()
{
	if (m_pszTmp)
		delete [] m_pszTmp;
}

char* CCustomizeHelper::AllocString(int iSize)
{
	if (m_pszTmp)
		delete [] m_pszTmp;
	m_pszTmp=new char[iSize];
	*m_pszTmp=0;
	return m_pszTmp;
}

const char* CCustomizeHelper::GetModelName(int iTeam, int iClass)
{
	const char* pszTmp1=g_pTeamLookup[iTeam]->GetFullName();
	const char* pszTmp2=g_pTeamLookup[iTeam]->GetClass(iClass)->GetName();
	int size=Q_strlen(pszTmp1)+Q_strlen(pszTmp2)+4;
	AllocString(size);
	Q_strcat(m_pszTmp,pszTmp1,size);
	Q_strcat(m_pszTmp," - ",size);
	Q_strcat(m_pszTmp,pszTmp2,size);
	return m_pszTmp;
}

void CCustomizeHelper::LoadModel(int iTeam, int iClass)
{
	m_iTeam=iTeam;
	m_iClass=iClass;
	m_pClass=g_pTeamLookup[iTeam]->GetClass(iClass);
	m_pModelData=NULL;
	if (m_pClass)
		m_pModelData=m_pClass->GetModelData();
	if (!m_pModelData)
		m_pModelData=g_pTeamLookup[iTeam]->GetModelData();
	if (m_pModelData)
	{
		m_pCustomization=&m_pModelData->GetCustomization();
		if (!m_pCustomization->bLoaded)
			m_pCustomization=NULL;
	}
	else
	{		
		m_pCustomization=NULL;
	}
	if (m_pCustomization)
	{
		m_pStuHdr=modelinfo->GetStudiomodel(modelinfo->GetModel(m_pCustomization->nModelIndex));
		LoadModelCustomization(*m_pCustomization,false,m_nBody,m_nSkin,m_iWeapon);
	}
}

void CCustomizeHelper::SaveModel()
{
	if (!m_pCustomization)
		return;

	SaveModelCustomization(*m_pCustomization,true,m_nBody,m_nSkin,m_iWeapon);
}

void CCustomizeHelper::CloseModel()
{
	m_pCustomization=NULL;
}

int CCustomizeHelper::GetModelIndex()
{
	if (!m_pCustomization)
		return -1;

	return m_pCustomization->nModelIndex;
}

int CCustomizeHelper::GetCustomizeCount()
{
	if (!m_pCustomization)
		return -1;

	return m_pCustomization->aBodygroups.Count();
}

const char* CCustomizeHelper::GetCustomizeAttachment(int iCust)
{
	if (!m_pCustomization)
		return NULL;

	switch (iCust)
	{
		case -2:
			return NULL;
		case -1:
			return m_pCustomization->pszHeadAttachment;
		default:
			if (m_pCustomization->aBodygroups[iCust].iFlags&CUSTOMIZE_FL_HIDDEN)
				return NULL;
			else
				return m_pCustomization->aBodygroups[iCust].pszAttachment;
	}
}

const char* CCustomizeHelper::GetCustomizeImage(int iCust)
{
	if (!m_pCustomization)
		return NULL;

	switch (iCust)
	{
		case -2:
			return WeaponIDToName(m_iWeapon); //we need a material here
		default:
			return NULL;
	}
}

const char* CCustomizeHelper::GetCustomizeName(int iCust)
{
	if (!m_pCustomization)
		return "";

	switch (iCust)
	{
		case -2:
			return "Primary weapon";
		case -1:
			return m_pCustomization->pszHeadName;
		default:
			return m_pCustomization->aBodygroups[iCust].pszName;
	}
}

int CCustomizeHelper::GetCustomizeOptionCount(int iCust)
{
	if (!m_pCustomization)
		return 0;

	switch (iCust)
	{
		case -2:
			return m_pClass->GetBlueprint().GetWeaponCount(WEAPONTYPE_PRIMARY);
		case -1:
			return m_pCustomization->aSkins.Count();
		default:
			return m_pCustomization->aBodygroups[iCust].aSubmodels.Count();
	}
}

const char* CCustomizeHelper::GetCustomizeOptionName(int iCust, int iOption)
{
	if (!m_pCustomization)
		return "";

	switch (iCust)
	{
		case -2:
			return WeaponIDToAlias(m_iWeapon);
		case -1:
			return m_pCustomization->aSkins[iOption].pszName;
		default:
			return m_pCustomization->aBodygroups[iCust].aSubmodels[iOption].pszName;
	}
}

const char* CCustomizeHelper::GetCustomizeOptionDesc(int iCust, int iOption)
{
	if (!m_pCustomization)
		return "";

	switch (iCust)
	{
		case -2:
			return "";
		case -1:
			return m_pCustomization->aSkins[iOption].pszDesc;
		default:
			return m_pCustomization->aBodygroups[iCust].aSubmodels[iOption].pszDesc;
	}
}

int CCustomizeHelper::GetSelectedWeapon()
{
	for (int i=0;i<m_pClass->GetBlueprint().GetWeaponCount(WEAPONTYPE_PRIMARY);i++)
	{
		if (m_pClass->GetBlueprint().GetWeapon(WEAPONTYPE_PRIMARY,i).m_iID==m_iWeapon)
			return i;
	}
	return 0;
}

int CCustomizeHelper::GetSelectedSkin()
{
	CUtlVector<skincustomization_t> &aVector=m_pCustomization->aSkins;
	for (int i=0;i<aVector.Count();i++)
	{
		if (aVector[i].iSkin==m_nSkin)
			return i;
	}
	return 0;
}

#pragma warning(disable: 4238)

int CCustomizeHelper::GetSelectedBodygroup(int iGroup)
{
	CUtlVector<bodygroupmodelcustomization_t> &aVector=m_pCustomization->aBodygroups[iGroup].aSubmodels;
	
	int iValue=GetBodygroup(&CStudioHdr( m_pStuHdr, mdlcache ),m_nBody,iGroup);
	for (int i=0;i<aVector.Count();i++)
	{
		if (aVector[i].iGroupValue==iValue)
			return i;
	}
	return 0;
}

int CCustomizeHelper::GetSelectedCustomizeOption(int iCust)
{
	if (!m_pCustomization)
		return 0;

	switch (iCust)
	{
		case -2:
			return GetSelectedWeapon();
		case -1:
			return GetSelectedSkin();
		default:
			return GetSelectedBodygroup(iCust);
	}
}

void CCustomizeHelper::SetSelectedCustomizeOption(int iCust, int iOption)
{
	if (!m_pCustomization)
		return;

	switch (iCust)
	{
		case -2:
			m_iWeapon=m_pClass->GetBlueprint().GetWeapon(WEAPONTYPE_PRIMARY,iOption).m_iID;
			break;
		case -1:
			m_nSkin=m_pCustomization->aSkins[iOption].iSkin;
			break;
		default:
			SetBodygroup(&CStudioHdr( m_pStuHdr, mdlcache ),m_nBody,m_pCustomization->aBodygroups[iCust].aSubmodels[iOption].iGroup,m_pCustomization->aBodygroups[iCust].aSubmodels[iOption].iGroupValue);
	}
}

int CCustomizeHelper::GetSelectedRealWeaponID()
{
	if (!m_pCustomization)
		return -1;

	return m_iWeapon;
}

int CCustomizeHelper::GetSelectedRealSkin()
{
	if (!m_pCustomization)
		return -1;

	return m_nSkin;
}

int CCustomizeHelper::GetSelectedRealBody()
{
	if (!m_pCustomization)
		return -1;

	return m_nBody;
}