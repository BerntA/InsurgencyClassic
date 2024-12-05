#define PROTECTED_THINGS_H
#include "cbase.h"
#include <windows.h>
#undef PROTECTED_THINGS_H
#include "protected_things.h"
#include "delayload.h"

#include "materialsystem/imaterialproxy.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/itexture.h"
#include "filesystem.h"
#include "svgtexture.h"
#include "../../utils/svgimage/svgimage.h"
#include <keyvalues.h>

void SVGLoadLibrary()
{
	char szBinPath[MAX_PATH];
	Q_snprintf(szBinPath,MAX_PATH,"%s/bin/",engine->GetGameDirectory());

	__DLInitDelayLoadDLL(__DLNoFail);
	__DLSetSearchPath(szBinPath);
	__DLLoadDelayLoadDLL("svgimage.dll");
	__DLFinishDelayLoadDLL;
}

void GetTextureSize(const char* pszTexture, int &iWidth, int &iHeight)
{
	iWidth=iHeight=0;
	IMaterial* pMat=materials->FindMaterial("rocket","",false);
	if (!IsErrorMaterial(pMat))
	{
		bool bFound;
		IMaterialVar* pVar=pMat->FindVar("$basetexture",&bFound,false);
		if (bFound)
		{
			ITexture *pTex=pVar->GetTextureValue();
			if (!IsErrorTexture(pTex))
			{
				iWidth=pTex->GetActualWidth();
				iHeight=pTex->GetActualHeight();
			}
		}
	}
}

class CSVGRenderRegenerator;
class CSVGRenderProxy;

class CSVGRenderRegenerator : public ITextureRegenerator
{
public:
	CSVGRenderRegenerator(unsigned char* pData) : m_pData(pData) {};
	virtual void RegenerateTextureBits( ITexture *pTexture, IVTFTexture *pVTFTexture, Rect_t *pSubRect );
	virtual void Release( void ) {};
private:
	unsigned char* m_pData;
};

void CSVGRenderRegenerator::RegenerateTextureBits( ITexture *pTexture, IVTFTexture *pVTFTexture, Rect_t *pSubRect )
{
	memcpy(pVTFTexture->ImageData(0, 0, 0),m_pData,pVTFTexture->RowSizeInBytes(0)*pVTFTexture->Height());
}

class CSVGRenderProxy: public IMaterialProxy
{
public:
	CSVGRenderProxy();
	virtual bool Init( IMaterial* pMaterial, KeyValues *pKeyValues );
	virtual void OnBind( void *pC_BaseEntity );
	virtual void Release( void );

private:
	IMaterial			*m_pMaterial;
	ITexture			*m_pTexture;
	ITextureRegenerator	*m_pTextureRegen;
	unsigned char		*m_pBuffer;
	int					m_iWidth,m_iHeight;
};

CSVGRenderProxy::CSVGRenderProxy()
{
	m_pBuffer=NULL;
	m_pTexture=NULL;
	m_pTextureRegen=NULL;
}

void CSVGRenderProxy::Release()
{
	if (m_pTexture)
	{
		m_pTexture->SetTextureRegenerator(NULL);
		m_pTexture->DecrementReferenceCount();
		m_pTexture->DeleteIfUnreferenced();
		m_pTexture=NULL;
	}
	if (m_pTextureRegen)
	{
		delete m_pTextureRegen;
		m_pTextureRegen=NULL;
	}
	if (m_pBuffer)
	{
		delete [] m_pBuffer;
		m_pBuffer=NULL;
	}
	delete this;
}

bool CSVGRenderProxy::Init(IMaterial *pMaterial, KeyValues *pKeyValues)
{
	const char *pszSVG=pKeyValues->GetString("file");
	const char *pszMVar=pKeyValues->GetString("materialvar","$basetexture");
	int iWidth=pKeyValues->GetInt("width");
	int iHeight=pKeyValues->GetInt("height");
	bool bPreserve=pKeyValues->GetInt("preservescale")?true:false;

	bool bFound;
	IMaterialVar *pTextureVar;

	m_pMaterial=pMaterial;

	pTextureVar=pMaterial->FindVar(pszMVar, &bFound, false);
	if(!bFound)
		return false;

	int iW,iH;
	engine->GetScreenSize(m_iWidth,m_iHeight);
	iWidth*=(float)m_iWidth/640.0;
	if (bPreserve)
	{
		iHeight*=(float)m_iWidth/640.0;
	}
	else
	{
		iHeight*=(float)m_iHeight/480.0;
	}
	for (iW=2;iW<iWidth;iW<<=1);
	for (iH=2;iH<iHeight;iH<<=1);

	m_pTexture=materials->CreateProceduralTexture(pszSVG,"",iW,iH,IMAGE_FORMAT_BGRA8888,TEXTUREFLAGS_EIGHTBITALPHA|TEXTUREFLAGS_NOMIP|TEXTUREFLAGS_NOLOD);
	m_pTexture->IncrementReferenceCount();
	if (IsErrorTexture(m_pTexture))
	{
		AssertMsg(false,"Couldn't create texture!");
		return false;
	}
	pTextureVar->SetTextureValue(m_pTexture);

	m_pBuffer=new unsigned char[iW*iH*4];
	memset(m_pBuffer,0,iW*iH*4);

	SVGImage svgImage;
	CUtlBuffer svgbuf;
	filesystem->ReadFile(pszSVG,NULL,svgbuf);
	svgImage.SVGOpen((unsigned char*)svgbuf.Base(),svgbuf.Size());

	svgImage.SVGSetTranslation(ImageTranslations::AutoFit::Create());
	svgImage.SVGRender(m_pBuffer,svgimgfmt_bgra,iWidth,iHeight,iW*4);

	m_pTextureRegen=new CSVGRenderRegenerator(m_pBuffer);
	m_pTexture->SetTextureRegenerator(m_pTextureRegen);
	m_pTexture->Download();

	return true;
}

void CSVGRenderProxy::OnBind(void *pC_BaseEntity )
{
	int iW,iH;
	engine->GetScreenSize(iW,iH);
	if (iW!=m_iWidth||iH!=m_iHeight)
		m_pMaterial->Refresh();
}

EXPOSE_INTERFACE( CSVGRenderProxy, IMaterialProxy, "SVGRender" IMATERIAL_PROXY_INTERFACE_VERSION );
