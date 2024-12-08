#include "cbase.h"

#include "playercust.h"
#include "animation.h"
#include <filesystem.h>
#include <keyvalues.h>
#include <engine/ivmodelinfo.h>
#include "studio.h"
#include "ins_player_shared.h"
#include "team_shared.h"
#include "play_team_shared.h"
#include "team_lookup.h"

#include "tier0/memdbgon.h"

DEFINE_STRING_LOOKUP_CONSTANTS(int,CUSTOMIZE_FL)
	ADD_LOOKUP(CUSTOMIZE_FL_DEFAULT)
	ADD_LOOKUP(CUSTOMIZE_FL_HIDDEN)
	ADD_LOOKUP(CUSTOMIZE_FL_ITEMDEPENDENT)
	ADD_LOOKUP(CUSTOMIZE_FL_DISPLAYSKIN)
END_STRING_LOOKUP_CONSTANTS()

int strtoi(const char* str)
{
	char* tmp;
	int r=strtol(str,&tmp,10);
	if (str==tmp)
		return -1;
	return r;
}

#ifdef CLIENT_DLL
#define DLL_STR ":client"
#else
#define DLL_STR ":server"
#endif // CLIENT_DLL

// NOTE (from Pongles): I had to add in studioHdr because Valve
// have changed the parm's of a bunch of their functions

bool AttachmentExists(const char* pszAttach, studiohdr_t* pStuHdr)
{
	int iNum=pStuHdr->GetNumAttachments();
	for (int i=0;i<iNum;i++)
	{
		if (FStrEq(pStuHdr->pAttachment(i).pszName(),pszAttach))
			return true;
	}
	return false;
}

