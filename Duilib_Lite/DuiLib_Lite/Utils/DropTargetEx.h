#pragma once
#include <OleIdl.h>
#include <Shobjidl.h>
#include <Shlwapi.h>
#include <Ole2.h>

#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Ole32.lib")

namespace DuiLib_Lite
{
	class DUILIB_API IDuiDropTarget
	{
	public:
		virtual HRESULT	OnDragEnter(IDataObject *pDataObj, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect) = 0;
		virtual HRESULT OnDragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect) = 0;
		virtual HRESULT OnDragLeave() = 0;
		virtual HRESULT OnDrop(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect) = 0;
	};

	class CDropTargetEx : public IDropTarget
	{
	public:
		CDropTargetEx();
		virtual ~CDropTargetEx();

		HRESULT DragDropRegister(
			_In_ HWND         hwnd,
			_In_ IDuiDropTarget* pDropTarget
		);
		HRESULT DragDropRevoke(
			_In_ HWND hwnd
		);

		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, __RPC__deref_out void **ppvObject);
		ULONG STDMETHODCALLTYPE AddRef();
		ULONG STDMETHODCALLTYPE Release();

		//����  
		HRESULT STDMETHODCALLTYPE DragEnter(__RPC__in_opt IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, __RPC__inout DWORD *pdwEffect);
		//�ƶ�  
		HRESULT STDMETHODCALLTYPE DragOver(DWORD grfKeyState, POINTL pt, __RPC__inout DWORD *pdwEffect);
		//�뿪  
		HRESULT STDMETHODCALLTYPE DragLeave();
		//�ͷ�  
		HRESULT STDMETHODCALLTYPE Drop(__RPC__in_opt IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, __RPC__inout DWORD *pdwEffect);

	private:
		HWND m_hWnd;
		IDropTargetHelper* m_piDropHelper;
		bool    m_bUseDnDHelper;
		IDuiDropTarget* m_pDuiDropTarget;
		DWORD m_dAcceptKeyState;
		ULONG  m_lRefCount;
	};
}