#include "stdafx.h"
#include "DropTargetEx.h"
#include <ShlGuid.h>

namespace DuiLib
{

	CDropTargetEx::CDropTargetEx()
	{
		m_lRefCount = 1;
		if (SUCCEEDED(CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER, IID_IDropTargetHelper, (void**)&m_piDropHelper)))
		{
			m_bUseDnDHelper = true;
		}
	}

	CDropTargetEx::~CDropTargetEx()
	{
		if (m_piDropHelper)
		{
			m_piDropHelper->Release();
		}

		m_bUseDnDHelper = false;
		m_lRefCount = 0;
	}

	HRESULT CDropTargetEx::DragDropRegister(HWND hwnd, IDuiDropTarget * pDropTarget)
	{
		if (!::IsWindow(hwnd))
			return S_FALSE;

		if (!m_pDuiDropTarget)
			return S_FALSE;

		if (FAILED(::RegisterDragDrop(hwnd, this)))
			return S_FALSE;

		m_hWnd = hwnd;
		m_pDuiDropTarget = pDropTarget;
		return S_OK;
	}

	HRESULT CDropTargetEx::DragDropRevoke(HWND hwnd)
	{
		if (!::IsWindow(hwnd))
			return S_FALSE;

		return ::RevokeDragDrop(hwnd);
	}

	HRESULT CDropTargetEx::QueryInterface(REFIID riid, void ** ppvObject)
	{
		static QITAB rgqit[] =
		{
			QITABENT(CDropTargetEx, IDropTarget),
			{ 0 }
		};
		return QISearch(this, rgqit, riid, ppvObject);
	}

	ULONG CDropTargetEx::AddRef()
	{
		return InterlockedIncrement(&m_lRefCount);
	}

	ULONG CDropTargetEx::Release()
	{
		ULONG lRef = InterlockedDecrement(&m_lRefCount);
		if (0 == lRef)
		{
			delete this;
			return 0;
		}
		return m_lRefCount;
	}

	HRESULT CDropTargetEx::DragEnter(IDataObject * pDataObj, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect)
	{
		if (m_bUseDnDHelper)
		{
			m_piDropHelper->DragEnter(m_hWnd, pDataObj, (POINT*)&pt, *pdwEffect);
		}
		return m_pDuiDropTarget->OnDragEnter(pDataObj, grfKeyState, pt, pdwEffect);
	}

	HRESULT CDropTargetEx::DragOver(DWORD grfKeyState, POINTL pt, DWORD * pdwEffect)
	{
		if (m_bUseDnDHelper)
		{
			m_piDropHelper->DragOver((LPPOINT)&pt, *pdwEffect);
		}
		return m_pDuiDropTarget->OnDragOver(grfKeyState, pt, pdwEffect);
	}

	HRESULT CDropTargetEx::DragLeave()
	{
		if (m_bUseDnDHelper)
		{
			m_piDropHelper->DragLeave();
		}
		return m_pDuiDropTarget->OnDragLeave();
	}

	HRESULT CDropTargetEx::Drop(IDataObject * pDataObj, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect)
	{
		m_piDropHelper->Drop(pDataObj, (LPPOINT)&pt, *pdwEffect);
		return m_pDuiDropTarget->OnDrop(pDataObj, grfKeyState, pt, pdwEffect);
	}
}