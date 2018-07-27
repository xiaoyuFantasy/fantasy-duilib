#pragma once

namespace DuiLib_Lite {
/////////////////////////////////////////////////////////////////////////////////////
//

class CControlUI;

/////////////////////////////////////////////////////////////////////////////////////
//

typedef enum EVENTTYPE_UI
{
    UIEVENT__FIRST = 1,
    UIEVENT__KEYBEGIN,
    UIEVENT_KEYDOWN,
    UIEVENT_KEYUP,
    UIEVENT_CHAR,
    UIEVENT_SYSKEY,
    UIEVENT__KEYEND,
    UIEVENT__MOUSEBEGIN,
    UIEVENT_MOUSEMOVE,
    UIEVENT_MOUSELEAVE,
    UIEVENT_MOUSEENTER,
    UIEVENT_MOUSEHOVER,
    UIEVENT_BUTTONDOWN,
    UIEVENT_BUTTONUP,
    UIEVENT_RBUTTONDOWN,
    UIEVENT_DBLCLICK,
    UIEVENT_CONTEXTMENU,
    UIEVENT_SCROLLWHEEL,
    UIEVENT__MOUSEEND,
    UIEVENT_KILLFOCUS,
    UIEVENT_SETFOCUS,
    UIEVENT_WINDOWSIZE,
    UIEVENT_SETCURSOR,
    UIEVENT_TIMER,
    UIEVENT_NOTIFY,
    UIEVENT_COMMAND,
    UIEVENT__LAST,
};

/////////////////////////////////////////////////////////////////////////////////////
//

// Flags for CControlUI::GetControlFlags()
#define UIFLAG_TABSTOP       0x00000001
#define UIFLAG_SETCURSOR     0x00000002
#define UIFLAG_WANTRETURN    0x00000004

// Flags for FindControl()
#define UIFIND_ALL           0x00000000
#define UIFIND_VISIBLE       0x00000001
#define UIFIND_ENABLED       0x00000002
#define UIFIND_HITTEST       0x00000004
#define UIFIND_UPDATETEST    0x00000008
#define UIFIND_TOP_FIRST     0x00000010
#define UIFIND_ME_FIRST      0x80000000

// Flags for the CDialogLayout stretching
#define UISTRETCH_NEWGROUP   0x00000001
#define UISTRETCH_NEWLINE    0x00000002
#define UISTRETCH_MOVE_X     0x00000004
#define UISTRETCH_MOVE_Y     0x00000008
#define UISTRETCH_SIZE_X     0x00000010
#define UISTRETCH_SIZE_Y     0x00000020

// Flags used for controlling the paint
#define UISTATE_FOCUSED      0x00000001
#define UISTATE_SELECTED     0x00000002
#define UISTATE_DISABLED     0x00000004
#define UISTATE_HOT          0x00000008
#define UISTATE_PUSHED       0x00000010
#define UISTATE_READONLY     0x00000020
#define UISTATE_CAPTURED     0x00000040



/////////////////////////////////////////////////////////////////////////////////////
//
typedef struct DUILIB_API tagTResInfo
{
	DWORD m_dwDefaultDisabledColor = 0;
	DWORD m_dwDefaultFontColor = 0;
	DWORD m_dwDefaultLinkFontColor = 0;
	DWORD m_dwDefaultLinkHoverFontColor = 0;
	DWORD m_dwDefaultSelectedBkColor = 0;
	TFontInfo m_DefaultFontInfo;
	std::map<tstring, TFontInfo*>	m_mCustomFonts;
	std::map<tstring, TImageInfo*>	m_mImageHash;
	std::map<int, tstring>			m_mMultiLanguageHash;
} TResInfo;

// Structure for notifications from the system
// to the control implementation.
typedef struct DUILIB_API tagTEventUI
{
    int Type;
    CControlUI* pSender;
    DWORD dwTimestamp;
    POINT ptMouse;
    TCHAR chKey;
    WORD wKeyState;
    WPARAM wParam;
    LPARAM lParam;
} TEventUI;

// Listener interface
class DUILIB_API INotifyUI
{
public:
    virtual void Notify(TNotifyUI& msg) = 0;
};

// MessageFilter interface
class DUILIB_API IMessageFilterUI
{
public:
    virtual LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) = 0;
};

class DUILIB_API ITranslateAccelerator
{
public:
	virtual LRESULT TranslateAccelerator(MSG *pMsg) = 0;
};


/////////////////////////////////////////////////////////////////////////////////////
//
typedef CControlUI* (*LPCREATECONTROL)(LPCTSTR pstrType);


