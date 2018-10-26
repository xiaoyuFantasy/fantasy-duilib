#include "StdAfx.h"
#include "UIResource.h"

namespace DuiLib_Lite
{
	DECLARE_HANDLE(HZIP);	// An HZIP identifies a zip file that has been opened
	typedef DWORD ZRESULT;
#define OpenZip OpenZipU
#define CloseZip(hz) CloseZipU(hz)
	extern HZIP OpenZipU(void *z, unsigned int len, DWORD flags);
	extern ZRESULT CloseZipU(HZIP hz);

	tagTDrawInfo::tagTDrawInfo()
	{
		Clear();
	}

	tagTDrawInfo::tagTDrawInfo(LPCTSTR lpsz)
	{
		Clear();
		sDrawString = lpsz;
	}

	void tagTDrawInfo::Clear()
	{
		sDrawString.clear();
		sImageName.clear();
		::ZeroMemory(&bLoaded, sizeof(tagTDrawInfo) - offsetof(tagTDrawInfo, bLoaded));
		uFade = 255;
	}

	CResourceManagerUI::CResourceManagerUI()
	{
		m_dwDefaultDisabledColor = 0xFFA7A6AA;
		m_dwDefaultFontColor = 0xFF000000;
		m_dwDefaultLinkFontColor = 0xFF0000FF;
		m_dwDefaultLinkHoverFontColor = 0xFFD3215F;
		m_dwDefaultSelectedBkColor = 0xFFBAE4FF;

		LOGFONT lf = { 0 };
		::GetObject(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf);
		lf.lfCharSet = DEFAULT_CHARSET;
		HFONT hDefaultFont = ::CreateFontIndirect(&lf);
		m_DefaultFontInfo.hFont = hDefaultFont;
		m_DefaultFontInfo.sFontName = lf.lfFaceName;
		m_DefaultFontInfo.iSize = -lf.lfHeight;
		m_DefaultFontInfo.bBold = (lf.lfWeight >= FW_BOLD);
		m_DefaultFontInfo.bUnderline = (lf.lfUnderline == TRUE);
		m_DefaultFontInfo.bItalic = (lf.lfItalic == TRUE);
		::ZeroMemory(&m_DefaultFontInfo.tm, sizeof(m_DefaultFontInfo.tm));
	}

	CResourceManagerUI::~CResourceManagerUI()
	{
		::DeleteObject(m_DefaultFontInfo.hFont);
		RemoveAllFonts();
		RemoveAllImages();
	}

	HINSTANCE CResourceManagerUI::GetInstance()
	{
		return m_hInstance;
	}

	tstring CResourceManagerUI::GetInstancePath()
	{
		if (m_hInstance == NULL) 
			return _T("");

		TCHAR tszModule[MAX_PATH + 1] = { 0 };
		::GetModuleFileName(m_hInstance, tszModule, MAX_PATH);
		PathRemoveFileSpec(tszModule);
		PathAddBackslash(tszModule);
		return tszModule;
	}

	tstring CResourceManagerUI::GetCurrentPath()
	{
		TCHAR tszModule[MAX_PATH + 1] = { 0 };
		::GetCurrentDirectory(MAX_PATH, tszModule);
		return tszModule;
	}

	HINSTANCE CResourceManagerUI::GetResourceDll()
	{
		if (m_hResourceInstance == NULL) 
			return m_hInstance;
		return m_hResourceInstance;
	}

	const tstring & CResourceManagerUI::GetResourcePath()
	{
		return m_strResourcePath;
	}

	const tstring & CResourceManagerUI::GetResourceZip()
	{
		return m_strResourceZip;
	}

	bool CResourceManagerUI::IsCachedResourceZip()
	{
		return m_bCachedResourceZip;
	}

	HANDLE CResourceManagerUI::GetResourceZipHandle()
	{
		return m_hResourceZip;
	}

	void CResourceManagerUI::SetInstance(HINSTANCE hInst)
	{
		m_hInstance = hInst;
	}

	void CResourceManagerUI::SetCurrentPath(LPCTSTR pStrPath)
	{
		::SetCurrentDirectory(pStrPath);
	}

	void CResourceManagerUI::SetResourceDll(HINSTANCE hInst)
	{
		m_hResourceInstance = hInst;
	}

	void CResourceManagerUI::SetResourcePath(LPCTSTR pStrPath)
	{
		if (!pStrPath)
			return;
		TCHAR szResourcePath[MAX_PATH + 1] = { 0 };
		_tcscpy_s(szResourcePath, MAX_PATH, pStrPath);
		PathAddBackslash(szResourcePath);
		m_strResourcePath = szResourcePath;
	}

	void CResourceManagerUI::SetResourceZip(LPCTSTR pstrZip, bool bCachedResourceZip)
	{
		if (m_strResourceZip.compare(pstrZip) == 0  && m_bCachedResourceZip == bCachedResourceZip) 
			return;
		if (m_bCachedResourceZip && m_hResourceZip != NULL) 
		{
			CloseZip((HZIP)m_hResourceZip);
			m_hResourceZip = NULL;
		}
		m_strResourceZip = pstrZip;
		m_bCachedResourceZip = bCachedResourceZip;
		if (m_bCachedResourceZip) 
		{
			tstring sFile = GetResourcePath();
			sFile += GetResourceZip();
			m_hResourceZip = (HANDLE)OpenZip((void*)sFile.c_str(), 0, 2);
		}
	}

