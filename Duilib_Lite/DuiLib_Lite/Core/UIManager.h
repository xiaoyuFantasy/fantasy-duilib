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

typedef struct DUILIB_API tagTFontInfo
{
    HFONT hFont;
    tstring sFontName;
    int iSize;
    bool bBold;
    bool bUnderline;
    bool bItalic;
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
    bool bUseHSL;
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
	double left;
	double top;
	double right;
	double bottom;
} TPercentInfo;

typedef struct DUILIB_API tagTResInfo
{
	DWORD m_dwDefaultDisabledColor;
	DWORD m_dwDefaultFontColor;
	DWORD m_dwDefaultLinkFontColor;
	DWORD m_dwDefaultLinkHoverFontColor;
	DWORD m_dwDefaultSelectedBkColor;
	TFontInfo m_DefaultFontInfo;
	std::map<tstring, tstring> m_mCustomFonts;
	std::map<tstring, tstring> m_mImageHash;
	std::map<tstring, tstring> m_mAttrHash;
	std::map<tstring, tstring> m_mMultiLanguageHash;
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
	LPCTSTR GetName() const;
	POINT GetMousePos() const;
	SIZE GetClientSize() const;
	SIZE GetInitSize();
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
	CDuiString GetWindowAttribute(LPCTSTR pstrName);
	void SetWindowAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
	CDuiString GetWindowAttributeList(bool bIgnoreDefault = true);
	void SetWindowAttributeList(LPCTSTR pstrList);
	bool RemoveWindowAttribute(LPCTSTR pstrName);
	
	//custom attribute
	void AddWindowCustomAttribute(LPCTSTR pstrName, LPCTSTR pstrAttr);
	LPCTSTR GetWindowCustomAttribute(LPCTSTR pstrName) const;
	bool RemoveWindowCustomAttribute(LPCTSTR pstrName);
	void RemoveAllWindowCustomAttribute();

	//shared attribute
	//color
	DWORD GetDefaultDisabledColor() const;
	void SetDefaultDisabledColor(DWORD dwColor, bool bShared = false);
	DWORD GetDefaultFontColor() const;
	void SetDefaultFontColor(DWORD dwColor, bool bShared = false);
	DWORD GetDefaultLinkFontColor() const;
	void SetDefaultLinkFontColor(DWORD dwColor, bool bShared = false);
	DWORD GetDefaultLinkHoverFontColor() const;
	void SetDefaultLinkHoverFontColor(DWORD dwColor, bool bShared = false);
	DWORD GetDefaultSelectedBkColor() const;
	void SetDefaultSelectedBkColor(DWORD dwColor, bool bShared = false);
	TFontInfo* GetDefaultFontInfo();
	//font
	void SetDefaultFont(LPCTSTR pStrFontName, int nSize, bool bBold, bool bUnderline, bool bItalic, bool bShared = false);
	DWORD GetCustomFontCount(bool bShared = false) const;
	HFONT AddFont(int id, LPCTSTR pStrFontName, int nSize, bool bBold, bool bUnderline, bool bItalic, bool bShared = false);
	HFONT GetFont(int id);
	void RemoveFont(HFONT hFont, bool bShared = false);
	void RemoveFont(int id, bool bShared = false);
	void RemoveAllFonts(bool bShared = false);
	TFontInfo* GetFontInfo(int id);
	TFontInfo* GetFontInfo(HFONT hFont);
	//image
	const TImageInfo* GetImage(LPCTSTR bitmap);
	const TImageInfo* AddImage(LPCTSTR bitmap, HBITMAP hBitmap, int iWidth, int iHeight, bool bAlpha, bool bShared = false);
	void RemoveImage(LPCTSTR bitmap, bool bShared = false);
	void RemoveAllImages(bool bShared = false);
	//default
	void AddDefaultAttributeList(LPCTSTR pStrControlName, LPCTSTR pStrControlAttrList, bool bShared = false);
	LPCTSTR GetDefaultAttributeList(LPCTSTR pStrControlName) const;
	bool RemoveDefaultAttributeList(LPCTSTR pStrControlName, bool bShared = false);
	void RemoveAllDefaultAttributeList(bool bShared = false);

	//paint
	HDC GetPaintDC() const;
	void Invalidate();
	void Invalidate(const CDuiRect& rcItem);

	//drop and drag
	bool SetDropEnable(bool bDrop);
	HRESULT	OnDragEnter(IDataObject *pDataObj, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect);
	HRESULT OnDragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
	HRESULT OnDragLeave();
	HRESULT OnDrop(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);

	//
	bool AddOptionGroup(LPCTSTR pStrGroupName, CControlUI* pControl);
	std::vector<CControlUI*>& GetOptionGroup(LPCTSTR pStrGroupName);
	void RemoveOptionGroup(LPCTSTR pStrGroupName, CControlUI* pControl);
	void RemoveAllOptionGroups();
    CControlUI* GetFocus() const;
    void SetFocus(CControlUI* pControl, bool bFocusWnd=true);
    void SetFocusNeeded(CControlUI* pControl);
    bool SetNextTabControl(bool bForward = true);
    void SetCapture();
    void ReleaseCapture();
    bool IsCaptured();

	bool IsPainting();
	void SetPainting(bool bIsPainting);

    bool AddNotifier(INotifyUI* pControl);
    bool RemoveNotifier(INotifyUI* pControl);   
    void SendNotify(TNotifyUI& Msg, bool bAsync = false, bool bEnableRepeat = true);
    void SendNotify(CControlUI* pControl, LPCTSTR pstrMessage, WPARAM wParam = 0, LPARAM lParam = 0, bool bAsync = false, bool bEnableRepeat = true);

    bool AddPreMessageFilter(IMessageFilterUI* pFilter);
    bool RemovePreMessageFilter(IMessageFilterUI* pFilter);
    bool AddMessageFilter(IMessageFilterUI* pFilter);
    bool RemoveMessageFilter(IMessageFilterUI* pFilter);

    void AddDelayedCleanup(CControlUI* pControl);
    void AddMouseLeaveNeeded(CControlUI* pControl);
    bool RemoveMouseLeaveNeeded(CControlUI* pControl);

	bool AddTranslateAccelerator(ITranslateAccelerator *pTranslateAccelerator);
	bool RemoveTranslateAccelerator(ITranslateAccelerator *pTranslateAccelerator);
	
	//
	CShadowUI* GetShadow();

    CControlUI* GetRoot() const;
    CControlUI* FindControl(POINT pt) const;
    CControlUI* FindControl(LPCTSTR pstrName) const;
    CControlUI* FindSubControlByPoint(CControlUI* pParent, POINT pt) const;
    CControlUI* FindSubControlByName(CControlUI* pParent, LPCTSTR pstrName) const;
    CControlUI* FindSubControlByClass(CControlUI* pParent, LPCTSTR pstrClass, int iIndex = 0);
    std::vector<CControlUI*>& FindSubControlsByClass(CControlUI* pParent, LPCTSTR pstrClass);

	bool TranslateAccelerator(LPMSG pMsg);
    bool MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lRes);
	bool PreMessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lRes);

	// static member function
	static void MessageLoop();
	static bool TranslateMessage(const LPMSG pMsg);
	static void Term();
	static void AddMultiLanguageString(int id, LPCTSTR pStrMultiLanguage);
	static LPCTSTR GetMultiLanguageString(int id);
	static bool RemoveMultiLanguageString(int id);
	static void RemoveAllMultiLanguageString();
	static void ProcessMultiLanguageTokens(CDuiString& pStrMultiLanguage);
	static HINSTANCE GetInstance();
	static CDuiString GetInstancePath();
	static CDuiString GetCurrentPath();
	static HINSTANCE GetResourceDll();
	static const CDuiString& GetResourcePath();
	static const CDuiString& GetResourceZip();
	static bool IsCachedResourceZip();
	static HANDLE GetResourceZipHandle();
	static void SetInstance(HINSTANCE hInst);
	static void SetCurrentPath(LPCTSTR pStrPath);
	static void SetResourceDll(HINSTANCE hInst);
	static void SetResourcePath(LPCTSTR pStrPath);
	static void SetResourceZip(LPVOID pVoid, unsigned int len);
	static void SetResourceZip(LPCTSTR pstrZip, bool bCachedResourceZip = false);
	static void ReloadSkin();
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
    HDC m_hDcPaint;
    HDC m_hDcOffscreen;
    HDC m_hDcBackground;
    HBITMAP m_hbmpOffscreen;
	COLORREF* m_pOffscreenBits;
    HBITMAP m_hbmpBackground;
	COLORREF* m_pBackgroundBits;
	int m_iTooltipWidth;
    int m_iLastTooltipWidth;
	HWND m_hwndTooltip;
	TOOLINFO m_ToolTip;
	int m_iHoverTime;
    bool m_bNoActivate;
    bool m_bShowUpdateRect;

	//
	CShadowUI m_shadow;
	
    //
    CControlUI* m_pRoot;
    CControlUI* m_pFocus;
    CControlUI* m_pEventHover;
    CControlUI* m_pEventClick;
    CControlUI* m_pEventKey;
    CControlUI* m_pLastToolTip;
	CControlUI* m_pEventDrop;

    //
    POINT m_ptLastMousePos;
    SIZE m_szMinWindow;
    SIZE m_szMaxWindow;
    SIZE m_szInitWindowSize;
    RECT m_rcSizeBox;
    SIZE m_szRoundCorner;
    RECT m_rcCaption;
    UINT m_uTimerID;
    bool m_bFirstLayout;
    bool m_bUpdateNeeded;
    bool m_bFocusNeeded;
    bool m_bOffscreenPaint;

	BYTE m_nOpacity;
	bool m_bLayered;
	RECT m_rcLayeredInset;
	bool m_bLayeredChanged;
	RECT m_rcLayeredUpdate;
	TDrawInfo m_diLayered;

    bool m_bMouseTracking;
    bool m_bMouseCapture;
	bool m_bIsPainting;
	bool m_bUsedVirtualWnd;
	bool m_bAsyncNotifyPosted;

	//drop and drag
	bool m_bDragMode;
	HBITMAP m_hDragBitmap;
	bool m_bDropEnable;
	CDropTargetEx* m_pDropTarget;
	IDataObject*   m_pCurDataObject;

    //
	std::vector<INotifyUI*>	m_vNofiers;
	std::vector<IMessageFilterUI*>	m_vPreMessageFilters;
	std::vector<IMessageFilterUI*>	m_vMessageFilters;
	std::vector<ITranslateAccelerator*> m_vTranslateAccelerator;
	std::vector<CControlUI*>  m_vDelayedCleanup;
	std::vector<CControlUI*> m_vFoundControls;
	std::map<tstring, CControlUI*> m_mNameHash;
	std::map<tstring, tstring> m_mWindowAttrHash;
	std::map <tstring, std::vector<CControlUI*>> m_mOptionGroup;

    //
	bool m_bForceUseSharedRes;
	TResInfo m_ResInfo;

    //
	static HINSTANCE m_hResourceInstance;
	static tstring m_strResourcePath;
	static tstring m_strResourceZip;
	static HANDLE m_hResourceZip;
	static TResInfo m_SharedResInfo;
    static HINSTANCE m_hInstance;
    static std::vector<tstring> m_vPlugins;
};

} // namespace DuiLib_Lite
