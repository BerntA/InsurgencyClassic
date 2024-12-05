#ifndef SVGIMAGE_H
#define SVGIMAGE_H

typedef struct svg_cairo svg_cairo_t;
typedef struct _cairo cairo_t;
class ITranslation;
class SVGImage;

#ifndef NULL
#define NULL 0
#endif

//We might add more later
enum svgimage_format_e {
	svgimgfmt_none=0,
	svgimgfmt_bgra,
	svgimgfmt_png, //pBuffer=filename
	svgimgfmt_last,
};

#define TranslateArgs svg_cairo_t* scImage, cairo_t* cCairo, int iWidth, int iHeight, bool bNewSize, int iNewWidth, int iNewHeight
#define TranslateArgsCall scImage, cCairo, iWidth, iHeight, bNewSize, iNewWidth, iNewHeight
class __declspec(dllimport) ITranslation
{
	friend class SVGImage;
	friend class ITranslation;
protected:
	virtual void Translate(TranslateArgs) = 0;
	void TranslateOther(ITranslation* pTranslation, TranslateArgs);
};

class __declspec(dllimport) SVGImage
{
public:

protected:
	svg_cairo_t* m_scImage;

	bool m_bLoaded;
	ITranslation* m_pTranslation;

public:
	SVGImage();
	~SVGImage();

	void SVGOpen(const char* pszFilename);
	void SVGOpen(const unsigned char* pData, unsigned int nSize);
	void SVGClose();
	bool SVGLoaded();

	void SVGGetSize(unsigned int &nWidth, unsigned int &nHeight, float *flRatio=NULL);

	void SVGRender(void *pBuffer, svgimage_format_e sifFormat, int iWidth, int iHeight, int iStride);

	void SVGSetTranslation(ITranslation* pTranslation);
};

namespace ImageTranslations
{
	/*
	Usage:
	pImage->SVGSetTranslation(new ImageTranslations::AutoFit());

	You don't have to free it.

	All measurements are in pixels for lenghts and radians for angles.
	*/
	
	//==AutoFit==
	// Fit source image to destination
	class __declspec(dllimport) AutoFit : public ITranslation
	{
	private:
		AutoFit();
	public:
		static AutoFit* Create();
	protected:
		virtual void Translate(TranslateArgs);
	};

	//==FitPreserveRatioPosition==
	// Fit source image to destination while preserving the source ratio, then position
	// Pos parameters:
	//   -1 = Left/Top
	//    0 = Center
	//    1 = Right/Bottom
	// Position center means the center of the Image gets positioned,
	// Default means the bounds of the image get positioned
	class __declspec(dllimport) FitPreserveRatioPosition : public ITranslation
	{
	private:
		FitPreserveRatioPosition(int iXPos, int iYPos, bool bPositionXCenter, bool bPositionYCenter);
	public:
		static FitPreserveRatioPosition* Create(int iXPos, int iYPos, bool bPositionXCenter, bool bPositionYCenter);
	protected:
		virtual void Translate(TranslateArgs);
	private:
		int m_iXPos;
		int m_iYPos;
		bool m_bPositionXCenter;
		bool m_bPositionYCenter;
	};

	//==SizePosition==
	// Scale to specified size, then position
	// Pos parameters:
	//   -1 = Left/Top
	//    0 = Center
	//    1 = Right/Bottom
	// Position center means the center of the Image gets positioned,
	// Default means the bounds of the image get positioned
	class __declspec(dllimport) SizePosition : public ITranslation
	{
	private:
		SizePosition(int iWidth, int iHeight, int iXPos, int iYPos, bool bPositionXCenter, bool bPositionYCenter);
	public:
		static SizePosition* Create(int iWidth, int iHeight, int iXPos, int iYPos, bool bPositionXCenter, bool bPositionYCenter);
	protected:
		virtual void Translate(TranslateArgs);
	private:
		int m_iWidth;
		int m_iHeight;
		int m_iXPos;
		int m_iYPos;
		bool m_bPositionXCenter;
		bool m_bPositionYCenter;
	};

	//==RotateSource==
	// The source gets rotated before being positioned
	// Center of rotation parameters:
	//   -1 = Left/Top
	//    0 = Center
	//    1 = Right/Bottom
	// bAdjustBounds=true adjusts the size of the source so the whole rotated image fits in
	// bAdjustBounds=true renders the center parameters useless
	class __declspec(dllimport) RotateSource : public ITranslation
	{
	private:
		RotateSource(ITranslation *pTranslation, float flAngle, int iXCenter, int iYCenter, bool bAdjustBounds);
	public:
		static RotateSource* Create(ITranslation *pTranslation, float flAngle, int iXCenter, int iYCenter, bool bAdjustBounds);
		~RotateSource();
	protected:
		virtual void Translate(TranslateArgs);
	private:
		ITranslation *m_pTranslation;
		float m_flAngle;
		int m_iXCenter;
		int m_iYCenter;
		bool m_bAdjustBounds;
	};

	//==Scale==
	// The source gets scaled before being positioned
	class __declspec(dllimport) Scale : public ITranslation
	{
	private:
		Scale(ITranslation *pTranslation, float flXScale, float flYScale);
	public:
		static Scale* Create(ITranslation *pTranslation, float flXScale, float flYScale);
		~Scale();
	protected:
		virtual void Translate(TranslateArgs);
	private:
		ITranslation *m_pTranslation;
		float m_flXScale;
		float m_flYScale;
	};

	//==Custom==
	// I wouldn't recommend using this
	// Type parameter:
	//  0: Translate
	//  1: Rotate (uses XParam only)
	//  2: Scale
	class __declspec(dllimport) Custom : public ITranslation
	{
	private:
		Custom(Custom *pTranslation, int iType, float flXParam, float flYParam);
	public:
		static Custom* Create(Custom *pTranslation, int iType, float flXParam, float flYParam);
		~Custom();
	protected:
		virtual void Translate(TranslateArgs);
	private:
		Custom *m_pTranslation;
		int m_iType;
		float m_flXParam;
		float m_flYParam;
	};
};

#endif //SVGIMAGE_H