class DUILIB_API CPaintManagerUI : public IDuiDropTarget
{
public:
    CPaintManagerUI();
    ~CPaintManagerUI();

public:
	//init
    void Init(HWND hWnd, LPCTSTR pstrName = NULL);
	bool AttachDialog(CControlUI* pControl);
	bool InitControls(CControlUI* pControl, CControlUI* pParent = NULL);
	void ReapObjects(CControlUI* pControl);

	//attribute
	tstring GetName() const;
	CDuiPoint GetMousePos() const;
	CDuiSize GetClientSize() const;
	CDuiSize GetInitSize();
	void SetInitSize(int cx, int cy);
	CDuiRect GetSizeBox() const;
	void SetSizeBox(CDuiRect& rcSizeBox);
	CDuiRect& GetCaptionRect();
	void SetCaptionRect(CDuiRect& rcCaption);
	CDuiSize GetRoundCorner() const;
	void SetRoundCorner(int cx, int cy);
	CDuiSize GetMinInfo() const;
	void SetMinInfo(int cx, int cy);
	CDuiSize GetMaxInfo() const;
	void SetMaxInfo(int cx, int cy);
	BYTE GetOpacity() const;
	void SetOpacity(BYTE nOpacity);
	bool IsLayered();
	void SetLayered(bool bLayered);
	bool IsNoActivate();
	void SetNoActivate(bool bNoActivate);
	bool GetWindowAttribute(const tstring& strName, tstring& strValue);
	void SetWindowAttribute(const tstring& strName, const tstring& strValue);
	
	//custom attribute
	void AddWindowCustomAttribute(const tstring& strName, const tstring& strAttr);
	tstring GetWindowCustomAttribute(const tstring& strName) const;
	bool RemoveWindowCustomAttribute(const tstring& strName);
	void RemoveAllWindowCustomAttribute();

	//paint
	HDC GetPaintDC() const;
	void Invalidate();
	void Invalidate(CDuiRect& rcItem);

	//drop and drag
	bool SetDropEnable(bool bDrop);
	HRESULT	OnDragEnter(IDataObject *pDataObj, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect);
	HRESULT OnDragOver(DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect);
	HRESULT OnDragLeave();
	HRESULT OnDrop(IDataObject *pDataObj, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect);

	//group
	bool AddOptionGroup(LPCTSTR pStrGroupName, CControlUI* pControl);
	std::vector<CControlUI*>* GetOptionGroup(LPCTSTR pStrGroupName);
	void RemoveAllOptionGroups();

	//focus
    CControlUI* GetFocus() const;
    void SetFocus(CControlUI* pControl, bool bFocusWnd=true);
    void SetFocusNeeded(CControlUI* pControl);
	void KillFocus();
    bool SetNextTabControl(bool bForward = true);
    void SetCapture();
    void ReleaseCapture();
    bool IsCaptured();
	
	//paint
	bool IsPainting();
	void SetPainting(bool bIsPainting);
	
	//notify
    bool AddNotifier(INotifyUI* pControl);
    bool RemoveNotifier(INotifyUI* pControl);   
    void SendNotify(TNotifyUI& Msg, bool bAsync = false, bool bEnableRepeat = true);
    void SendNotify(CControlUI* pControl, LPCTSTR pstrMessage, WPARAM wParam = 0, LPARAM lParam = 0, bool bAsync = false, bool bEnableRepeat = true);
	
	//message filter
    bool AddPreMessageFilter(IMessageFilterUI* pFilter);
    bool RemovePreMessageFilter(IMessageFilterUI* pFilter);
    bool AddMessageFilter(IMessageFilterUI* pFilter);
    bool RemoveMessageFilter(IMessageFilterUI* pFilter);
	bool AddTranslateAccelerator(ITranslateAccelerator *pTranslateAccelerator);
	bool RemoveTranslateAccelerator(ITranslateAccelerator *pTranslateAccelerator);
	
	//control
    CControlUI* GetRoot() const;
    CControlUI* FindControl(POINT pt) const;
    CControlUI* FindControl(LPCTSTR pstrName) const;
    CControlUI* FindSubControlByPoint(CControlUI* pParent, POINT pt) const;
    CControlUI* FindSubControlByName(CControlUI* pParent, LPCTSTR pstrName) const;
    CControlUI* FindSubControlByClass(CControlUI* pParent, LPCTSTR pstrClass, int iIndex = 0);
    std::vector<CControlUI*>& FindSubControlsByClass(CControlUI* pParent, LPCTSTR pstrClass);
	void AddDelayedCleanup(CControlUI* pControl);

