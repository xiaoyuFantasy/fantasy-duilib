#include "StdAfx.h"
#include <zmouse.h>
#include <stdlib.h>
#include "UIManager.h"

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

	/////////////////////////////////////////////////////////////////////////////////////
	typedef BOOL(__stdcall *PFUNCUPDATELAYEREDWINDOW)(HWND, HDC, POINT*, SIZE*, HDC, POINT*, COLORREF, BLENDFUNCTION*, DWORD);
	PFUNCUPDATELAYEREDWINDOW g_fUpdateLayeredWindow = NULL;

	HPEN m_hUpdateRectPen = NULL;

	std::vector<CPaintManagerUI*> CPaintManagerUI::m_aPreMessages;
	std::vector<tstring> CPaintManagerUI::m_vPlugins;

	CPaintManagerUI::CPaintManagerUI() :
		m_iHoverTime(1000),
		m_bNoActivate(false),
		m_bShowUpdateRect(false),
		m_uTimerID(0x1000),
		m_bFirstLayout(true),
		m_bFocusNeeded(false),
		m_bUpdateNeeded(false),
		m_bMouseTracking(false),
		m_bMouseCapture(false),
		m_bIsPainting(false),
		m_bOffscreenPaint(true),
		m_nOpacity(0xFF),
		m_bLayered(false),
		m_bLayeredChanged(false),
		m_bDragMode(false),
		m_bDropEnable(false),
		m_pDropTarget(nullptr)
	{
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

	bool CPaintManagerUI::AttachDialog(CControlUI * pControl)
	{
		ASSERT(::IsWindow(m_hWndPaint));
		m_pEventKey = nullptr;
		m_pEventClick = nullptr;
		m_pEventHover = nullptr;
		m_pLastToolTip = nullptr;

		if (m_pRoot)
		{
			for (int i = 0; i < m_vDelayedCleanup.size(); i++)
				static_cast<CControlUI*>(m_vDelayedCleanup[i])->Delete();
			m_vDelayedCleanup.clear();
			m_mNameHash.clear();
			AddDelayedCleanup(m_pRoot);
		}

		m_pRoot = pControl;
		if (m_shadow.IsShowShadow())
			m_shadow.Create(this);

		return InitControls(pControl);
	}

	bool CPaintManagerUI::InitControls(CControlUI * pControl, CControlUI * pParent)
	{
		ASSERT(pControl);
		if (pControl == NULL)
			return false;
		pControl->SetManager(this, pParent != NULL ? pParent : pControl->GetParent(), true);
		pControl->FindControl(__FindControlFromNameHash, this, UIFIND_ALL);
		return true;
	}

	void CPaintManagerUI::ReapObjects(CControlUI * pControl)
	{
		if (pControl == NULL) return;
		if (pControl == m_pEventKey) m_pEventKey = NULL;
		if (pControl == m_pEventHover) m_pEventHover = NULL;
		if (pControl == m_pEventClick) m_pEventClick = NULL;
		if (pControl == m_pFocus) m_pFocus = NULL;

		const tstring& sName = pControl->GetName();
		if (!sName.empty()) {
			auto itor = m_mNameHash.find(sName);
			if (itor != m_mNameHash.end())
				m_mNameHash.erase(itor);
		}

	}

	tstring CPaintManagerUI::GetName() const
	{
		return m_sName;
	}

	CDuiPoint CPaintManagerUI::GetMousePos() const
	{
		return m_ptLastMousePos;
	}

	CDuiSize CPaintManagerUI::GetClientSize() const
	{
		RECT rcClient = { 0 };
		::GetClientRect(m_hWndPaint, &rcClient);
		return CDuiSize(rcClient.right - rcClient.left, rcClient.bottom - rcClient.top);
	}

	CDuiSize CPaintManagerUI::GetInitSize()
	{
		return m_szInitWindowSize;
	}

	void CPaintManagerUI::SetInitSize(int cx, int cy)
	{
		m_szInitWindowSize.cx = cx;
		m_szInitWindowSize.cy = cy;
		if (m_pRoot == NULL && m_hWndPaint != NULL) {
			::SetWindowPos(m_hWndPaint, NULL, 0, 0, cx, cy, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
		}
	}

	CDuiRect CPaintManagerUI::GetSizeBox() const
	{
		return m_rcSizeBox;
	}

	void CPaintManagerUI::SetSizeBox(CDuiRect & rcSizeBox)
	{
		m_rcSizeBox = rcSizeBox;
	}

	CDuiRect & CPaintManagerUI::GetCaptionRect()
	{
		// TODO: 在此处插入 return 语句
		return m_rcCaption;
	}

	void CPaintManagerUI::SetCaptionRect(CDuiRect & rcCaption)
	{
		m_rcCaption = rcCaption;
	}

	CDuiSize CPaintManagerUI::GetRoundCorner() const
	{
		return m_szRoundCorner;
	}

	void CPaintManagerUI::SetRoundCorner(int cx, int cy)
	{
		m_szRoundCorner.cx = cx;
		m_szRoundCorner.cy = cy;
	}

	CDuiSize CPaintManagerUI::GetMinInfo() const
	{
		return m_szMinWindow;
	}

	void CPaintManagerUI::SetMinInfo(int cx, int cy)
	{
		ASSERT(cx >= 0 && cy >= 0);
		m_szMinWindow.cx = cx;
		m_szMinWindow.cy = cy;
	}

	CDuiSize CPaintManagerUI::GetMaxInfo() const
	{
		return m_szMaxWindow;
	}

	void CPaintManagerUI::SetMaxInfo(int cx, int cy)
	{
		ASSERT(cx >= 0 && cy >= 0);
		m_szMaxWindow.cx = cx;
		m_szMaxWindow.cy = cy;
	}

	BYTE CPaintManagerUI::GetOpacity() const
	{
		return m_nOpacity;
	}

	void CPaintManagerUI::SetOpacity(BYTE nOpacity)
	{
		m_nOpacity = nOpacity;
		if (m_hWndPaint) {
			typedef BOOL(__stdcall *PFUNCSETLAYEREDWINDOWATTR)(HWND, COLORREF, BYTE, DWORD);
			PFUNCSETLAYEREDWINDOWATTR fSetLayeredWindowAttributes = NULL;

			HMODULE hUser32 = ::GetModuleHandle(_T("User32.dll"));
			if (hUser32)
			{
				fSetLayeredWindowAttributes =
					(PFUNCSETLAYEREDWINDOWATTR)::GetProcAddress(hUser32, "SetLayeredWindowAttributes");
				if (fSetLayeredWindowAttributes == NULL) return;
			}

			DWORD dwStyle = ::GetWindowLong(m_hWndPaint, GWL_EXSTYLE);
			DWORD dwNewStyle = dwStyle;
			if (nOpacity >= 0 && nOpacity < 256) dwNewStyle |= WS_EX_LAYERED;
			else dwNewStyle &= ~WS_EX_LAYERED;
			if (dwStyle != dwNewStyle) ::SetWindowLong(m_hWndPaint, GWL_EXSTYLE, dwNewStyle);
			fSetLayeredWindowAttributes(m_hWndPaint, 0, nOpacity, LWA_ALPHA);

			m_bLayered = false;
			Invalidate();
		}
	}

	bool CPaintManagerUI::IsLayered()
	{
		return m_bLayered;
	}

	void CPaintManagerUI::SetLayered(bool bLayered)
	{
		if (m_hWndPaint != NULL && bLayered != m_bLayered) {
			UINT uStyle = GetWindowStyle(m_hWndPaint);
			if ((uStyle & WS_CHILD) != 0) return;

			if (g_fUpdateLayeredWindow == NULL) {
				HMODULE hUser32 = ::GetModuleHandle(_T("User32.dll"));
				if (hUser32) {
					g_fUpdateLayeredWindow =
						(PFUNCUPDATELAYEREDWINDOW)::GetProcAddress(hUser32, "UpdateLayeredWindow");
					if (g_fUpdateLayeredWindow == NULL) return;
				}
			}
			DWORD dwExStyle = ::GetWindowLong(m_hWndPaint, GWL_EXSTYLE);
			DWORD dwNewExStyle = dwExStyle;
			if (bLayered) {
				dwNewExStyle |= WS_EX_LAYERED;
				::SetTimer(m_hWndPaint, LAYEREDUPDATE_TIMERID, 10L, NULL);
			}
			else {
				dwNewExStyle &= ~WS_EX_LAYERED;
				::KillTimer(m_hWndPaint, LAYEREDUPDATE_TIMERID);
			}
			if (dwExStyle != dwNewExStyle) ::SetWindowLong(m_hWndPaint, GWL_EXSTYLE, dwNewExStyle);
			m_bLayered = bLayered;
			if (m_pRoot != NULL) m_pRoot->NeedUpdate();
			Invalidate();
		}
	}

	bool CPaintManagerUI::IsNoActivate()
	{
		return m_bNoActivate;
	}

	void CPaintManagerUI::SetNoActivate(bool bNoActivate)
	{
		m_bNoActivate = bNoActivate;
	}

	bool CPaintManagerUI::GetWindowAttribute(const tstring & strName, tstring & strValue)
	{
		auto itor = m_mWindowAttrHash.find(strName);
		if (itor != m_mWindowAttrHash.end())
		{
			strValue = itor->second;
			return true;
		}

		return false;
	}


	void CPaintManagerUI::SetWindowAttribute(const tstring & strName, const tstring & strValue)
	{
		if (strName.compare(_T("size")) == 0) 
		{
			LPTSTR pstr = NULL;
			int cx = _tcstol(strValue.c_str(), &pstr, 10);  
			ASSERT(pstr);
			int cy = _tcstol(pstr + 1, &pstr, 10);    
			ASSERT(pstr);
			SetInitSize(cx, cy);
		}
		else if (strName.compare(_T("sizebox")) == 0) 
		{
			CDuiRect rcSizeBox = { 0 };
			LPTSTR pstr = NULL;
			rcSizeBox.left = _tcstol(strValue.c_str(), &pstr, 10);  
			ASSERT(pstr);
			rcSizeBox.top = _tcstol(pstr + 1, &pstr, 10);    
			ASSERT(pstr);
			rcSizeBox.right = _tcstol(pstr + 1, &pstr, 10);  
			ASSERT(pstr);
			rcSizeBox.bottom = _tcstol(pstr + 1, &pstr, 10); 
			ASSERT(pstr);
			SetSizeBox(rcSizeBox);
		}
		else if (strName.compare(_T("caption")) == 0) 
		{
			CDuiRect rcCaption = { 0 };
			LPTSTR pstr = NULL;
			rcCaption.left = _tcstol(strValue.c_str(), &pstr, 10);  
			ASSERT(pstr);
			rcCaption.top = _tcstol(pstr + 1, &pstr, 10);    
			ASSERT(pstr);
			rcCaption.right = _tcstol(pstr + 1, &pstr, 10);  
			ASSERT(pstr);
			rcCaption.bottom = _tcstol(pstr + 1, &pstr, 10); 
			ASSERT(pstr);
			SetCaptionRect(rcCaption);
		}
		else if (strName.compare(_T("roundcorner")) == 0) 
		{
			LPTSTR pstr = NULL;
			int cx = _tcstol(strValue.c_str(), &pstr, 10);  
			ASSERT(pstr);
			int cy = _tcstol(pstr + 1, &pstr, 10);    
			ASSERT(pstr);
			SetRoundCorner(cx, cy);
		}
		else if (strName.compare(_T("mininfo")) == 0) 
		{
			LPTSTR pstr = NULL;
			int cx = _tcstol(strValue.c_str(), &pstr, 10);  
			ASSERT(pstr);
			int cy = _tcstol(pstr + 1, &pstr, 10);    
			ASSERT(pstr);
			SetMinInfo(cx, cy);
		}
		else if (strName.compare(_T("maxinfo")) == 0) 
		{
			LPTSTR pstr = NULL;
			int cx = _tcstol(strValue.c_str(), &pstr, 10);  
			ASSERT(pstr);
			int cy = _tcstol(pstr + 1, &pstr, 10);    
			ASSERT(pstr);
			SetMaxInfo(cx, cy);
		}
		else if (strName.compare(_T("noactivate")) == 0) 
			SetNoActivate(strValue.compare(_T("true")) == 0);
		else if (strName.compare(_T("opacity")) == 0)
			SetOpacity(stoi(strValue));
		else if (strName.compare(_T("shadowsize")) == 0)
			m_shadow.SetSize(stoi(strValue));
		else if (strName.compare(_T("shadowsharpness")) == 0) 
			m_shadow.SetSharpness(stoi(strValue));
		else if (strName.compare(_T("shadowdarkness")) == 0) 
			m_shadow.SetDarkness(stoi(strValue));
		else if (strName.compare(_T("shadowposition")) == 0) 
		{
			LPCTSTR pstrValue = strValue.c_str();
			LPTSTR pstr = NULL;
			int cx = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);
			int cy = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
			m_shadow.SetPosition(cx, cy);
		}
		else if (strName.compare(_T("shadowcolor")) == 0) 
		{
			LPCTSTR pstrValue = strValue.c_str();
			if (*pstrValue == _T('#'))
				pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			m_shadow.SetColor(clrColor);
		}
		else if (strName.compare(_T("shadowcorner")) == 0) 
		{
			LPCTSTR pstrValue = strValue.c_str();
			RECT rcCorner = { 0 };
			LPTSTR pstr = NULL;
			rcCorner.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);
			rcCorner.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
			rcCorner.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
			rcCorner.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
			m_shadow.SetShadowCorner(rcCorner);
		}
		else if (strName.compare(_T("shadowimage")) == 0)
		{
			//m_shadow.SetImage(strValue);
		}
		else if (strName.compare(_T("showshadow")) == 0)
			m_shadow.ShowShadow(strValue.compare(_T("true")) == 0);
		else if (strName.compare(_T("drop")) == 0)
			SetDropEnable(strValue.compare(_T("true")) == 0);
		else
			AddWindowCustomAttribute(strName, strValue);

		m_mWindowAttrHash[strName] = strValue;
	}

	void CPaintManagerUI::AddWindowCustomAttribute(const tstring & strName, const tstring & strAttr)
	{
		m_mWindowCustomAttrHash[strName] = strAttr;
	}

	tstring CPaintManagerUI::GetWindowCustomAttribute(const tstring & strName) const
	{
		auto itor = m_mWindowCustomAttrHash.find(strName);
		if (itor != m_mWindowCustomAttrHash.end())
			return itor->second;
		return _T("");
	}

	bool CPaintManagerUI::RemoveWindowCustomAttribute(const tstring & strName)
	{
		auto itor = m_mWindowCustomAttrHash.find(strName);
		if (itor != m_mWindowCustomAttrHash.end())
		{
			m_mWindowCustomAttrHash.erase(itor);
			return true;
		}

		return false;
	}

	void CPaintManagerUI::RemoveAllWindowCustomAttribute()
	{
		m_mWindowCustomAttrHash.clear();
	}

	HDC CPaintManagerUI::GetPaintDC() const
	{
		return m_hDcPaint;
	}

	void CPaintManagerUI::Invalidate()
	{
		if (!m_bLayered) 
			::InvalidateRect(m_hWndPaint, NULL, FALSE);
		else {
			RECT rcClient = { 0 };
			::GetClientRect(m_hWndPaint, &rcClient);
			::UnionRect(&m_rcLayeredUpdate, &m_rcLayeredUpdate, &rcClient);
		}
	}

	void CPaintManagerUI::Invalidate(CDuiRect & rcItem)
	{
		if (rcItem.left < 0) 
			rcItem.left = 0;
		if (rcItem.top < 0) 
			rcItem.top = 0;
		if (rcItem.right < rcItem.left) 
			rcItem.right = rcItem.left;
		if (rcItem.bottom < rcItem.top) 
			rcItem.bottom = rcItem.top;
		if (!m_bLayered) 
			::InvalidateRect(m_hWndPaint, &rcItem, FALSE);
		else 
			::UnionRect(&m_rcLayeredUpdate, &m_rcLayeredUpdate, &rcItem);
	}

	bool CPaintManagerUI::SetDropEnable(bool bDrop)
	{
		HRESULT hRet = S_OK;
		if (bDrop)
		{
			hRet = m_pDropTarget->DragDropRegister(m_hWndPaint, this);
			if (hRet == S_OK)
			{
				m_bDropEnable = true;
			}
			else
			{
				m_bDropEnable = false;
			}
		}
		else
		{
			hRet = m_pDropTarget->DragDropRevoke(m_hWndPaint);
			m_bDropEnable = false;
		}
		return SUCCEEDED(hRet);
	}

	HRESULT CPaintManagerUI::OnDragEnter(IDataObject * pDataObj, DWORD grfKeyState, POINTL ptl, DWORD * pdwEffect)
	{
		m_pCurDataObject = pDataObj;
		POINT pt = { ptl.x,ptl.y };
		::ScreenToClient(m_hWndPaint, &pt);
		CControlUI* pHover = FindControl(pt);
		if (pHover == NULL)
		{
			*pdwEffect = DROPEFFECT_NONE;
			return S_OK;
		}
		// Generate mouse hover event  
		pHover->OnDragEnter(pDataObj, grfKeyState, pt, pdwEffect);
		m_pEventDrop = pHover;
		return S_OK;
	}

	HRESULT CPaintManagerUI::OnDragOver(DWORD grfKeyState, POINTL ptl, DWORD * pdwEffect)
	{
		POINT pt = { ptl.x, ptl.y };
		::ScreenToClient(m_hWndPaint, &pt);
		CControlUI* pNewHover = FindControl(pt);
		if (pNewHover == NULL)
		{
			*pdwEffect = DROPEFFECT_NONE;
			return S_OK;
		}

		if (pNewHover != NULL && pNewHover->GetManager() != this)
		{
			*pdwEffect = DROPEFFECT_NONE;
			return S_OK;
		}

		if (pNewHover != m_pEventDrop && m_pEventDrop != NULL) {
			m_pEventDrop->OnDragLeave();
			m_pEventDrop = NULL;
		}

		if (pNewHover != m_pEventDrop && pNewHover != NULL) {
			pNewHover->OnDragEnter(m_pCurDataObject, grfKeyState, pt, pdwEffect);
			m_pEventDrop = pNewHover;
		}

		if (pNewHover != NULL) {
			pNewHover->OnDragOver(grfKeyState, pt, pdwEffect);
		}
		return S_OK;
	}

	HRESULT CPaintManagerUI::OnDragLeave()
	{
		m_pCurDataObject = NULL;
		if (m_pEventDrop != NULL) {
			m_pEventDrop->OnDragLeave();
			m_pEventDrop = NULL;
		}
		return S_OK;
	}

	HRESULT CPaintManagerUI::OnDrop(IDataObject * pDataObj, DWORD grfKeyState, POINTL ptl, DWORD * pdwEffect)
	{
		POINT pt = { ptl.x, ptl.y };
		::ScreenToClient(m_hWndPaint, &pt);
		if (m_pEventDrop != NULL) {

			m_pEventDrop->OnDrop(pDataObj, grfKeyState, pt, pdwEffect);
		}
		else
		{
			*pdwEffect = DROPEFFECT_NONE;
			return S_OK;
		}
		return S_OK;
	}

	bool CPaintManagerUI::AddOptionGroup(LPCTSTR pStrGroupName, CControlUI * pControl)
	{
		if (!pStrGroupName || !pControl)
			return false;

		auto itor = m_mOptionGroup.find(pStrGroupName);
		if (itor != m_mOptionGroup.end())
		{
			auto itor2 = std::find_if(itor->second.begin(), itor->second.end(), [&](CControlUI* p) {
				return p = pControl;
			});

			if (itor2 == itor->second.end())
				return false;

			itor->second.push_back(pControl);
		}

		return false;
	}

	std::vector<CControlUI*>* CPaintManagerUI::GetOptionGroup(LPCTSTR pStrGroupName)
	{
		auto itor = m_mOptionGroup.find(pStrGroupName);
		if (itor != m_mOptionGroup.end())
			return &itor->second;

		return nullptr;
	}

	void CPaintManagerUI::RemoveAllOptionGroups()
	{
		m_mOptionGroup.clear();
	}

	CControlUI * CPaintManagerUI::GetFocus() const
	{
		return m_pFocus;
	}

	void CPaintManagerUI::SetFocus(CControlUI * pControl, bool bFocusWnd)
	{
		// Paint manager window has focus ?
		HWND hFocusWnd = ::GetFocus();
		if (bFocusWnd && hFocusWnd != m_hWndPaint && pControl != m_pFocus && !m_bNoActivate) 
			::SetFocus(m_hWndPaint);
		// Already has focus?
		if (pControl == m_pFocus) 
			return;
		// Remove focus from old control
		if (m_pFocus != NULL)
		{
			TEventUI event = { 0 };
			event.Type = UIEVENT_KILLFOCUS;
			event.pSender = pControl;
			event.dwTimestamp = ::GetTickCount();
			m_pFocus->Event(event);
			SendNotify(m_pFocus, DUI_MSGTYPE_KILLFOCUS);
			m_pFocus = NULL;
		}
		if (pControl == NULL) 
			return;
		// Set focus to new control
		if (pControl != NULL
			&& pControl->GetManager() == this
			&& pControl->IsVisible()
			&& pControl->IsEnabled())
		{
			m_pFocus = pControl;
			TEventUI event = { 0 };
			event.Type = UIEVENT_SETFOCUS;
			event.pSender = pControl;
			event.dwTimestamp = ::GetTickCount();
			m_pFocus->Event(event);
			SendNotify(m_pFocus, DUI_MSGTYPE_SETFOCUS);
		}
	}

	void CPaintManagerUI::SetFocusNeeded(CControlUI * pControl)
	{
	}

	void CPaintManagerUI::KillFocus()
	{
	}

	bool CPaintManagerUI::SetNextTabControl(bool bForward)
	{
		// Find next/previous tabbable control
		FINDTABINFO info1 = { 0 };
		info1.pFocus = m_pFocus;
		info1.bForward = bForward;
		CControlUI* pControl = m_pRoot->FindControl(__FindControlFromTab, &info1, UIFIND_VISIBLE | UIFIND_ENABLED | UIFIND_ME_FIRST);
		if (pControl == NULL) 
		{
			if (bForward)
			{
				// Wrap around
				FINDTABINFO info2 = { 0 };
				info2.pFocus = bForward ? NULL : info1.pLast;
				info2.bForward = bForward;
				pControl = m_pRoot->FindControl(__FindControlFromTab, &info2, UIFIND_VISIBLE | UIFIND_ENABLED | UIFIND_ME_FIRST);
			}
			else 
			{
				pControl = info1.pLast;
			}
		}
		if (pControl != NULL) SetFocus(pControl);
		m_bFocusNeeded = false;
		return true;
	}

	void CPaintManagerUI::SetCapture()
	{
		::SetCapture(m_hWndPaint);
		m_bMouseCapture = true;
	}

	void CPaintManagerUI::ReleaseCapture()
	{
		::ReleaseCapture();
		m_bMouseCapture = false;
	}

	bool CPaintManagerUI::IsCaptured()
	{
		return m_bMouseCapture;
	}

	bool CPaintManagerUI::IsPainting()
	{
		return m_bIsPainting;
	}

	void CPaintManagerUI::SetPainting(bool bIsPainting)
	{
		m_bIsPainting = bIsPainting;
	}

	bool CPaintManagerUI::AddPreMessageFilter(IMessageFilterUI * pFilter)
	{
		if (!pFilter)
			return false;

		auto itor = std::find_if(m_vPreMessageFilters.begin(), m_vPreMessageFilters.end(), [&](IMessageFilterUI* pItem) {
			return pItem == pFilter;
		});

		if (itor != m_vPreMessageFilters.end())
			return false;

		m_vPreMessageFilters.push_back(pFilter);
		return true;
	}

	bool CPaintManagerUI::RemovePreMessageFilter(IMessageFilterUI * pFilter)
	{
		if (!pFilter)
			return;

		auto itor = std::find_if(m_vPreMessageFilters.begin(), m_vPreMessageFilters.end(), [&](IMessageFilterUI* pItem) {
			return pItem == pFilter;
		});

		if (itor == m_vPreMessageFilters.end())
			return false;

		m_vPreMessageFilters.erase(itor);
		return true;
	}

	bool CPaintManagerUI::AddMessageFilter(IMessageFilterUI * pFilter)
	{
		if (!pFilter)
			return false;

		auto itor = std::find_if(m_vMessageFilters.begin(), m_vMessageFilters.end(), [&](IMessageFilterUI* pItem) {
			return pItem == pFilter;
		});

		if (itor != m_vMessageFilters.end())
			return false;

		m_vMessageFilters.push_back(pFilter);
		return true;
	}

	bool CPaintManagerUI::RemoveMessageFilter(IMessageFilterUI * pFilter)
	{
		if (!pFilter)
			return;

		auto itor = std::find_if(m_vMessageFilters.begin(), m_vMessageFilters.end(), [&](IMessageFilterUI* pItem) {
			return pItem == pFilter;
		});

		if (itor == m_vMessageFilters.end())
			return false;

		m_vMessageFilters.erase(itor);
		return true;
	}

	bool CPaintManagerUI::AddTranslateAccelerator(ITranslateAccelerator * pTranslateAccelerator)
	{
		if (!pTranslateAccelerator)
			return false;

		auto itor = std::find_if(m_vTranslateAccelerator.begin(), m_vTranslateAccelerator.end(), [&](ITranslateAccelerator* pItem) {
			return pItem == pTranslateAccelerator;
		});

		if (itor != m_vTranslateAccelerator.end())
			return false;

		m_vTranslateAccelerator.push_back(pTranslateAccelerator);
		return true;
	}

	bool CPaintManagerUI::RemoveTranslateAccelerator(ITranslateAccelerator * pTranslateAccelerator)
	{
		if (!pTranslateAccelerator)
			return;

		auto itor = std::find_if(m_vTranslateAccelerator.begin(), m_vTranslateAccelerator.end(), [&](ITranslateAccelerator* pItem) {
			return pItem == pTranslateAccelerator;
		});

		if (itor == m_vTranslateAccelerator.end())
			return false;

		m_vTranslateAccelerator.erase(itor);
		return true;
	}

	CControlUI * CPaintManagerUI::GetRoot() const
	{
		ASSERT(m_pRoot);
		return m_pRoot;
	}

	CControlUI * CPaintManagerUI::FindControl(POINT pt) const
	{
		ASSERT(m_pRoot);
		return m_pRoot->FindControl(__FindControlFromPoint, &pt, UIFIND_VISIBLE | UIFIND_HITTEST | UIFIND_TOP_FIRST);
	}

	CControlUI * CPaintManagerUI::FindControl(LPCTSTR pstrName) const
	{
		ASSERT(m_pRoot);
		auto itor = m_mNameHash.find(pstrName);
		if (itor == m_mNameHash.end())
			return nullptr;

		return itor->second;
	}

	CControlUI * CPaintManagerUI::FindSubControlByPoint(CControlUI * pParent, POINT pt) const
	{
		if (pParent == NULL) 
			pParent = GetRoot();
		ASSERT(pParent);
		return pParent->FindControl(__FindControlFromPoint, &pt, UIFIND_VISIBLE | UIFIND_HITTEST | UIFIND_TOP_FIRST);
	}

	CControlUI * CPaintManagerUI::FindSubControlByName(CControlUI * pParent, LPCTSTR pstrName) const
	{
		if (pParent == NULL) 
			pParent = GetRoot();
		ASSERT(pParent);
		return pParent->FindControl(__FindControlFromName, (LPVOID)pstrName, UIFIND_ALL);
	}

	CControlUI * CPaintManagerUI::FindSubControlByClass(CControlUI * pParent, LPCTSTR pstrClass, int iIndex)
	{
		if (pParent == NULL) 
			pParent = GetRoot();
		ASSERT(pParent);
		m_vFoundControls.resize(iIndex + 1);
		return pParent->FindControl(__FindControlFromClass, (LPVOID)pstrClass, UIFIND_ALL);
	}

	std::vector<CControlUI*>& CPaintManagerUI::FindSubControlsByClass(CControlUI * pParent, LPCTSTR pstrClass)
	{
		if (pParent == NULL) 
			pParent = GetRoot();
		ASSERT(pParent);
		m_vFoundControls.clear();
		pParent->FindControl(__FindControlsFromClass, (LPVOID)pstrClass, UIFIND_ALL);
		return m_vFoundControls;

	}

	void CPaintManagerUI::AddDelayedCleanup(CControlUI * pControl)
	{
		if (pControl == NULL) 
			return;
		pControl->SetManager(this, NULL, false);
		m_vDelayedCleanup.push_back(pControl);
	}

	bool CPaintManagerUI::TranslateMessage(const LPMSG pMsg)
	{
		UINT uStyle = GetWindowStyle(pMsg->hwnd);
		UINT uChildRes = uStyle & WS_CHILD;
		LRESULT lRes = 0;
	}

	bool CPaintManagerUI::TranslateAccelerator(LPMSG pMsg)
	{
		for (int i = 0; i < m_vTranslateAccelerator.size(); i++)
		{
			LRESULT lResult = m_vTranslateAccelerator[i]->TranslateAccelerator(pMsg);
			if (lResult == S_OK) 
				return true;
		}
		return false;
	}

	bool CPaintManagerUI::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT & lRes)
	{
#ifdef _DEBUG
	switch (uMsg) 
	{
	case WM_NCPAINT:
	case WM_NCHITTEST:
	case WM_SETCURSOR:
		break;
	default:
		DUITRACE(_T("MSG: %-20s (%08ld)"), DUITRACEMSG(uMsg), ::GetTickCount());
	}
#endif
		if (m_hWndPaint == NULL) 
			return false;

		for (int i = 0; i < m_vMessageFilters.size(); i++)
		{
			bool bHandled = false;
			LRESULT lResult = m_vMessageFilters[i]->MessageHandler(uMsg, wParam, lParam, bHandled);
			if (bHandled)
			{
				lRes = lResult;
				switch (uMsg) 
				{
				case WM_MOUSEMOVE:
				case WM_LBUTTONDOWN:
				case WM_LBUTTONDBLCLK:
				case WM_LBUTTONUP:
				{
					POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
					m_ptLastMousePos = pt;
				}
				break;
				case WM_CONTEXTMENU:
				case WM_MOUSEWHEEL:
				{
					POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
					::ScreenToClient(m_hWndPaint, &pt);
					m_ptLastMousePos = pt;
				}
				break;
				}
				return true;
			}
		}

		switch (uMsg)
		{
		case WM_DESTROY:break;
		case WM_MOVE:break;
		case WM_SIZE:break;
		case WM_ACTIVATE:break;
		case WM_SETFOCUS:break;
		case WM_KILLFOCUS:break;
		case WM_ENABLE:break;
		case WM_PAINT:break;
		case WM_CLOSE: break;
		case WM_QUERYENDSESSION:break;
		case WM_ERASEBKGND: break;
		case WM_ENDSESSION: break;
		case WM_SHOWWINDOW:break;
		case WM_SETCURSOR:break;
		case WM_MOUSEACTIVATE:break;
		case WM_GETMINMAXINFO: break;
		case WM_WINDOWPOSCHANGING:break;
		case WM_WINDOWPOSCHANGED:break;
		case WM_COPYDATA:break;
		case WM_NOTIFY:break;
		case WM_CONTEXTMENU:break;
		case WM_NCPAINT:break;
		case WM_CHAR:break;
		case WM_KEYDOWN:break;
		case WM_KEYUP:break;
		case WM_COMMAND:break;
		case WM_SYSCOMMAND:break;
		case WM_TIMER:break;
		case WM_CTLCOLOREDIT:
		case WM_CTLCOLORSTATIC:break;
		case WM_MOUSEHOVER:break;
		case WM_MOUSELEAVE:break;
		case WM_MOUSEMOVE:break;
		case WM_LBUTTONDOWN:break;
		case WM_LBUTTONUP: break;
		case WM_LBUTTONDBLCLK:break;
		case WM_RBUTTONDOWN: break;
		case WM_RBUTTONUP:break;
		case WM_RBUTTONDBLCLK:break;
		case WM_MOUSEWHEEL:break;
		case WM_SIZING:break;
		default:
			break;
		}

		return false;
	}

	bool CPaintManagerUI::PreMessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT & lRes)
	{
		return false;
	}




} // namespace DuiLib_Lite
