#pragma once

namespace DuiLib_Lite
{
	typedef struct DUILIB_API tagTFontInfo
	{
		HFONT hFont = nullptr;
		tstring sFontName;
		int iSize = 0;
		bool bBold = false;
		bool bUnderline = false;
		bool bItalic = false;
		TEXTMETRIC tm;
	} TFontInfo;

	typedef struct DUILIB_API tagTImageInfo
	{
		HBITMAP hBitmap;
		LPBYTE pBits;
		LPBYTE pSrcBits;
		int nX;
		int nY;
		bool bAlpha;
		tstring sResType;
		DWORD dwMask;
	} TImageInfo;

	typedef struct DUILIB_API tagTDrawInfo
	{
		tagTDrawInfo();
		tagTDrawInfo(LPCTSTR lpsz);
		void Clear();
		tstring sDrawString;
		tstring sImageName;
		bool bLoaded;
		const TImageInfo* pImageInfo;
		RECT rcDestOffset;
		RECT rcBmpPart;
		RECT rcScale9;
		BYTE uFade;
		bool bHole;
		bool bTiledX;
		bool bTiledY;
	} TDrawInfo;

	typedef struct DUILIB_API tagTPercentInfo
	{
		double left = 0.0;
		double top = 0.0;
		double right = 0.0;
		double bottom = 0.0;
	} TPercentInfo;

	class DUILIB_API CResourceManagerUI
	{
	public:
		CResourceManagerUI();
		~CResourceManagerUI();

	public:
		HINSTANCE GetInstance();
		tstring GetInstancePath();
		tstring GetCurrentPath();
		HINSTANCE GetResourceDll();
		const tstring& GetResourcePath();
		const tstring& GetResourceZip();
		bool IsCachedResourceZip();
		HANDLE GetResourceZipHandle();
		void SetInstance(HINSTANCE hInst);
		void SetCurrentPath(LPCTSTR pStrPath);
		void SetResourceDll(HINSTANCE hInst);
		void SetResourcePath(LPCTSTR pStrPath);
		void SetResourceZip(LPCTSTR pstrZip, bool bCachedResourceZip = false);

		//shared attribute
		//color
		DWORD GetDefaultDisabledColor() const;
		void SetDefaultDisabledColor(DWORD dwColor);
		DWORD GetDefaultFontColor() const;
		void SetDefaultFontColor(DWORD dwColor);
		DWORD GetDefaultLinkFontColor() const;
		void SetDefaultLinkFontColor(DWORD dwColor);
		DWORD GetDefaultLinkHoverFontColor() const;
		void SetDefaultLinkHoverFontColor(DWORD dwColor);
		DWORD GetDefaultSelectedBkColor() const;
		void SetDefaultSelectedBkColor(DWORD dwColor);
		TFontInfo* GetDefaultFontInfo();
		//font
		void SetDefaultFont(LPCTSTR pStrFontName, int nSize, bool bBold, bool bUnderline, bool bItalic);
		HFONT AddFont(tstring strId, LPCTSTR pStrFontName, int nSize, bool bBold, bool bUnderline, bool bItalic);
		HFONT GetFont(tstring strId);
		TFontInfo* GetFontInfo(tstring strId);
		void RemoveAllFonts();
		//image
		const TImageInfo* GetImage(LPCTSTR bitmap);
		const TImageInfo* AddImage(LPCTSTR bitmap, HBITMAP hBitmap, int iWidth, int iHeight, bool bAlpha);
		void RemoveImage(LPCTSTR bitmap);
		void RemoveAllImages();
		//language
		void AddMultiLanguageString(int id, LPCTSTR pStrMultiLanguage);
		tstring GetMultiLanguageString(int id);
		bool RemoveMultiLanguageString(int id);
		void RemoveAllMultiLanguageString();

		void AddDefaultResourceAttribute(const tstring& strName, const tstring& strValue);
		
	private:
		HINSTANCE	m_hResourceInstance = nullptr;
		tstring		m_strResourcePath;
		tstring		m_strResourceZip;
		bool		m_bCachedResourceZip = false;
		HANDLE		m_hResourceZip = nullptr;
		HINSTANCE	m_hInstance = nullptr;

		//res
		DWORD m_dwDefaultDisabledColor = 0;
		DWORD m_dwDefaultFontColor = 0;
		DWORD m_dwDefaultLinkFontColor = 0;
		DWORD m_dwDefaultLinkHoverFontColor = 0;
		DWORD m_dwDefaultSelectedBkColor = 0;
		TFontInfo m_DefaultFontInfo;
		std::map<tstring, TFontInfo*>	m_mCustomFonts;
		std::map<tstring, TImageInfo*>	m_mImageHash;
		std::map<int, tstring>			m_mMultiLanguageHash;
	};
}