	bool TranslateMessage(const LPMSG pMsg);
	bool TranslateAccelerator(LPMSG pMsg);
    bool MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lRes);
	bool PreMessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lRes);

	// static member function
	static void MessageLoop();
	static void Term();
	
	static CPaintManagerUI* GetPaintManager(LPCTSTR pstrName);
	static std::vector<CPaintManagerUI*>& GetPaintManagers();
	static bool LoadPlugin(LPCTSTR pstrModuleName);
	static std::vector<tstring>& GetPlugins();

private:
	std::vector<CControlUI*>& GetFoundControls();
    static CControlUI* CALLBACK __FindControlFromNameHash(CControlUI* pThis, LPVOID pData);
    static CControlUI* CALLBACK __FindControlFromCount(CControlUI* pThis, LPVOID pData);
    static CControlUI* CALLBACK __FindControlFromPoint(CControlUI* pThis, LPVOID pData);
    static CControlUI* CALLBACK __FindControlFromTab(CControlUI* pThis, LPVOID pData);
    static CControlUI* CALLBACK __FindControlFromShortcut(CControlUI* pThis, LPVOID pData);
    static CControlUI* CALLBACK __FindControlFromName(CControlUI* pThis, LPVOID pData);
    static CControlUI* CALLBACK __FindControlFromClass(CControlUI* pThis, LPVOID pData);
    static CControlUI* CALLBACK __FindControlsFromClass(CControlUI* pThis, LPVOID pData);
	static CControlUI* CALLBACK __FindControlsFromUpdate(CControlUI* pThis, LPVOID pData);

private:
	tstring m_sName;
	HWND m_hWndPaint = nullptr;
    HDC m_hDcPaint = nullptr;
    HDC m_hDcOffscreen = nullptr;
    HDC m_hDcBackground = nullptr;
    HBITMAP m_hbmpOffscreen = nullptr;
	COLORREF* m_pOffscreenBits = nullptr;
    HBITMAP m_hbmpBackground;
	COLORREF* m_pBackgroundBits = nullptr;
	int m_iTooltipWidth = 0;
    int m_iLastTooltipWidth = 0;
	HWND m_hwndTooltip = nullptr;
	TOOLINFO m_ToolTip;
	int m_iHoverTime;
    bool m_bNoActivate;
    bool m_bShowUpdateRect;

	//
	CShadowUI m_shadow;
	
    //
    CControlUI* m_pRoot = nullptr;
    CControlUI* m_pFocus = nullptr;
    CControlUI* m_pEventHover = nullptr;
    CControlUI* m_pEventClick = nullptr;
    CControlUI* m_pEventKey = nullptr;
    CControlUI* m_pLastToolTip = nullptr;
	CControlUI* m_pEventDrop = nullptr;

    //
    CDuiPoint m_ptLastMousePos;
    CDuiSize m_szMinWindow;
	CDuiSize m_szMaxWindow;
	CDuiSize m_szInitWindowSize;
	CDuiRect m_rcSizeBox;
	CDuiSize m_szRoundCorner;
	CDuiRect m_rcCaption;
    UINT m_uTimerID;
    bool m_bFirstLayout;
    bool m_bUpdateNeeded;
    bool m_bFocusNeeded;
    bool m_bOffscreenPaint;

	BYTE m_nOpacity;
	bool m_bLayered;
	CDuiRect m_rcLayeredInset;
	bool m_bLayeredChanged;
	CDuiRect m_rcLayeredUpdate;
	TDrawInfo m_diLayered;

    bool m_bMouseTracking;
    bool m_bMouseCapture;
	bool m_bIsPainting;

	//drop and drag
	bool m_bDragMode;
	HBITMAP m_hDragBitmap;
	bool m_bDropEnable;
	CDropTargetEx* m_pDropTarget = nullptr;
	IDataObject*   m_pCurDataObject = nullptr;

    //
	std::vector<INotifyUI*>	m_vNofiers;
	std::vector<IMessageFilterUI*>	m_vPreMessageFilters;
	std::vector<IMessageFilterUI*>	m_vMessageFilters;
	std::vector<ITranslateAccelerator*> m_vTranslateAccelerator;
	std::vector<CControlUI*>  m_vDelayedCleanup;
	std::vector<CControlUI*> m_vFoundControls;
	std::map<tstring, CControlUI*> m_mNameHash;
	std::map<tstring, tstring> m_mWindowAttrHash;
	std::map<tstring, tstring> m_mWindowCustomAttrHash;
	std::map<tstring, std::vector<CControlUI*>> m_mOptionGroup;

    //
	static std::vector<CPaintManagerUI*> m_aPreMessages;
    static std::vector<tstring> m_vPlugins;
};

} // namespace DuiLib_Lite