	DWORD CResourceManagerUI::GetDefaultDisabledColor() const
	{
		return m_dwDefaultDisabledColor;
	}

	void CResourceManagerUI::SetDefaultDisabledColor(DWORD dwColor)
	{
		m_dwDefaultDisabledColor = dwColor;
	}

	DWORD CResourceManagerUI::GetDefaultFontColor() const
	{
		return m_dwDefaultFontColor;
	}

	void CResourceManagerUI::SetDefaultFontColor(DWORD dwColor)
	{
		m_dwDefaultFontColor = dwColor;
	}

	DWORD CResourceManagerUI::GetDefaultLinkFontColor() const
	{
		return m_dwDefaultLinkFontColor;
	}

	void CResourceManagerUI::SetDefaultLinkFontColor(DWORD dwColor)
	{
		m_dwDefaultLinkFontColor = dwColor;
	}

	DWORD CResourceManagerUI::GetDefaultLinkHoverFontColor() const
	{
		return m_dwDefaultLinkHoverFontColor;
	}

	void CResourceManagerUI::SetDefaultLinkHoverFontColor(DWORD dwColor)
	{
		m_dwDefaultLinkHoverFontColor = dwColor;
	}

	DWORD CResourceManagerUI::GetDefaultSelectedBkColor() const
	{
		return m_dwDefaultSelectedBkColor;
	}

	void CResourceManagerUI::SetDefaultSelectedBkColor(DWORD dwColor)
	{
		m_dwDefaultSelectedBkColor = dwColor;
	}

	TFontInfo * CResourceManagerUI::GetDefaultFontInfo()
	{
		if (m_DefaultFontInfo.hFont)
			return &m_DefaultFontInfo;
		
		return nullptr;
	}

	void CResourceManagerUI::SetDefaultFont(LPCTSTR pStrFontName, int nSize, bool bBold, bool bUnderline, bool bItalic)
	{
		LOGFONT lf = { 0 };
		::GetObject(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf);
		_tcsncpy(lf.lfFaceName, pStrFontName, LF_FACESIZE);
		lf.lfCharSet = DEFAULT_CHARSET;
		lf.lfHeight = -nSize;
		if (bBold) lf.lfWeight += FW_BOLD;
		if (bUnderline) 
			lf.lfUnderline = TRUE;
		if (bItalic) 
			lf.lfItalic = TRUE;
		HFONT hFont = ::CreateFontIndirect(&lf);
		if (hFont == NULL) 
			return;

		::DeleteObject(m_DefaultFontInfo.hFont);
		m_DefaultFontInfo.hFont = hFont;
		m_DefaultFontInfo.sFontName = pStrFontName;
		m_DefaultFontInfo.iSize = nSize;
		m_DefaultFontInfo.bBold = bBold;
		m_DefaultFontInfo.bUnderline = bUnderline;
		m_DefaultFontInfo.bItalic = bItalic;
		::ZeroMemory(&m_DefaultFontInfo.tm, sizeof(m_DefaultFontInfo.tm));
	}

	HFONT CResourceManagerUI::AddFont(tstring strId, LPCTSTR pStrFontName, int nSize, bool bBold, bool bUnderline, bool bItalic)
	{
		LOGFONT lf = { 0 };
		::GetObject(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf);
		_tcsncpy(lf.lfFaceName, pStrFontName, LF_FACESIZE);
		lf.lfCharSet = DEFAULT_CHARSET;
		lf.lfHeight = -nSize;
		if (bBold) 
			lf.lfWeight += FW_BOLD;
		if (bUnderline) 
			lf.lfUnderline = TRUE;
		if (bItalic) 
			lf.lfItalic = TRUE;
		HFONT hFont = ::CreateFontIndirect(&lf);
		if (hFont == NULL) 
			return NULL;

		TFontInfo* pFontInfo = new TFontInfo;
		if (!pFontInfo) return false;
		::ZeroMemory(pFontInfo, sizeof(TFontInfo));
		pFontInfo->hFont = hFont;
		pFontInfo->sFontName = pStrFontName;
		pFontInfo->iSize = nSize;
		pFontInfo->bBold = bBold;
		pFontInfo->bUnderline = bUnderline;
		pFontInfo->bItalic = bItalic;
		auto itor = m_mCustomFonts.find(strId);
		if (itor != m_mCustomFonts.end())
		{
			TFontInfo* pOldFontInfo = itor->second;
			::DeleteObject(pOldFontInfo->hFont);
			delete pOldFontInfo;
			m_mCustomFonts.erase(itor);
		}

		m_mCustomFonts[strId] = pFontInfo;
		return hFont;
	}

	HFONT CResourceManagerUI::GetFont(tstring strId)
	{
		auto itor = m_mCustomFonts.find(strId);
		if (itor != m_mCustomFonts.end())
			return itor->second->hFont;
		
		return GetDefaultFontInfo()->hFont;
	}

