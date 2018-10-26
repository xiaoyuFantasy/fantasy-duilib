#pragma once

namespace DuilibLite
{
	class DUILIB_API CDuiPoint : public tagPOINT
	{
	public:
		CDuiPoint();
		CDuiPoint(const POINT& pt);
		CDuiPoint(long _x, long _y);
		CDuiPoint(const tstring	&strValue);
		tstring ToString();
	};

	class DUILIB_API CDuiSize : public tagSIZE
	{
		CDuiSize();
		CDuiSize(const SIZE &size);
		CDuiSize(long _cx, long _cy);
		CDuiSize(const tstring &strValue);
		tstring ToString();
	};

	class DUILIB_API CDuiRect : public tagRECT
	{
	public:
		CDuiRect();
		CDuiRect(const RECT& rc);
		CDuiRect(long lLeft, long lTop, long lRight, long lBottom);
		CDuiRect(const tstring &strValue);
		tstring ToString();

		int Width();
		int Height();
		bool IsEmpty();
		void Join(const RECT &rc);
		void Offset(int cx, int cy);
		void Inflate(int cx, int cy);
		void Deflate(int cx, int cy);
		void Union(CDuiRect& rc);
	};
}