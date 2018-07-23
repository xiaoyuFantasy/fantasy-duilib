#include "StdAfx.h"
#include <zmouse.h>
#include <stdlib.h>
#include "UIManager.h"

DECLARE_HANDLE(HZIP);	// An HZIP identifies a zip file that has been opened
typedef DWORD ZRESULT;
#define OpenZip OpenZipU
#define CloseZip(hz) CloseZipU(hz)
extern HZIP OpenZipU(void *z, unsigned int len, DWORD flags);
extern ZRESULT CloseZipU(HZIP hz);

namespace DuiLib_Lite {

	static UINT MapKeyState()
	{
		UINT uState = 0;
		if (::GetKeyState(VK_CONTROL) < 0) uState |= MK_CONTROL;
		if (::GetKeyState(VK_RBUTTON) < 0) uState |= MK_RBUTTON;
		if (::GetKeyState(VK_LBUTTON) < 0) uState |= MK_LBUTTON;
		if (::GetKeyState(VK_SHIFT) < 0) uState |= MK_SHIFT;
		if (::GetKeyState(VK_MENU) < 0) uState |= MK_ALT;
		return uState;
	}

	typedef struct tagFINDTABINFO
	{
		CControlUI* pFocus;
		CControlUI* pLast;
		bool bForward;
		bool bNextIsIt;
	} FINDTABINFO;

	typedef struct tagFINDSHORTCUT
	{
		TCHAR ch;
		bool bPickNext;
	} FINDSHORTCUT;

	typedef struct tagTIMERINFO
	{
		CControlUI* pSender;
		UINT nLocalID;
		HWND hWnd;
		UINT uWinTimer;
		bool bKilled;
	} TIMERINFO;

	/////////////////////////////////////////////////////////////////////////////////////

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

	/////////////////////////////////////////////////////////////////////////////////////
	typedef BOOL(__stdcall *PFUNCUPDATELAYEREDWINDOW)(HWND, HDC, POINT*, SIZE*, HDC, POINT*, COLORREF, BLENDFUNCTION*, DWORD);
	PFUNCUPDATELAYEREDWINDOW g_fUpdateLayeredWindow = NULL;

	HPEN m_hUpdateRectPen = NULL;

	std::vector<CPaintManagerUI*> CPaintManagerUI::m_aPreMessages;
	HINSTANCE CPaintManagerUI::m_hResourceInstance = NULL;
	tstring CPaintManagerUI::m_strResourcePath;
	tstring CPaintManagerUI::m_strResourceZip;
	HANDLE CPaintManagerUI::m_hResourceZip = NULL;
	TResInfo CPaintManagerUI::m_SharedResInfo;
	HINSTANCE CPaintManagerUI::m_hInstance = NULL;
	std::vector<tstring> CPaintManagerUI::m_vPlugins;

