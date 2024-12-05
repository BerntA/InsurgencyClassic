#include "svg-cairo.h"
#include "cairo.h"
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#define  dllimport dllexport
#include "svgimage.h"

#define _USE_MATH_DEFINES

#include <math.h>

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

SVGImage::SVGImage()
{
	m_bLoaded=false;

	m_pTranslation=NULL;

	svg_cairo_create(&m_scImage);
}

SVGImage::~SVGImage()
{
	if (m_scImage)
		svg_cairo_destroy(m_scImage);
	if (m_pTranslation)
		delete m_pTranslation;
}

void SVGImage::SVGOpen(const char* pszFilename)
{
	struct stat statStruct;
	FILE* fp=fopen(pszFilename,"rb");
	if (fp)
	{
		fstat(fileno(fp),&statStruct);
		unsigned char* data=new unsigned char[statStruct.st_size];
		unsigned int size=fread(data,1,statStruct.st_size,fp);
		if (size==statStruct.st_size)
			SVGOpen(data,size);
		fclose(fp);
		delete [] data;
	}
}

void SVGImage::SVGOpen(const unsigned char* pData, unsigned int nSize)
{
	svg_cairo_parse_buffer(m_scImage,(char*)pData,nSize);
	m_bLoaded=true;
}

void SVGImage::SVGClose()
{
	if (m_scImage)
		svg_cairo_destroy(m_scImage);
	svg_cairo_create(&m_scImage);
	m_bLoaded=false;
}

bool SVGImage::SVGLoaded()
{
	return m_bLoaded;
}

void SVGImage::SVGGetSize(unsigned int &nWidth, unsigned int &nHeigth, float *flRatio)
{
	svg_cairo_get_size(m_scImage,&nWidth,&nHeigth);
	if (flRatio)
		*flRatio=(float)nWidth/(float)nHeigth;
}

void SVGImage::SVGRender(void *pBuffer, svgimage_format_e sifFormat, int iWidth, int iHeight, int iStride)
{
	if (sifFormat==svgimgfmt_none)
		return;
	cairo_surface_t* csSurface;
	cairo_t* cRender;
	switch (sifFormat)
	{
		case svgimgfmt_bgra:
			csSurface=cairo_image_surface_create_for_data((unsigned char*)pBuffer,CAIRO_FORMAT_ARGB32,iWidth,iHeight,iStride);
			break;
		case svgimgfmt_png:
			csSurface=cairo_image_surface_create(CAIRO_FORMAT_ARGB32,iWidth,iHeight);
			break;
	}
	cRender=cairo_create(csSurface);
	if (m_pTranslation)
		m_pTranslation->Translate(m_scImage,cRender,iWidth,iHeight,false,0,0);
	svg_cairo_render(m_scImage,cRender);
	if (sifFormat==svgimgfmt_png)
		cairo_surface_write_to_png(csSurface,(char*)pBuffer);
	cairo_destroy(cRender);
	cairo_surface_destroy(csSurface);
}

void SVGImage::SVGSetTranslation(ITranslation* pTranslation)
{
	if (m_pTranslation)
		delete m_pTranslation;
	m_pTranslation=pTranslation;
}

void ITranslation::TranslateOther(ITranslation* pTranslation, TranslateArgs)
{
	pTranslation->Translate(TranslateArgsCall);
}

namespace ImageTranslations
{
#define GetSVGSize(nW,nH) if (bNewSize) {nW=iNewWidth;nH=iNewHeight;} else svg_cairo_get_size(scImage,&nWidth,&nHeight)

	AutoFit::AutoFit() {}
	AutoFit* AutoFit::Create() { return new AutoFit(); }

	void AutoFit::Translate(TranslateArgs)
	{
		unsigned int nWidth, nHeight;
		GetSVGSize(nWidth,nHeight);

		cairo_scale(cCairo,(float)iWidth/(float)nWidth,(float)iHeight/(float)nHeight);
	}