void SetupModelCustomization(const char* pszModel, modelcustomization_t& MdlCust)
{
	KeyValues *pKVMdl,*pKV[4];
	studiohdr_t* pStuHdr;
	char szTmp[512];
	const char *pszTmp;
	const char *pszModelName;
	int i,j;

	pszModelName=szTmp;
	if (!strpbrk(pszModel,"/\\"))
	{
		Q_snprintf(szTmp,512,"models/player/%s",pszModel);
	}
	else
	{
		Q_strncpy(szTmp,pszModel,512);
	}
	i=modelinfo->GetModelIndex(pszModelName);
	if (i==-1)
	{
		DevWarning("(SetupModelCustomization" DLL_STR ") No such player model: %s; skipped\n",pszModel);
		return;
	}
	MdlCust.nModelIndex=i;
	const model_t* mdl=modelinfo->GetModel(i);
	if (!mdl)
	{
		DevWarning("(SetupModelCustomization" DLL_STR ") No such player model: %s; skipped\n",pszModel);
		return;
	}
	pStuHdr=modelinfo->GetStudiomodel(mdl); 
	CStudioHdr studioHdr( pStuHdr, mdlcache );
	if (!pStuHdr)
	{
		DevWarning("(SetupModelCustomization" DLL_STR ") No such player model: %s; skipped\n",pszModel);
		return;
	}

	MdlCust.aSkins.SetCount(pStuHdr->numskinfamilies);
	MdlCust.aBodygroups.SetCount(GetNumBodyGroups(&studioHdr));
	MdlCust.iMaxSubmodels=0;
	for(i=0;i<MdlCust.aBodygroups.Count();i++)
	{
		j=GetBodygroupCount(&studioHdr,i);
		MdlCust.aBodygroups[i].aSubmodels.SetCount(j);
		if (j>MdlCust.iMaxSubmodels)
			MdlCust.iMaxSubmodels=j;
	}

	pKVMdl=new KeyValues("mdlkv");
	pKVMdl->LoadFromBuffer(NULL,pStuHdr->KeyValueText());
	for (pKV[0]=pKVMdl->GetFirstSubKey();pKV[0];pKV[0]=pKV[0]->GetNextKey())
	{
		pszTmp=pKV[0]->GetName();
		if (FStrEq(pszTmp,"ModelCustomization"))
			break;
	}
	if (!pKV[0])
	{
		DevWarning("(SetupModelCustomization" DLL_STR ") No embedded ModelCustomization data for player model: %s; skipped\n",pszModel);
		pKVMdl->deleteThis();
		return;
	}

	pszTmp=pszModelName;
	pszTmp--;
	while (*++pszTmp)
		if (*pszTmp=='/'||*pszTmp=='\\')
			*(char*)pszTmp='_';

	i=Q_strlen(szTmp)+1;
	MdlCust.pszModelFile=new char[i];
	Q_strncpy(MdlCust.pszModelFile,szTmp,i);

	bool bGotSkins=false,bGotSubmodels=false,bGotVersion=false;
	skincustomization_t* CurSkin;
	bodygroupcustomization_t* CurGroup;
	bodygroupmodelcustomization_t* CurGroupModel;
	CUtlVector<int>* aiFlags;

#define FOR_KV(var,base) \
	for (var[base+1]=var[base]->GetFirstSubKey();var[base+1];var[base+1]=var[base+1]->GetNextKey())

	FOR_KV(pKV,0)
	{
		pszTmp=pKV[1]->GetName();
		if (FStrEq(pszTmp,"Skins"))
		{
			if (bGotSkins)
			{
				DevWarning("(SetupModelCustomization" DLL_STR ") Duplicate skins entry for player model %s; ignored\n",pszModel);
				continue;
			}
			pszTmp=pKV[1]->GetString("attach");
			i=Q_strlen(pszTmp)+1;
			MdlCust.pszHeadAttachment=new char[i];
			Q_strncpy(MdlCust.pszHeadAttachment,pszTmp,i);
			pszTmp=pKV[1]->GetString("name");
			i=Q_strlen(pszTmp)+1;
			MdlCust.pszHeadName=new char[i];
			Q_strncpy(MdlCust.pszHeadName,pszTmp,i);
			if (!AttachmentExists(pszTmp,pStuHdr))
			{
				DevWarning("(SetupModelCustomization" DLL_STR ") Non existent attachment %s for skins; ignored\n",pszTmp);
			}
			bGotSkins=true;
			FOR_KV(pKV,1)
			{
				i=strtoi(pKV[2]->GetName());
				if (i==-1)
					continue;
				if (i>=MdlCust.aSkins.Count())
				{
					DevWarning("(SetupModelCustomization" DLL_STR ") Skins entry overflow for player model %s <i=%d>; ignored\n",pszModel,i);
					continue;
				}
				CurSkin=&MdlCust.aSkins[i];
				CurSkin->bLoaded=true;
				CurSkin->iSkin=i;
				CurSkin->iFlags=0;
				aiFlags=CONVERT_STRING(CUSTOMIZE_FL,pKV[2]->GetString("flags"));
				for (i=0;i<aiFlags->Count();i++)
					CurSkin->iFlags|=(*aiFlags)[i];
				pszTmp=pKV[2]->GetString("name");
				i=Q_strlen(pszTmp)+1;
				CurSkin->pszName=new char[i];
				Q_strncpy(CurSkin->pszName,pszTmp,i);
				pszTmp=pKV[2]->GetString("desc");
				i=Q_strlen(pszTmp)+1;
				CurSkin->pszDesc=new char[i];
				Q_strncpy(CurSkin->pszDesc,pszTmp,i);
				pszTmp=pKV[2]->GetString("model");
				i=Q_strlen(pszTmp)+1;
				CurSkin->pszModel=new char[i];
				Q_strncpy(CurSkin->pszModel,pszTmp,i);
			}
		}
		else if (FStrEq(pszTmp,"Submodels"))
		{
			if (bGotSubmodels)
			{
				DevWarning("(SetupModelCustomization" DLL_STR ") Duplicate submodels entry for player model %s; ignored\n",pszModel);
				continue;
			}
			bGotSubmodels=true;
			FOR_KV(pKV,1)
			{
				pszTmp=pKV[2]->GetName();
				i=FindBodygroupByName(&studioHdr,pszTmp);
				if (i==-1)
					continue;
				if (i>=MdlCust.aBodygroups.Count())
				{
					DevWarning("(SetupModelCustomization" DLL_STR ") Bodygroup entry overflow for player model %s: %s <i=%d>; ignored\n",pszModel,pszTmp,i);
					continue;
				}
				CurGroup=&MdlCust.aBodygroups[i];
				CurGroup->bLoaded=true;
				CurGroup->iGroup=i;
				CurGroup->iFlags=0;
				pszTmp=pKV[2]->GetString("name");
				i=Q_strlen(pszTmp)+1;
				CurGroup->pszName=new char[i];
				Q_strncpy(CurGroup->pszName,pszTmp,i);
				pszTmp=pKV[2]->GetString("attach");
				i=Q_strlen(pszTmp)+1;
				CurGroup->pszAttachment=new char[i];
				Q_strncpy(CurGroup->pszAttachment,pszTmp,i);
				if (!AttachmentExists(pszTmp,pStuHdr))
				{
					DevWarning("(SetupModelCustomization" DLL_STR ") Non existent attachment %s for bodygroup %s; ignored\n",pszTmp,CurGroup->pszName);
				}
				aiFlags=CONVERT_STRING(CUSTOMIZE_FL,pKV[2]->GetString("flags"));

				for (i=0;i<aiFlags->Count();i++)
					CurGroup->iFlags|=(*aiFlags)[i];

				if (CurGroup->iFlags&CUSTOMIZE_FL_ITEMDEPENDENT)
				{
					pszTmp=pKV[2]->GetString("itempos");
					i=Q_strlen(pszTmp)+1;
					CurGroup->pszItempos=new char[i];
					Q_strncpy(CurGroup->pszItempos,pszTmp,i);
					CurGroup->iItempos=0;
				}
				else
				{
					CurGroup->iItempos=0;
					CurGroup->pszItempos=NULL;
				}

				CurGroup->iItempos=0;
				CurGroup->pszItempos=NULL;

				FOR_KV(pKV,2)
				{
					i=strtoi(pKV[3]->GetName());
					if (i==-1)
						continue;
					if (i>=CurGroup->aSubmodels.Count())
					{
						DevWarning("(SetupModelCustomization" DLL_STR ") Bodygroup submodel overflow for player model %s: %s <i=%d>; ignored\n",pszModel,pKV[2]->GetName(),i);
						continue;
					}
					CurGroupModel=&CurGroup->aSubmodels[i];
					CurGroupModel->bLoaded=true;
					CurGroupModel->iGroupValue=i;
					CurGroupModel->iGroup=CurGroup->iGroup;
					CurGroupModel->iFlags=0;
					aiFlags=CONVERT_STRING(CUSTOMIZE_FL,pKV[3]->GetString("flags"));
					for (i=0;i<aiFlags->Count();i++)
						CurGroupModel->iFlags|=(*aiFlags)[i];
					pszTmp=pKV[3]->GetString("name");
					i=Q_strlen(pszTmp)+1;
					CurGroupModel->pszName=new char[i];
					Q_strncpy(CurGroupModel->pszName,pszTmp,i);
					pszTmp=pKV[3]->GetString("desc");
					i=Q_strlen(pszTmp)+1;
					CurGroupModel->pszDesc=new char[i];
					Q_strncpy(CurGroupModel->pszDesc,pszTmp,i);
					pszTmp=pKV[3]->GetString("itemcode");
					i=Q_strlen(pszTmp)+1;
					CurGroupModel->pszItemcode=new char[i];
					Q_strncpy(CurGroupModel->pszItemcode,pszTmp,i);
				}
			}
		}
		else if (FStrEq(pszTmp,"Version"))
		{
			if (bGotVersion)
			{
				DevWarning("(SetupModelCustomization" DLL_STR ") Duplicate version entry for player model %s; replaced\n",pszModel);
			}
			bGotVersion=true;
			pszTmp=pKV[1]->GetString();
			i=Q_strlen(pszTmp)+1;
			MdlCust.pszVersion=new char[i];
			Q_strncpy(MdlCust.pszVersion,pszTmp,i);
		}
		else
		{
			DevWarning("(SetupModelCustomization" DLL_STR ") Bogus entry for player model %s: %s; ignored\n",pszModel,pszTmp);
			continue;
		}
	}

	if (!bGotSkins)
	{
		DevWarning("(SetupModelCustomization" DLL_STR ") No skins entry for player model %s!!!\n",pszModel);
	}

	if (!bGotSubmodels)
	{
		DevWarning("(SetupModelCustomization" DLL_STR ") No submodels entry for player model %s!!!\n",pszModel);
	}

	if (!bGotVersion)
	{
		DevWarning("(SetupModelCustomization" DLL_STR ") No version entry for player model %s!!!\n",pszModel);
	}

	for (i=0;i<MdlCust.aBodygroups.Count();i++)
		if (!MdlCust.aBodygroups[i].bLoaded)
		{
			DevWarning("(SetupModelCustomization" DLL_STR ") Bodygroup<%d:%s>: no entry found for player model %s\n",i,GetBodygroupName(&studioHdr,i),pszModel);
		}
		else
		{
			for (j=0;j<MdlCust.aBodygroups[i].aSubmodels.Count();j++)
				if (!MdlCust.aBodygroups[i].aSubmodels[j].bLoaded)
				{
					DevWarning("(SetupModelCustomization" DLL_STR ") Bodygroup<%d:%s> submodel<%d>: no entry found for player model %s\n",i,GetBodygroupName(&studioHdr,i),j,pszModel);
				}
		}
	for (i=0;i<MdlCust.aSkins.Count();i++)
		if (!MdlCust.aSkins[i].bLoaded)
		{
			DevWarning("(SetupModelCustomization" DLL_STR ") Skin<%d>: no entry found for player model %s\n",i,pszModel);
		}

	MdlCust.bLoaded=true;
	pKVMdl->deleteThis();
}

