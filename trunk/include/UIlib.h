#pragma once

#ifdef UILIB_STATIC
#	define DUILIB_API 
#else
#	if defined(UILIB_EXPORTS)
#		if	defined(_MSC_VER)
#			define DUILIB_API __declspec(dllexport)
#		else
#			define DUILIB_API 
#		endif
#	else
#		if defined(_MSC_VER)
#			define DUILIB_API __declspec(dllimport)
#		else
#			define DUILIB_API 
#		endif
#	endif
#endif

#include <mutex>
#include <list>
#include <vector>
#include <map>
#include <queue>
#include <string>
#include <memory>
#include <algorithm>
#include <functional>
using namespace std;

#ifdef UNICODE
typedef std::wstring tstring;
#define to_tstring std::to_wstring
#else
typedef std::string	 tstring;
#define to_tstring std::to_string
#endif // UNICODE

#include <Windows.h>
#include <WinDef.h>

#include "Core\UIManager.h"

#include "Utils\Utils.h"