	TFontInfo * CResourceManagerUI::GetFontInfo(tstring strId)
	{
		auto itor = m_mCustomFonts.find(strId);
		if (itor != m_mCustomFonts.end())
			return itor->second;

		return GetDefaultFontInfo();
	}

	void CResourceManagerUI::RemoveAllFonts()
	{
		for (auto itor = m_mCustomFonts.begin(); itor != m_mCustomFonts.end(); itor	++)
		{
			if (itor->second) 
			{
				::DeleteObject(itor->second->hFont);
				delete itor->second;
				itor->second = nullptr;
			}
		}

		m_mCustomFonts.clear();
	}

	const TImageInfo * CResourceManagerUI::GetImage(LPCTSTR bitmap)
	{
		auto itor = m_mImageHash.find(bitmap);
		if (itor != m_mImageHash.end())
			return itor->second;
		
		return nullptr;
	}

	const TImageInfo * CResourceManagerUI::AddImage(LPCTSTR bitmap, HBITMAP hBitmap, int iWidth, int iHeight, bool bAlpha)
	{
		if (bitmap)
			return nullptr;

		if (!hBitmap || iWidth <= 0 || iHeight <= 0) 
			return nullptr;

		TImageInfo* data = new TImageInfo;
		data->hBitmap = hBitmap;
		data->pBits = nullptr;
		data->nX = iWidth;
		data->nY = iHeight;
		data->bAlpha = bAlpha;
		data->pSrcBits = nullptr;
		data->dwMask = 0;

		m_mImageHash[bitmap] = data;
		return data;
	}

	void CResourceManagerUI::RemoveImage(LPCTSTR bitmap)
	{
		auto itor = m_mImageHash.find(bitmap);
		if (itor != m_mImageHash.end())
		{
			if (itor->second->hBitmap)
			{
				::DeleteObject(itor->second->hBitmap);
				itor->second = nullptr;
			}

			if (itor->second->pSrcBits)
			{
				delete[] itor->second->pSrcBits;
				itor->second->pSrcBits = nullptr;
			}

			delete itor->second;
			itor->second = nullptr;
			m_mImageHash.erase(itor);
		}
	}

	void CResourceManagerUI::RemoveAllImages()
	{
		for (auto itor = m_mImageHash.begin(); itor != m_mImageHash.end(); itor++)
		{
			if (itor->second->hBitmap)
			{
				::DeleteObject(itor->second->hBitmap);
				itor->second = nullptr;
			}

			if (itor->second->pSrcBits)
			{
				delete[] itor->second->pSrcBits;
				itor->second->pSrcBits = nullptr;
			}

			delete itor->second;
			itor->second = nullptr;
		}

		m_mImageHash.clear();
	}

	void CResourceManagerUI::AddMultiLanguageString(int id, LPCTSTR pStrMultiLanguage)
	{
		if (id < 0 || !pStrMultiLanguage)
			return;

		m_mMultiLanguageHash[id] = pStrMultiLanguage;
	}

	tstring CResourceManagerUI::GetMultiLanguageString(int id)
	{
		auto itor = m_mMultiLanguageHash.find(id);
		if (itor != m_mMultiLanguageHash.end())
			return itor->second;
		
		return _T("");
	}

	bool CResourceManagerUI::RemoveMultiLanguageString(int id)
	{
		auto itor = m_mMultiLanguageHash.find(id);
		if (itor != m_mMultiLanguageHash.end())
		{
			m_mMultiLanguageHash.erase(itor);
			return true;
		}

		return false;
	}

	void CResourceManagerUI::RemoveAllMultiLanguageString()
	{
		m_mMultiLanguageHash.clear();
	}

	void CResourceManagerUI::AddDefaultResourceAttribute(const tstring & strName, const tstring & strValue)
	{
		if (strName.compare(_T("disabledfontcolor")) == 0)
		{
			LPCTSTR pstrValue = strValue.c_str();
			if (*pstrValue == _T('#'))
				pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetDefaultDisabledColor(clrColor);
		}
		else if (strName.compare(_T("defaultfontcolor")) == 0)
		{
			LPCTSTR pstrValue = strValue.c_str();
			if (*pstrValue == _T('#'))
				pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetDefaultFontColor(clrColor);
		}
		else if (strName.compare(_T("linkfontcolor")) == 0)
		{
			LPCTSTR pstrValue = strValue.c_str();
			if (*pstrValue == _T('#'))
				pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetDefaultLinkFontColor(clrColor);
		}
		else if (strName.compare(_T("linkhoverfontcolor")) == 0)
		{
			LPCTSTR pstrValue = strValue.c_str();
			if (*pstrValue == _T('#'))
				pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetDefaultLinkHoverFontColor(clrColor);
		}
		else if (strName.compare(_T("selectedcolor")) == 0)
		{
			LPCTSTR pstrValue = strValue.c_str();
			if (*pstrValue == _T('#'))
				pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetDefaultSelectedBkColor(clrColor);
		}
	}
}