#pragma warning( disable: 4706 )
int GetStrLength(int i)
{
	int iRet=1;
	if (i<0)
	{
		iRet++;
		i=-1;
	}
	while (i=(i>>4))
	{
		iRet++;
	}
	return iRet;
}
#pragma warning( default: 4706 )

#ifdef CLIENT_DLL
void SaveModelCustomization(modelcustomization_t& MdlCust, bool bSendToServer, int iPlayerBody, int iPlayerSkin, int iWeapon)
{
	studiohdr_t* pStuHdr=modelinfo->GetStudiomodel(modelinfo->GetModel(MdlCust.nModelIndex));
	CStudioHdr studioHdr( pStuHdr, mdlcache );

	KeyValues *pKV[3];
	pKV[0]=new KeyValues("ModelCustomizationSave");
	pKV[0]->SetString("Version",MdlCust.pszVersion);

	char *szCommandString,*szCurrent;
	int i,j,iSkins,iBodygroups;
	int iLen=1; //[NULL]
	iSkins=MdlCust.aSkins.Count();
	//		"skin"	[space]	id						[semicolon]
	iLen+=	4+		1+		GetStrLength(iSkins)+	1;
	iBodygroups=MdlCust.aBodygroups.Count();
	//						"body"	[space]	id							[space]	id										[semicolon]
	iLen+=	iBodygroups*(	4+		1+		GetStrLength(iBodygroups)+	1+		GetStrLength(MdlCust.iMaxSubmodels)+	1);
	szCommandString=new char[iLen];
	szCommandString[0]='\0';
	szCurrent=szCommandString;
	j=iPlayerSkin;
	pKV[1]=pKV[0]->FindKey("Skin",true);
	pKV[1]->SetString("name",MdlCust.aSkins[j].pszName);
	Q_snprintf(szCurrent,iLen,"skin %x;",j);
	szCurrent+=Q_strlen(szCurrent);
	pKV[1]=pKV[0]->FindKey("Submodels",true);
	for (i=0;i<iBodygroups;i++)
	{
		if (MdlCust.aBodygroups[i].iFlags&CUSTOMIZE_FL_HIDDEN)
			continue;
		j=GetBodygroup(&studioHdr,iPlayerBody,i);
		pKV[2]=pKV[1]->FindKey(MdlCust.aBodygroups[i].pszName,true);
		if (MdlCust.aBodygroups[i].iFlags&CUSTOMIZE_FL_ITEMDEPENDENT)
			pKV[2]->SetString("itempos",MdlCust.aBodygroups[i].pszItempos);
		pKV[2]->SetString("name",MdlCust.aBodygroups[i].aSubmodels[j].pszName);
		pKV[2]->SetString("itemcode",MdlCust.aBodygroups[i].aSubmodels[j].pszItemcode);
		Q_snprintf(szCurrent,iLen,"body %x %x;",i,j);
		szCurrent+=Q_strlen(szCurrent);
	}
	pKV[0]->SetString("Command",szCommandString);
	pKV[0]->SetString("Weapon",WeaponIDToName(iWeapon));
	if (bSendToServer)
		engine->ServerCmd(szCommandString);
	delete [] szCommandString;

	char szFileName[512];
	Q_snprintf(szFileName,512,"cfg/%s.txt",MdlCust.pszModelFile);
	pKV[0]->SaveToFile(filesystem,szFileName);
	pKV[0]->deleteThis();
}

