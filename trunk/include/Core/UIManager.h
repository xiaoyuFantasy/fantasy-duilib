#pragma once

namespace DuilibLite
{
	class CControlUI;

	class DUILIB_API CPaintManagerUI
	{
	public:
		CPaintManagerUI();
		~CPaintManagerUI();

	public:
		//init
		void Init(HWND hWnd, const tstring& strName);
		bool InitControls(CControlUI* pCtrl, CControlUI* pParentCtrl = nullptr);
		bool AttachDialog(CControlUI* pCtrl);

		//attribute
		tstring GetName() const;

	};
}