	void MoveIntoPosition(cairo_t* cCairo, int iSrcWidth, int iSrcHeight, int iDstWidth, int iDstHeight, int iXPos, int iYPos, bool bPositionXCenter, bool bPositionYCenter)
	{
		switch (iXPos)
		{
			case -1:
				if (bPositionXCenter)
					cairo_translate(cCairo,-iSrcWidth>>1,0);
				break;
			case 0:
				cairo_translate(cCairo,(iDstWidth>>1)-(iSrcWidth>>1),0);
				break;
			case 1:
				if (bPositionXCenter)
					cairo_translate(cCairo,iDstWidth-(iSrcWidth>>1),0);
				else
					cairo_translate(cCairo,iDstWidth-iSrcWidth,0);
				break;
		}
		switch (iYPos)
		{
			case -1:
				if (bPositionYCenter)
					cairo_translate(cCairo,0,-iSrcHeight>>1);
				break;
			case 0:
				cairo_translate(cCairo,0,(iDstHeight>>1)-(iSrcHeight>>1));
				break;
			case 1:
				if (bPositionYCenter)
					cairo_translate(cCairo,0,iDstHeight-(iSrcHeight>>1));
				else
					cairo_translate(cCairo,0,iDstHeight-iSrcHeight);
				break;
		}
	}

	FitPreserveRatioPosition::FitPreserveRatioPosition(int iXPos, int iYPos, bool bPositionXCenter, bool bPositionYCenter) : m_iXPos(iXPos), m_iYPos(iYPos), m_bPositionXCenter(bPositionXCenter), m_bPositionYCenter(bPositionYCenter) {}
	FitPreserveRatioPosition* FitPreserveRatioPosition::Create(int iXPos, int iYPos, bool bPositionXCenter, bool bPositionYCenter) { return new FitPreserveRatioPosition(iXPos,iYPos,bPositionXCenter,bPositionYCenter); }

	void FitPreserveRatioPosition::Translate(TranslateArgs)
	{
		unsigned int nWidth, nHeight;
		GetSVGSize(nWidth,nHeight);

		float flXScale=(float)iWidth/(float)nWidth,flYScale=(float)iHeight/(float)nHeight,flScale=min(flXScale,flYScale);
		
		MoveIntoPosition(cCairo,nWidth*flScale,nHeight*flScale,iWidth,iHeight,m_iXPos,m_iYPos,m_bPositionXCenter,m_bPositionYCenter);

		cairo_scale(cCairo,flScale,flScale);
	}

	SizePosition::SizePosition(int iWidth, int iHeight, int iXPos, int iYPos, bool bPositionXCenter, bool bPositionYCenter) : m_iWidth(iWidth), m_iHeight(iHeight), m_iXPos(iXPos), m_iYPos(iYPos), m_bPositionXCenter(bPositionXCenter), m_bPositionYCenter(bPositionYCenter) {}
	SizePosition* SizePosition::Create(int iWidth, int iHeight, int iXPos, int iYPos, bool bPositionXCenter, bool bPositionYCenter) { return new SizePosition(iWidth,iHeight,iXPos,iYPos,bPositionXCenter,bPositionYCenter); }
	
	void SizePosition::Translate(TranslateArgs)
	{
		unsigned int nWidth, nHeight;
		GetSVGSize(nWidth,nHeight);

		MoveIntoPosition(cCairo,m_iWidth,m_iHeight,iWidth,iHeight,m_iXPos,m_iYPos,m_bPositionXCenter,m_bPositionYCenter);
		
		cairo_scale(cCairo,(float)m_iWidth/(float)nWidth,(float)m_iHeight/(float)nHeight);
	}

	RotateSource::RotateSource(ITranslation *pTranslation, float flAngle, int iXCenter, int iYCenter, bool bAdjustBounds) : m_pTranslation(pTranslation), m_flAngle(flAngle), m_iXCenter(iXCenter), m_iYCenter(iYCenter), m_bAdjustBounds(bAdjustBounds)
	{
		if (m_bAdjustBounds)
			m_iXCenter=m_iYCenter=0;
	}
	RotateSource* RotateSource::Create(ITranslation *pTranslation, float flAngle, int iXCenter, int iYCenter, bool bAdjustBounds) { return new RotateSource(pTranslation,flAngle,iXCenter,iYCenter,bAdjustBounds); }

	RotateSource::~RotateSource()
	{
		if (m_pTranslation)
			delete m_pTranslation;
	}
	