void AddCommand(studiohdr_t* pStuHdr, char* pszCmd, int& iPlayerBody, int& iPlayerSkin)
{
	char* pszCur=pszCmd;
	if (*pszCur=='b')
	{
		pszCur+=5;
		int iBG=strtol(pszCur,&pszCur,16);
		pszCur++;
		CStudioHdr studioHdr( pStuHdr, mdlcache );
		SetBodygroup(&studioHdr,iPlayerBody,iBG,strtol(pszCur,NULL,16));
	}
	else if (*pszCur=='s')
	{
		pszCur+=5;
		iPlayerSkin=strtol(pszCur,NULL,16);
	}
}

void LoadModelCustomization(modelcustomization_t& MdlCust, bool bSendToServer, int& iPlayerBody, int& iPlayerSkin, int &iWeapon)
{
	studiohdr_t* pStuHdr=modelinfo->GetStudiomodel(modelinfo->GetModel(MdlCust.nModelIndex));
	CStudioHdr studioHdr( pStuHdr, mdlcache );

	KeyValues *pKV[3];
	char szTmp[512];
	pKV[0]=new KeyValues("ModelCustomizationSave");
	Q_snprintf(szTmp,512,"cfg/%s.txt",MdlCust.pszModelFile);
	if (!pKV[0]->LoadFromFile(filesystem,szTmp))
		return;
	iWeapon=WeaponNameToID(pKV[0]->GetString("Weapon"));
	if (FStrEq(MdlCust.pszVersion,pKV[0]->GetString("Version")))
	{
		const char* pszStr=pKV[0]->GetString("Command");
		if (bSendToServer)
			engine->ServerCmd(pszStr);
		const char* pszCur=NULL;
		const char* pszIt;
		for(pszIt=pszStr;*pszIt;pszIt++)
		{
			if (pszCur)
			{
				if (*pszIt==';')
				{
					Q_strncpy(szTmp,pszCur,min((int)(pszIt-pszCur)+1,512));
					AddCommand(pStuHdr,szTmp,iPlayerBody,iPlayerSkin);
					pszCur=NULL;
				}
			}
			else
			{
				pszCur=pszIt;
			}
		}
		AssertMsg(!pszCur,"Invalid customization file."); //this should _NEVER_ happen, as a genuine customization
		                                                  //  save file ends this string with a semi-colon
	}
	else
	{
		int i,j;
		iPlayerBody=iPlayerSkin=0;
		Msg("(LoadModelCustomization) Invalid customization file. You have version %s. Required version: %s. Trying to salvage customization though.\n",pKV[0]->GetString("Version"),MdlCust.pszVersion);
		DevMsg("Salvaging...");
		pKV[1]=pKV[0]->FindKey("Submodels");
		if (pKV[1])
		{
			for (i=0;i<MdlCust.aBodygroups.Count();i++)
				if (MdlCust.aBodygroups[i].bLoaded)
				{
					pKV[2]=pKV[1]->FindKey(MdlCust.aBodygroups[i].pszName);
					if (pKV[2])
					{
						const char* pszTmp=pKV[2]->GetString("itempos",NULL);
						if (pszTmp&&!FStrEq(pszTmp,MdlCust.aBodygroups[i].pszItempos))
							continue;
						int iGroup=-1;
						for (j=0;j<MdlCust.aBodygroups[i].aSubmodels.Count();j++)
							if (MdlCust.aBodygroups[i].aSubmodels[j].bLoaded)
							{
								const char* pszTmp=pKV[2]->GetString("name",NULL);
								if (pszTmp&&FStrEq(pszTmp,MdlCust.aBodygroups[i].aSubmodels[j].pszName))
								{
									const char* pszTmp=pKV[2]->GetString("itemcode",NULL);
									if (pszTmp&&FStrEq(pszTmp,MdlCust.aBodygroups[i].aSubmodels[j].pszItemcode))
									{
										iGroup=MdlCust.aBodygroups[i].aSubmodels[j].iGroupValue;
										break;
									}
									else if (!pszTmp)
									{
										iGroup=MdlCust.aBodygroups[i].aSubmodels[j].iGroupValue;
										break;
									}
								}
							}
						if (iGroup!=-1)
						{
							DevMsg("bodygroup<%d,%d>, ",MdlCust.aBodygroups[i].iGroup,iGroup);
							SetBodygroup(&studioHdr,iPlayerBody,MdlCust.aBodygroups[i].iGroup,iGroup);
						}
					}
				}
		}
		pKV[1]=pKV[0]->FindKey("Skin");
		if (pKV[1])
		{
			const char* pszTmp=pKV[1]->GetString("name",NULL);
			for (i=0;i<MdlCust.aSkins.Count();i++)
				if (MdlCust.aSkins[i].bLoaded)
				{
					if (pszTmp&&FStrEq(pszTmp,MdlCust.aSkins[i].pszName))
					{
						DevMsg("skin<%d>, ",MdlCust.aSkins[i].iSkin);
						iPlayerSkin=MdlCust.aSkins[i].iSkin;
						break;
					}
				}
		}
		DevMsg("...Done\n");
	}

	pKV[0]->deleteThis();
}
#endif

modelcustomization_t& GetModelCustomization(CINSPlayer* pPlayer)
{
	CPlayerModelData *pModelData = pPlayer->GetPlayerModelData( );
	return pModelData->GetCustomization( );
}

skincustomization_s::skincustomization_s()
{
	iFlags=0;
	iSkin=0;
	pszName=NULL;
	pszDesc=NULL;
	pszModel=NULL;
	bLoaded=false;
}

bodygroupmodelcustomization_s::bodygroupmodelcustomization_s()
{
	iFlags=0;
	iGroupValue=0;
	iGroup=0;
	pszName=NULL;
	pszDesc=NULL;
	pszItemcode=NULL;
	bLoaded=false;
}

bodygroupcustomization_s::bodygroupcustomization_s()
{
	iFlags=0;
	iGroup=0;
	pszName=NULL;
	pszItempos=NULL;
	iItempos=0;
	aSubmodels.Purge();
	bLoaded=false;
}