	CPaintManagerUI::CPaintManagerUI() :
		m_hWndPaint(NULL),
		m_hDcPaint(NULL),
		m_hDcOffscreen(NULL),
		m_hDcBackground(NULL),
		m_hbmpOffscreen(NULL),
		m_pOffscreenBits(NULL),
		m_hbmpBackground(NULL),
		m_pBackgroundBits(NULL),
		m_iTooltipWidth(-1),
		m_iLastTooltipWidth(-1),
		m_hwndTooltip(NULL),
		m_iHoverTime(1000),
		m_bNoActivate(false),
		m_bShowUpdateRect(false),
		m_uTimerID(0x1000),
		m_pRoot(NULL),
		m_pFocus(NULL),
		m_pEventHover(NULL),
		m_pEventClick(NULL),
		m_pEventKey(NULL),
		m_pLastToolTip(NULL),
		m_bFirstLayout(true),
		m_bFocusNeeded(false),
		m_bUpdateNeeded(false),
		m_bMouseTracking(false),
		m_bMouseCapture(false),
		m_bIsPainting(false),
		m_bOffscreenPaint(true),
		m_bUsedVirtualWnd(false),
		m_bAsyncNotifyPosted(false),
		m_bForceUseSharedRes(false),
		m_nOpacity(0xFF),
		m_bLayered(false),
		m_bLayeredChanged(false),
		m_bDragMode(false),
		m_bDropEnable(false),
		m_pDropTarget(nullptr)
	{
		if (m_SharedResInfo.m_DefaultFontInfo.sFontName.empty())
		{
			m_SharedResInfo.m_dwDefaultDisabledColor = 0xFFA7A6AA;
			m_SharedResInfo.m_dwDefaultFontColor = 0xFF000000;
			m_SharedResInfo.m_dwDefaultLinkFontColor = 0xFF0000FF;
			m_SharedResInfo.m_dwDefaultLinkHoverFontColor = 0xFFD3215F;
			m_SharedResInfo.m_dwDefaultSelectedBkColor = 0xFFBAE4FF;

			LOGFONT lf = { 0 };
			::GetObject(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf);
			lf.lfCharSet = DEFAULT_CHARSET;
			HFONT hDefaultFont = ::CreateFontIndirect(&lf);
			m_SharedResInfo.m_DefaultFontInfo.hFont = hDefaultFont;
			m_SharedResInfo.m_DefaultFontInfo.sFontName = lf.lfFaceName;
			m_SharedResInfo.m_DefaultFontInfo.iSize = -lf.lfHeight;
			m_SharedResInfo.m_DefaultFontInfo.bBold = (lf.lfWeight >= FW_BOLD);
			m_SharedResInfo.m_DefaultFontInfo.bUnderline = (lf.lfUnderline == TRUE);
			m_SharedResInfo.m_DefaultFontInfo.bItalic = (lf.lfItalic == TRUE);
			::ZeroMemory(&m_SharedResInfo.m_DefaultFontInfo.tm, sizeof(m_SharedResInfo.m_DefaultFontInfo.tm));
		}

		m_ResInfo.m_dwDefaultDisabledColor = m_SharedResInfo.m_dwDefaultDisabledColor;
		m_ResInfo.m_dwDefaultFontColor = m_SharedResInfo.m_dwDefaultFontColor;
		m_ResInfo.m_dwDefaultLinkFontColor = m_SharedResInfo.m_dwDefaultLinkFontColor;
		m_ResInfo.m_dwDefaultLinkHoverFontColor = m_SharedResInfo.m_dwDefaultLinkHoverFontColor;
		m_ResInfo.m_dwDefaultSelectedBkColor = m_SharedResInfo.m_dwDefaultSelectedBkColor;

		if (m_hUpdateRectPen == NULL) {
			m_hUpdateRectPen = ::CreatePen(PS_SOLID, 1, RGB(220, 0, 0));
			// Boot Windows Common Controls (for the ToolTip control)
			::InitCommonControls();
			::LoadLibrary(_T("msimg32.dll"));
		}

		m_szMinWindow.cx = 0;
		m_szMinWindow.cy = 0;
		m_szMaxWindow.cx = 0;
		m_szMaxWindow.cy = 0;
		m_szInitWindowSize.cx = 0;
		m_szInitWindowSize.cy = 0;
		m_szRoundCorner.cx = m_szRoundCorner.cy = 0;
		::ZeroMemory(&m_rcSizeBox, sizeof(m_rcSizeBox));
		::ZeroMemory(&m_rcCaption, sizeof(m_rcCaption));
		::ZeroMemory(&m_rcLayeredInset, sizeof(m_rcLayeredInset));
		::ZeroMemory(&m_rcLayeredUpdate, sizeof(m_rcLayeredUpdate));
		m_ptLastMousePos.x = m_ptLastMousePos.y = -1;
	}

	CPaintManagerUI::~CPaintManagerUI()
	{
		if (m_pDropTarget)
			m_pDropTarget->Release();

		// Delete the control-tree structures
		for (int i = 0; i < m_vDelayedCleanup.size(); i++)
			static_cast<CControlUI*>(m_vDelayedCleanup[i])->Delete();

		m_mNameHash.clear();

		if (m_pRoot != NULL)
			m_pRoot->Delete();

		::DeleteObject(m_ResInfo.m_DefaultFontInfo.hFont);
		RemoveAllFonts();
		RemoveAllImages();
		RemoveAllDefaultAttributeList();
		RemoveAllWindowCustomAttribute();
		RemoveAllOptionGroups();

		// Reset other parts...
		if (m_hwndTooltip != NULL)
		{
			::DestroyWindow(m_hwndTooltip);
			m_hwndTooltip = NULL;
		}
		m_pLastToolTip = NULL;
		if (m_hDcOffscreen != NULL) ::DeleteDC(m_hDcOffscreen);
		if (m_hDcBackground != NULL) ::DeleteDC(m_hDcBackground);
		if (m_hbmpOffscreen != NULL) ::DeleteObject(m_hbmpOffscreen);
		if (m_hbmpBackground != NULL) ::DeleteObject(m_hbmpBackground);
		if (m_hDcPaint != NULL) ::ReleaseDC(m_hWndPaint, m_hDcPaint);
	}

	void CPaintManagerUI::Init(HWND hWnd, LPCTSTR pstrName)
	{
		ASSERT(::IsWindow(hWnd));

		m_mNameHash.clear();
		RemoveAllFonts();
		RemoveAllImages();
		RemoveAllDefaultAttributeList();
		RemoveAllWindowCustomAttribute();
		RemoveAllOptionGroups();

		m_sName.clear();
		if (pstrName != NULL) m_sName = pstrName;

		m_pDropTarget = new CDropTargetEx;

		if (m_hWndPaint != hWnd) {
			m_hWndPaint = hWnd;
			m_hDcPaint = ::GetDC(hWnd);
			m_aPreMessages.push_back(this);
		}
	}



} // namespace DuiLib_Lite
