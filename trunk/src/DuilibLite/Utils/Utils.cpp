#include "stdafx.h"
#include "Utils.h"

namespace DuilibLite
{
	//CDuiPoint
	CDuiPoint::CDuiPoint()
	{
		x = y = 0;
	}
	CDuiPoint::CDuiPoint(const POINT & pt)
	{
		x = pt.x;
		y = pt.y;
	}
	CDuiPoint::CDuiPoint(long _x, long _y)
	{
		x = _x;
		y = _y;
	}
	CDuiPoint::CDuiPoint(const tstring & strValue)
	{
		size_t nPos = strValue.find(L",");
		if (nPos == std::string::npos)
			x = y = 0;
		else
		{
			x = stoi(strValue.substr(0, nPos));
			y = stoi(strValue.substr(nPos + 1, strValue.length()));
		}
		
	}
	tstring CDuiPoint::ToString()
	{
		tstring strPoint = to_tstring(x) + L"," + to_tstring(y);
		return strPoint;
	}

	//CDuiSize
	CDuiSize::CDuiSize()
	{
		cx = cy = 0;
	}
	CDuiSize::CDuiSize(const SIZE & size)
	{
		cx = size.cx;
		cy = size.cy;
	}
	CDuiSize::CDuiSize(long _cx, long _cy)
	{
		cx = _cx;
		cy = _cy;
	}
	CDuiSize::CDuiSize(const tstring & strValue)
	{
		size_t nPos = strValue.find(L",");
		if (nPos == std::string::npos)
			cx = cy = 0;
		else
		{
			cx = stoi(strValue.substr(0, nPos));
			cy = stoi(strValue.substr(nPos + 1, strValue.length()));
		}
	}
	tstring CDuiSize::ToString()
	{
		tstring strPoint = to_tstring(cx) + L"," + to_tstring(cy);
		return strPoint;
	}

	//CDuiRect
	CDuiRect::CDuiRect()
	{
		left = top = right = bottom = 0;
	}
	CDuiRect::CDuiRect(const RECT & rc)
	{
		left = rc.left;
		top = rc.top;
		right = rc.right;
		bottom = rc.bottom;
	}
	CDuiRect::CDuiRect(long lLeft, long lTop, long lRight, long lBottom)
	{
		left = lLeft;
		top = lTop;
		right = lRight;
		bottom = lBottom;
	}
	CDuiRect::CDuiRect(const tstring & strValue)
	{

	}
	tstring CDuiRect::ToString()
	{
		return tstring();
	}
	int CDuiRect::Width()
	{
		return 0;
	}
	int CDuiRect::Height()
	{
		return 0;
	}
	bool CDuiRect::IsEmpty()
	{
		return false;
	}
	void CDuiRect::Join(const RECT & rc)
	{
	}
	void CDuiRect::Offset(int cx, int cy)
	{
	}
	void CDuiRect::Inflate(int cx, int cy)
	{
	}
	void CDuiRect::Deflate(int cx, int cy)
	{
	}
	void CDuiRect::Union(CDuiRect & rc)
	{
	}
}