	void RotateSource::Translate(TranslateArgs)
	{
		unsigned int nWidth, nHeight;
		GetSVGSize(nWidth,nHeight);
		double flXR,flYR,flX1,flY1,flX2,flY2;
		flXR=nWidth>>1;
		flYR=nHeight>>1;
		double flSinA=sin(m_flAngle);
		double flCosA=cos(m_flAngle);
		flX2=flX1=flXR*flCosA;
		flY2=flY1=flXR*flSinA;
		flSinA*=flYR;
		flCosA*=flYR;
		flX1-=flSinA;
		flY1+=flCosA;
		flX2+=flSinA;
		flY2-=flCosA;

		flX1=abs(flX1);
		flY1=abs(flY1);
		flX2=abs(flX2);
		flY2=abs(flY2);

		flX1=max(flX1,flX2);
		flY1=max(flY1,flY2);

		bNewSize=true;
		iNewWidth=flX1*2;
		iNewHeight=flY1*2;
		if (m_pTranslation)
			TranslateOther(m_pTranslation,TranslateArgsCall);
		
		if (m_bAdjustBounds)
		{
			cairo_translate(cCairo,flX1,flY1);
		}
		else
		{
			switch (m_iXCenter)
			{
				case 0:
					cairo_translate(cCairo,(nWidth>>1),0);
					break;
				case 1:
					cairo_translate(cCairo,iWidth-nWidth,0);
					break;
			}
			switch (m_iYCenter)
			{
				case 0:
					cairo_translate(cCairo,0,(nHeight>>1));
					break;
				case 1:
					cairo_translate(cCairo,0,iHeight-nHeight);
					break;
			}
		}
		cairo_rotate(cCairo,m_flAngle);
		switch (m_iXCenter)
		{
			case 0:
				cairo_translate(cCairo,-((int)nWidth>>1),0);
				break;
			case 1:
				cairo_translate(cCairo,-(iWidth-(int)nWidth),0);
				break;
		}
		switch (m_iYCenter)
		{
			case 0:
				cairo_translate(cCairo,0,-((int)nHeight>>1));
				break;
			case 1:
				cairo_translate(cCairo,0,-(iHeight-(int)nHeight));
				break;
		}
	}

	Scale::Scale(ITranslation *pTranslation, float flXScale, float flYScale) : m_pTranslation(pTranslation), m_flXScale(flXScale), m_flYScale(flYScale) {}
	Scale* Scale::Create(ITranslation *pTranslation, float flXScale, float flYScale) { return new Scale(pTranslation,flXScale,flYScale); }
	
	Scale::~Scale()
	{
		if (m_pTranslation)
			delete m_pTranslation;
	}

	void Scale::Translate(TranslateArgs)
	{
		if (m_pTranslation)
			TranslateOther(m_pTranslation,TranslateArgsCall);

		unsigned int nWidth, nHeight;
		GetSVGSize(nWidth,nHeight);

		cairo_translate(cCairo,(nWidth>>1),(nHeight>>1));
		cairo_scale(cCairo,m_flXScale,m_flYScale);
		cairo_translate(cCairo,-((int)nWidth>>1),-((int)nHeight>>1));
	}

	Custom::Custom(Custom *pTranslation, int iType, float flXParam, float flYParam) : m_pTranslation(pTranslation), m_iType(iType), m_flXParam(flXParam), m_flYParam(flYParam) {}
	Custom* Custom::Create(Custom *pTranslation, int iType, float flXParam, float flYParam) { return new Custom(pTranslation,iType,flXParam,flYParam); }
	
	Custom::~Custom()
	{
		if (m_pTranslation)
			delete m_pTranslation;
	}

	void Custom::Translate(TranslateArgs)
	{
		switch (m_iType)
		{
			case 0:
				cairo_translate(cCairo,m_flXParam,m_flYParam);
				break;
			case 1:
				cairo_rotate(cCairo,m_flXParam);
				break;
			case 2:
				cairo_scale(cCairo,m_flXParam,m_flYParam);
				break;
			default:
				;
		}

		if (m_pTranslation)
			TranslateOther(m_pTranslation,TranslateArgsCall);
	}
};