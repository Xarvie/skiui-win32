#define _CRT_SECURE_NO_WARNINGS

#if(_MSC_VER >=1200)
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif
#include "skiui-lite.h"
#include <algorithm>
#include <typeinfo>
#include <Commctrl.h>
#include <assert.h>
#include <GdiPlus.h>
#include <fstream>
#include <time.h>
//#pragma comment(lib,"gdiplus.lib")
#pragma comment( lib, "comctl32.lib" )
extern WPARAM WindowProc::msgLoop(HWND hwnd, HACCEL hotKeyHandle, bool ocxHotKey, FUNCILPMSG callback);

#ifdef _UNICODE
tstring _TS(const std::wstring& tmp)
{
	return std::wstring(tmp);
}
tstring _TS(const std::string& tmp)
{
	return enc::a2w(tmp);
}
tstring _TS(const wchar_t* tmp)
{
	return std::wstring(tmp);
}
tstring _TS(const char* tmp)
{
	return enc::a2w(std::string(tmp));
}
#else

tstring _TS(const std::wstring& tmp)
{
	return enc::w2a(tmp);
}
tstring _TS(const std::string& tmp)
{
	return std::string(tmp);
}
tstring _TS(const wchar_t* tmp)
{
	return enc::w2a(std::wstring(tmp));
}
tstring _TS(const char* tmp)
{
	return std::string(tmp);
}
#endif
//编码类 encode
namespace enc
{
	///<summary>WideChar转ASCII。</summary><param name = "WideChar">WideChar文本。</param><returns>成功:返回文本，失败:string(“”)。</returns>
	std::string w2a(const std::wstring& wideString)
	{
		int iSize;
		std::string rt;
		iSize = WideCharToMultiByte(CP_ACP, 0, wideString.c_str(), -1, NULL, 0,
			NULL, NULL);
		char *tmp = new char[iSize]; //短字符串可优化
		rt.resize(iSize, 0);
		if (!WideCharToMultiByte(CP_ACP, 0, wideString.c_str(), -1, tmp, iSize,
			NULL, NULL))
		{
			return std::string("");
		}
		std::string ret = tmp;
		delete[] tmp;
		return std::string(ret);
	}
	///<summary>ASCII转WideChar。</summary><param name = "WideChar">ASCII文本。</param><returns>成功:返回文本，失败:wstring(“”)。</returns>
	std::wstring a2w(const std::string& a)
	{
		int iSize;
		iSize = MultiByteToWideChar(CP_ACP, 0, a.c_str(), -1, NULL, 0);
		wchar_t *tmp = new wchar_t[iSize]; //短字符串可优化
		if (!MultiByteToWideChar(CP_ACP, 0, a.c_str(), -1, tmp, iSize))
		{
			return std::wstring(L"");
		}
		std::wstring ret = tmp;
		delete[] tmp;
		return std::wstring(ret);
	}
	///<summary>WideChar转ASCII。</summary><param name = "WideChar">WideChar文本。</param><returns>成功:返回文本，失败:string(“”)。</returns>
	std::string t2a(const tstring& wideString)
	{
#ifdef _UNICODE
		return w2a(wideString);
#else
		return wideString;
#endif
	}
	///<summary>ASCII转WideChar。</summary><param name = "WideChar">ASCII文本。</param><returns>成功:返回文本，失败:wstring(“”)。</returns>
	std::wstring t2w(const tstring& a)
	{
#ifdef _UNICODE
		return a;
#else
		return a2w(a);
#endif
	}
}

namespace tim
{
	///<summary>处理事件。</summary>
	void __doEvents()
	{
		MSG msg;
		while (PeekMessage(&msg, (HWND)NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				return;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	///<summary>高精度等待。</summary><param name = "milliSecond">等待的毫秒数。</param><returns>成功:0，失败:-1。</returns>
	int wait(UINT64 milliSecond)
	{
		HANDLE hTimer;
		LARGE_INTEGER liDueTime;
		liDueTime.QuadPart = -10 * milliSecond * 1000LL;
		__doEvents();
		hTimer = CreateWaitableTimer(nullptr, FALSE, nullptr);
		if (NULL == hTimer)
		{
			return -1;
		}
		if (!SetWaitableTimer(hTimer, &liDueTime, 0, NULL, NULL, 0))
		{
			return -1;
		}
		DWORD ret;
		while (ret = MsgWaitForMultipleObjects(1, &hTimer, FALSE, INFINITE,
			QS_ALLINPUT) != WAIT_OBJECT_0)
		{
			if (((WAIT_OBJECT_0 + 1) == ret) || (WAIT_TIMEOUT == ret))
			{
				__doEvents();
			}
			else
			{
				break;
			}
		}
		CancelWaitableTimer(hTimer);
		CloseHandle(hTimer);
		__doEvents();
		return 0;
	}
	///<summary>创建时钟。</summary><param name = "func">响应函数。</param><param name = "timerCycle">默认1000ms做一次函数响应。</param><returns>成功:0，失败:-1。</returns>
	int timer::CreateTimer(WAITORTIMERCALLBACK func, int timerCycle, void * param)
	{
		if (hTimerQueue != 0 || hTimer != 0)
		{
			deleteTimer();
		}
		this->func = func;
		this->hTimerQueue = CreateTimerQueue();
		return (CreateTimerQueueTimer(&this->hTimer, hTimerQueue, func, param, 0,
			timerCycle, WT_EXECUTEDEFAULT));
	}
	///<summary>删除时钟。</summary></param><returns>成功:非0，失败:0。</returns>
	int timer::deleteTimer()
	{
		if (hTimerQueue == 0 || hTimer == 0)
		{
			return 1;
		}
		BOOL bol = DeleteTimerQueueTimer(hTimerQueue, hTimer, 0);
		DeleteTimerQueue(hTimerQueue);
		hTimerQueue = 0;
		hTimer = 0;
		timerCycle = 0;
		return bol;
	}
}

namespace global
{
	HINSTANCE instanceHandle = 0;
	LONG_PTR widgetId = 0;
	std::map<HWND, Base*> objMap;
	std::map<HWND, WNDPROC> msgProcMap;
	std::map<HWND, WNDPROC> widgetProcMap;
	HWND eventHandle = NULL;
	bool waitMouse = false;
	std::map<UINT_PTR, HWND> toolTip;
	ULONG_PTR m_gdiplusToken;
	int init(int *, char ***)
	{
		FARPROC spdpia = GetProcAddress(GetModuleHandle(TEXT("user32")), "SetProcessDPIAware");
		if (spdpia != NULL) spdpia();
		Gdiplus::GdiplusStartupInput StartupInput;
		GdiplusStartup(&m_gdiplusToken, &StartupInput, NULL);

		return 0;
	}
	int loop()
	{
		return (int)WindowProc::msgLoop();
	}
}

namespace other
{
	int getControlKeyState()
	{
		int state = 0;
		if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0)
			state = state | 1;
		if ((GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0)
			state = state | 2;
		if ((GetAsyncKeyState(VK_MENU) & 0x8000) != 0)
			state = state | 4;
		return state;
	}
}
//////////////////
WPARAM WindowProc::msgLoop(HWND hwnd, HACCEL hotKeyHandle, bool ocxHotKey, FUNCILPMSG callback)
{
	MSG msg;
	int ret = 0;
	HWND hWndCtl = NULL;

	static long long i = 0;


	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (callback)
		{
			if (0 == (ret = callback(&msg)))
				return ret;
		}

		if (ocxHotKey)
		{
			if ((msg.message >= 256 && msg.message <= 264) || msg.message == 512)
			{
				hWndCtl = GetFocus();
				if (IsChild(hwnd, hWndCtl))
				{
					while (GetParent(hWndCtl) != hwnd)
					{
						hWndCtl = GetParent(hWndCtl);
					}
					SendMessage(hWndCtl, 0x037F/*WM_FORWARDMSG*/, 0, (LPARAM)&msg);
				}
			}
		}

		if (!TranslateAccelerator(hWndCtl, hotKeyHandle, &msg))
		{
			if (IsDialogMessage(hwnd, &msg) == 0)
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		if (hwnd != NULL && IsWindow(hwnd) == false)
			break;
	}
	return msg.wParam;
}

////////////////////////////////
Base::Base()
{
}

Base::~Base()
{
	this->baseReset(this->_hwnd);
}

LRESULT Base::_windowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	return (DefWindowProc(hwnd, msg, wparam, lparam));
}

bool Base::reDraw(bool erase)
{
	return InvalidateRect(this->_hwnd, NULL, erase);
}

LRESULT Base::sendMsg(UINT msg, WPARAM wparam, LPARAM lparam)
{
	return SendMessage(this->_hwnd, msg, wparam, lparam);
}

LRESULT Base::postMsg(UINT msg, WPARAM wparam, LPARAM lparam)
{
	return PostMessage(this->_hwnd, msg, wparam, lparam);
}

HWND Base::_create(HWND parentHwnd, DWORD exStyle, const tstring & className,
	const tstring & title, DWORD style, int x, int y, int w, int d,
	LONG_PTR id, LPVOID lparam)
{
	if (this->_hwnd)
		return NULL;
	this->_hwnd = CreateWindowEx(exStyle, className.c_str(), title.c_str(),
		style | WS_CLIPSIBLINGS, x, y, w, d, parentHwnd, (HMENU)id,
		global::instanceHandle, lparam);
	if (this->_hwnd == NULL)
		return NULL;
	bindObjAndHwnd(this, this->_hwnd);
	SendMessage(this->_hwnd, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);
	//SetWindowPos(this->_hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
	return this->_hwnd;
}

LRESULT(Base::bindObjAndHwnd) (Base* objPtr, HWND hwnd)
{
	global::objMap[hwnd] = objPtr;
	SetSubClassMsg(objPtr, hwnd);
	return true;
}

void Base::SetSubClassMsg(Base* objPtr, HWND hwnd)
{
	LONG_PTR oldWndProc;
	if (GetWindowLongPtr(hwnd, GWLP_WNDPROC) != (LONG_PTR)&subClassMsg)
	{
		oldWndProc = SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)&subClassMsg);
		objPtr->_hwnd = hwnd;
		objPtr->_wndProcOld = (WNDPROC)oldWndProc;
	}
}

LRESULT Base::subClassMsg(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	LRESULT ret = 0;
	WNDPROC wndProc = NULL;
	NMHDR nmhdr;
	if (msg != WM_QUIT)
	{
		if (NULL != (wndProc = global::msgProcMap[hwnd]))
			if (0 != (ret = wndProc(hwnd, msg, wparam, lparam)))
				return ret;
		if (msg == WM_COMMAND)
		{
			if (NULL != (wndProc = global::widgetProcMap[hwnd]))
				if (0 != (ret = wndProc((HWND)lparam, (UINT)HIWORD(wparam), LOWORD(wparam), lparam)))
					return ret;
		}

		if (msg == WM_NOTIFY)
		{
			memcpy(&nmhdr, &lparam, 12);
			if (NULL != (wndProc = global::widgetProcMap[nmhdr.hwndFrom]))
				if (0 != (ret = wndProc(nmhdr.hwndFrom, nmhdr.code, nmhdr.idFrom, lparam)))
					return ret;
		}
	}
	Base* objPtr = global::objMap[hwnd];
	if (objPtr == 0)
		return (DefWindowProc(hwnd, msg, wparam, lparam));
	return (objPtr->subClassProc(hwnd, msg, wparam, lparam));
}

LRESULT Base::subClassProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	Base* objPtr = NULL;
	int keyState = 0;
	int hi = 0;
	int lo = 0;
	NMHDR nmhdr = { 0 };
	DRAWITEMSTRUCT drawItem;
	tstring fileName;
	tstring dropFileName;
	PAINTSTRUCT PS;
	HDC hdc = NULL;
	LRESULT ret = 0;
	global::eventHandle = hwnd;

	if (msg == WM_NCDESTROY)
	{
		this->_baseReset(hwnd);
	}

	else if (msg == WM_LBUTTONDOWN)
	{
		_OnLbuttonDown(hwnd, lparam);
		if (_evMouseLeftDown != 0)
		{
			keyState = other::getControlKeyState();
			ret = _evMouseLeftDown(LOWORD(lparam), HIWORD(lparam), keyState);
			if (ret != 0)
				return ret;
		}
	}

	else if (msg == WM_LBUTTONUP)
	{
		_OnLbuttonUp(hwnd, lparam);
		if (_evMouseLeftUp != 0)
		{
			keyState = other::getControlKeyState();
			ret = _evMouseLeftUp(LOWORD(lparam), HIWORD(lparam), keyState);
			if (ret != 0)
				return ret;
		}
	}

	else if (msg == WM_LBUTTONDBLCLK)
	{
		_OnLbuttonDblclk(hwnd, lparam);
		if (_evMouseDoubleClick != 0)
		{
			keyState = other::getControlKeyState();
			ret = _evMouseDoubleClick(LOWORD(lparam), HIWORD(lparam), keyState);
			if (ret != 0)
				return ret;
		}
	}

	else if (msg == WM_RBUTTONDOWN)
	{
		if (_evMouseRightDown != 0)
		{
			keyState = other::getControlKeyState();
			ret = _evMouseRightDown(LOWORD(lparam), HIWORD(lparam), keyState);
			if (ret != 0)
				return ret;
		}
	}

	else if (msg == WM_RBUTTONUP)
	{
		if (_evMouseRightUp != 0)
		{
			keyState = other::getControlKeyState();
			ret = _evMouseRightUp(LOWORD(lparam), HIWORD(lparam), keyState);
			if (ret != 0)
				return ret;
		}
	}

	else if (msg == WM_MOUSEMOVE)
	{
		lo = LOWORD(lparam);  // x
		hi = HIWORD(lparam);  // y
		if (_OnMouseMove(hwnd, lo, hi) != 0)
			return ret;

		if (_evMouseMove != 0)
		{
			keyState = other::getControlKeyState();
			ret = _evMouseMove(lo, hi, keyState);
			if (ret != 0)
				return ret;
		}
	}

	else if (msg == WM_SETFOCUS)
	{
		if (_evGetFocus != 0)
			_evGetFocus();
	}

	else if (msg == WM_KILLFOCUS)
	{
		if (_evLoseFocus != 0)
			_evLoseFocus();
	}

	else if (msg == WM_KEYDOWN)
	{
		_OnKeyDown(hwnd, wparam);
		if (_evkeyDown != 0)
		{
			keyState = other::getControlKeyState();
			ret = _evkeyDown(wparam, keyState);
			if (ret != 0)
				return ret;
		}
	}

	else if (msg == WM_KEYUP)
	{
		if (_evkeyUp != 0)
		{
			keyState = other::getControlKeyState();
			ret = _evkeyUp(wparam, keyState);
			if (ret != 0)
				return ret;
		}
	}

	else if (msg == WM_MOUSEWHEEL)
	{
		if (_evMouseWheel != 0)
		{
			keyState = other::getControlKeyState();
			ret = _evMouseWheel(HIWORD(wparam), keyState);
			if (ret != 0)
				return ret;
		}
	}

	else if (msg == WM_CHAR)
	{
		if (_evChar != 0)
		{
			ret = _evChar(wparam);
			if (ret != 0)
				return ret;
		}
	}

	else if (msg == WM_SETCURSOR)
	{
		ret = _OnSetCursor(hwnd, msg, wparam, lparam);
		if (ret != 0)
			return ret;
	}

	else if (msg == WM_PAINT)
	{
		if (_evPaint != 0)
		{
			hdc = BeginPaint(hwnd, &PS);
			ret = _evPaint((LONG_PTR)hdc, PS.rcPaint.left, PS.rcPaint.top, PS.rcPaint.right, PS.rcPaint.bottom);
			EndPaint(hwnd, &PS);
			if (ret != 0)
				return ret;
		}
		ret = _OnPaint(hwnd);
		if (ret != 0)
			return ret;
	}

	else if (msg == WM_ERASEBKGND)
	{
		ret = _OnErasebkgnd(hwnd, (HDC)wparam);
		if (ret != 0)
			return ret;
	}

	else if (msg == WM_HSCROLL || msg == WM_VSCROLL)
	{
		global::eventHandle = (HWND)lparam;
		objPtr = global::objMap[(HWND)lparam];
		if (objPtr != 0)
		{
			ret = objPtr->_OnScroll(LOWORD(wparam), HIWORD(wparam), lparam);
			if (ret != 0)
				return ret;
		}
	}

	else if (msg == WM_COMMAND)
	{
		global::eventHandle = (HWND)lparam;
		objPtr = global::objMap[(HWND)lparam];
		if (objPtr != 0)
		{
			lo = LOWORD(wparam);
			hi = HIWORD(wparam);
			ret = objPtr->_OnCommand(hwnd, lparam, hi, lo);
			if (ret != 0)
				return ret;
		}
	}

	else if (msg == WM_NOTIFY)
	{
		memcpy(&nmhdr, (void*)lparam, sizeof(nmhdr));
		global::eventHandle = nmhdr.hwndFrom;
		if (nmhdr.code == (UINT)-520/*TTN_NEEDTEXT*/)
		{
			if (nmhdr.idFrom != 0)
				ret = (long long)global::toolTip[nmhdr.idFrom];
			if (ret != 0)
				objPtr = global::objMap[(HWND)ret];
		}
		else
		{
			objPtr = global::objMap[nmhdr.hwndFrom];
		}

		if (objPtr != NULL)
		{
			ret = objPtr->_OnNotify(hwnd, nmhdr.hwndFrom, nmhdr.code, nmhdr.idFrom, lparam);
			if (ret != 0)
				return ret;
		}
	}

	else if (msg >= WM_CTLCOLORMSGBOX && msg <= WM_CTLCOLORSTATIC)
	{
		global::eventHandle = (HWND)lparam;
		objPtr = global::objMap[(HWND)lparam];
		//if (global::noErase)
		//	return 1;
		if (objPtr != 0)
		{
			ret = objPtr->_OnCtlColor((HWND)lparam, (HDC)wparam);
		}

		if (ret != 0)
			return ret;
	}
	else if (msg == WM_DRAWITEM)
	{
	    auto* ptr = (PDELETEITEMSTRUCT)&lparam;
		memcpy(&drawItem, ptr, 48);
		objPtr = global::objMap[drawItem.hwndItem];
		if (objPtr != 0)
		{
			ret = objPtr->_OnDrawItem(wparam, lparam);
			if (ret != 0)
				return ret;
		}
	}

	else if (msg == WM_DROPFILES)
	{
		TCHAR tmp[255] = { 0 };
		if (_evDropFiles != 0)
		{
			_tcscpy(tmp, fileName.c_str());
			hi = DragQueryFile((HDROP)wparam, (UINT)-1, tmp, 0);
			for (lo = 0; lo < hi; lo++)
			{
				TCHAR str[MAX_PATH] = { 0 };
				fileName = str;
				DragQueryFile((HDROP)wparam, lo - 1, tmp, MAX_PATH);
				dropFileName = dropFileName + fileName;
				if (hi != lo)
					dropFileName = dropFileName + _TS("\n");
			}
			DragFinish((HDROP)wparam);
			_evDropFiles(dropFileName);
		}
	}

	else if (msg == WM_MOUSELEAVE)
	{
		ret = _OnMouseLeave(hwnd);
		if (ret != 0)
			return ret;
	}


	else if (msg == WM_MOVING)
	{
		ret = _OnMoving(hwnd, lparam);
		if (ret != 0)
			return ret;
	}

	else if (msg == WM_MOVE)
	{
		ret = _OnMove(hwnd, LOWORD(lparam), HIWORD(lparam));
		if (ret != 0)
			return ret;
	}

	else if (msg == WM_SIZE)
	{
		_OnSize(LOWORD(lparam), HIWORD(lparam));
	}
	else if (msg == WM_SIZING)
	{

	}

	return (CallWindowProc(this->_wndProcOld, hwnd, msg, wparam, lparam));
}

void Base::_baseReset(HWND)
{

}
void Base::baseReset(HWND hwnd)
{
	Base *child = NULL;
	if (this->_parent)
	{
		this->_parent->lostChild(this);
	}
	if (hwnd == NULL)
		return;
	this->_baseReset(hwnd);
	if (this->_wndProcOld)
		SetWindowLongPtr(this->_hwnd, GWLP_WNDPROC, (LONG_PTR)this->_wndProcOld);
	DestroyWindow(this->_hwnd);
	global::objMap.erase(hwnd);
	if (this->_mouseCursor)
		DestroyCursor(this->_mouseCursor);
	if (this->_fontHandle)
		DeleteObject(this->_fontHandle);
	_hwnd = NULL;
	_wndProcOld = NULL;
	_mouseCursor = NULL;
	_evMouseLeftDown = NULL;
	_evMouseLeftUp = NULL;
	_evMouseDoubleClick = NULL;
	_evMouseRightDown = NULL;
	_evMouseRightUp = NULL;
	_evMouseMove = NULL;
	_evGetFocus = NULL;
	_evLoseFocus = NULL;
	_evkeyDown = NULL;
	_evkeyUp = NULL;
	_evChar = NULL;
	_evMouseWheel = NULL;
	_evDropFiles = NULL;
	_mouseMoveIn = NULL;
	_mouseMoveOut = NULL;
	_evPaint = NULL;
	_isMoveIn = NULL;
	_fontHandle = NULL;
}
LONG_PTR Base::assignId()
{
	return global::widgetId++;
}

tstring Base::getWindowText()
{
	int len = GetWindowTextLength(this->_hwnd) + 1;
	
	tchar * str = (tchar *)malloc(len * sizeof(_T('a')));
	GetWindowText(this->_hwnd, str, len);
	tstring s(str);
	free(str);
	return s;
}

bool Base::setWindowText(const tstring &text)
{
	return SetWindowText(this->_hwnd, text.c_str()) != 0;
}

RECT Base::getClientRectWithoutXY()
{
	RECT rc;
	GetClientRect(this->_hwnd, &rc);
	return rc;
}

void Base::move(int x, int y)
{
	RECT rc;
	GetWindowRect(this->_hwnd, &rc);
	MoveWindow(this->_hwnd, x, y, rc.right - rc.left, rc.bottom - rc.top, true);
}

void Base::size(int w, int d)
{
	RECT rc = this->getRect();
	MoveWindow(this->_hwnd, rc.left, rc.top, w, d, true);
}

LONG Base::addStyle(LONG style)
{
	LONG oldStyle = GetWindowLong(this->_hwnd, GWL_STYLE);
	LONG newStyle = SetWindowLong(this->_hwnd, GWL_STYLE,
		style | oldStyle);
	SetWindowPos(this->_hwnd, 0, 0, 0, 0, 0,
		SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_FRAMECHANGED);
	return newStyle;
}

LONG Base::getStyle()
{
	return GetWindowLong(this->_hwnd, GWL_STYLE);
}

LONG Base::removeStyle(LONG style)
{
	LONG oldStyle = GetWindowLong(this->_hwnd, GWL_STYLE);
	LONG newStyle = SetWindowLong(this->_hwnd, GWL_STYLE,
		~style & oldStyle); 
	SetWindowPos(this->_hwnd, 0, 0, 0, 0, 0,
		SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_FRAMECHANGED);
	return newStyle;
}

LONG Base::addExStyle(LONG exStyle)
{
	LONG oldExStyle = GetWindowLong(this->_hwnd, GWL_EXSTYLE);
	LONG newExStyle = SetWindowLong(this->_hwnd, GWL_EXSTYLE,
		exStyle | oldExStyle);
	SetWindowPos(this->_hwnd, 0, 0, 0, 0, 0,
		SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_FRAMECHANGED);
	return newExStyle;
}

LONG Base::getExStyle()
{
	return GetWindowLong(this->_hwnd, GWL_EXSTYLE);
}

LONG Base::removeExStyle(LONG exStyle)
{
	LONG oldExStyle = GetWindowLong(this->_hwnd, GWL_EXSTYLE);
	LONG newExStyle = SetWindowLong(this->_hwnd, GWL_EXSTYLE,
		~exStyle & oldExStyle);
	SetWindowPos(this->_hwnd, 0, 0, 0, 0, 0,
		SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_FRAMECHANGED);
	return newExStyle;
}

void Base::setParent(Box * box)
{
	if (box->_hwnd == NULL)
		return;

	this->_parent->lostChild(this);
	SetParent(this->_hwnd, box->_hwnd);
	this->_parent = box;
	box->_childList.push_back(this);
}

RECT Base::getRect()
{
	RECT rc;
	GetWindowRect(this->_hwnd, &rc);
	POINT pt;
	pt.x = rc.left;
	pt.y = rc.top;
	ScreenToClient(GetParent(this->_hwnd), &pt);
	rc.left = pt.x;
	rc.top = pt.y;
	pt.x = rc.right;
	pt.y = rc.bottom;
	ScreenToClient(GetParent(this->_hwnd), &pt);
	rc.right = pt.x;
	rc.bottom = pt.y;
	return rc;
}

SIZE Base::getSize()
{
	RECT rc;
	GetClientRect(this->_hwnd, &rc);
	int i = rc.left;
	SIZE size = { rc.right , rc.bottom };
	return size;
}

void Base::init(int widgetType, Box * parent, HWND thisHwnd, Base * thisPtr, int x, int y)
{
	thisPtr->_widgetType = widgetType;
	thisPtr->_parent = parent;
	if (parent)
	{
		parent->_childList.push_back(thisPtr);
		parent->createChildInit(this, x, y);
	}
}

LRESULT Base::_OnCommand(HWND, LPARAM, int, int)
{
	return 0;
}

LRESULT Base::_OnNotify(HWND, HWND, int, UINT_PTR, LPARAM)
{
	return 0;
}

LRESULT Base::_OnCtlColor(HWND, HDC)
{
	return 0;
}

LRESULT Base::_OnSetCursor(HWND, UINT, WPARAM, LPARAM)
{
	if (global::waitMouse)
		return (LRESULT)SetCursor(LoadCursor(NULL, IDC_WAIT));
	return 0;
}

LRESULT Base::_OnPaint(HWND)
{
	return 0;
}

LRESULT Base::_OnErasebkgnd(HWND, HDC)
{
	return 0;
}

LRESULT Base::_OnScroll(int, int, LPARAM)
{
	return 0;
}

LRESULT Base::_OnMouseMove(HWND, int, int)
{
	return 0;
}

LRESULT Base::_OnDrawItem(WPARAM, LPARAM)
{
	return 0;
}

LRESULT Base::_OnMouseLeave(HWND)
{
	return 0;
}

LRESULT Base::_OnLbuttonDown(HWND, LPARAM)
{
	return 0;
}

LRESULT Base::_OnLbuttonDblclk(HWND, LPARAM)
{
	return 0;
}

LRESULT Base::_OnKeyDown(HWND, LPARAM)
{
	return 0;
}

LRESULT Base::_OnMoving(HWND, LPARAM)
{
	return 0;
}

LRESULT Base::_OnMove(HWND, int, int)
{
	return 0;
}

LRESULT Base::_EditStreamCallback(int, int, int, int)
{
	return 0;
}

LRESULT Base::_OnLbuttonUp(HWND, LPARAM)
{
	return 0;
}

LRESULT Base::_OnInvoke(int, int, int)
{
	return 0;
}

LRESULT Base::_OnSize(int, int)
{
	return 0;
}


///////////
WindowBase::WindowBase()
{
}

WindowBase::~WindowBase()
{
}

HWND WindowBase::_createWindow(HWND parentWindow, const tstring & title, DWORD style,
	DWORD exStyle, int x, int y, int w, int d, const tstring & className)
{
	this->setBackColor(GetSysColor(COLOR_BTNFACE), false);
	HWND hwnd = this->Base::_create(parentWindow, exStyle, className.c_str(),
		title.c_str(), style, x, y, w, d, 0, NULL);
	if (hwnd != NULL && parentWindow != NULL)
	{

	}
	return hwnd;
}

void WindowBase::setBackColor(COLORREF color, bool immediate)
{
	if (this->_brush != NULL)
		DeleteObject(this->_brush);
	this->_backColor = color;
	this->_brush = CreateSolidBrush(color);
	if (immediate)
		Base::reDraw(true);
}
COLORREF WindowBase::getBackColor()
{
	return this->_backColor;
}

LRESULT WindowBase::_handleEvents(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	int id = 0;
	int ret = 0;
	if (msg == WM_MOVE && this->_evPositionChanged != NULL)
		this->_evPositionChanged(LOWORD(lparam), HIWORD(lparam));
	else if (msg == WM_SIZE && this->_evSizeChanged != NULL);
		// this->_evSizeChanged(LOWORD(lparam), HIWORD(lparam)); bug
	else if (msg == WM_ACTIVATE)
	{
		ret = LOWORD(wparam);
		if (ret == WA_INACTIVE)
		{
			if (this->_firstActiveFlag == 0)
			{
				this->_firstActiveFlag = 1;
				if (this->_evFirstActive)
					this->_evFirstActive();
			}
			if (this->_evActivated)
				this->_evActivated();
		}
		else
		{
			if (this->_evDeActivation)
				this->_evDeActivation();
		}
	}
	else if (msg == WM_SHOWWINDOW)
	{
		if (wparam)
		{
			if (this->_evShow)
				this->_evShow();
		}
		else
		{
			if (this->_evHide)
				this->_evHide();
		}
	}
	else if (msg == WM_DESTROY)
	{
		if (this->_evWillClose)
			this->_evWillClose();
	}
	else if (msg == TRAY_MSG)
	{
		if (this->_evTray)
		{
			if (lparam == WM_LBUTTONDOWN)
				SetCapture(hwnd);
			if (lparam == WM_RBUTTONDOWN)
				SetCapture(hwnd);
			if (lparam == WM_LBUTTONDBLCLK)
				this->_evTray(TRAY_LBUTTONDBLCLK);
			if (lparam == WM_RBUTTONDBLCLK)
				this->_evTray(TRAY_RBUTTONDBLCLK);
			if (lparam == WM_MOUSEMOVE)
				this->_evTray(TRAY_MOUSEMOVE);
			if (lparam == 1026)
				this->_evTray(TRAY_SHOW);
			if (lparam == 1028)
				this->_evTray(TRAY_HIDE);
			if (lparam == 1029)
				this->_evTray(TRAY_CLICK);
			if (lparam == WM_LBUTTONUP)
			{
				this->_evTray(TRAY_LBUTTONUP);
				ReleaseCapture();
			}
			if (lparam == WM_RBUTTONUP)
			{
				this->_evTray(TRAY_RBUTTONUP);
				ReleaseCapture();
			}
		}
	}
	else if (msg == this->_customMsg)
	{
		if (this->_trayIconPtr)
		{
			Shell_NotifyIcon(NIM_DELETE, this->_trayIconPtr);
			this->_trayIconPtr->uFlags = NIF_TIP | NIF_ICON | NIF_MESSAGE;
			Shell_NotifyIcon(NIM_ADD, this->_trayIconPtr);
		}
	}
	else if (msg == WM_NCACTIVATE)
	{
		if (wparam == 0 && this->_keepTitleActive)
			return 1;
	}
	else if (msg == WM_COMMAND)
	{
		id = LOWORD(wparam);
		ret = HIWORD(wparam);
		if ((ret == 0 || ret == 1) && lparam == 0)
		{
			for (std::map<int, WindowMenuPage>::iterator it = this->_menu._menuMap.begin();
				it != this->_menu._menuMap.end(); it++)
			{
				std::map<int, FUNCV>::iterator itemIt = it->second._funcMap.find(id);
				if (itemIt != it->second._funcMap.end())
				{
					if (itemIt->second != NULL)
						itemIt->second();
				}
			}
		}
	}
	else if (msg == WM_HOTKEY)
	{
		id = LOWORD(wparam);
		ret = HIWORD(wparam);
		if (this->_hotKeyDown)
			_hotKeyDown(wparam, id, ret);
	}
	return 0;
}

LRESULT WindowBase::_OnErasebkgnd(HWND hwnd, HDC hdc)
{
	RECT rc;
	GetClientRect(hwnd, &rc);
	FillRect(hdc, &rc, this->_brush);
	if (this->_backShowMode == 1)
	{
		if (this->_backImage)
		{
			HDC mdc = CreateCompatibleDC(hdc);
			HGDIOBJ oldBitMap = SelectObject(mdc, this->_backImage);
			BitBlt(hdc, rc.left, rc.top, rc.right, rc.bottom, mdc, 0, 0, SRCCOPY);
			SelectObject(mdc, oldBitMap);
			DeleteDC(mdc);
		}
	}
	if (this->_backShowMode == 2)
	{
		if (this->_backImage)
		{
			HDC mdc = CreateCompatibleDC(hdc);
			HGDIOBJ oldBitMap = SelectObject(mdc, this->_backImage);
			BITMAP bmp;
			GetObject(this->_backImage, 24, &bmp);
			StretchBlt(hdc, rc.left, rc.top, rc.right, rc.bottom, mdc, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);
			SelectObject(mdc, oldBitMap);
			DeleteDC(mdc);
		}
	}
	return 1;
}

////////////////
WidgetBase::WidgetBase()
{

}

WidgetBase::~WidgetBase()
{

}

void WidgetBase::_baseReset(HWND hwnd)
{
	if (this->_hwnd == NULL)
		return;
	_widgetReset(hwnd);
	if (this->_brush)
		DeleteObject(this->_brush);
	_frontColor = 0;
	_backColor = 0;
	_brush = NULL;
}

void WidgetBase::_widgetReset(HWND)
{

}

HWND WidgetBase::_createWidget(HWND parent, DWORD exStyle, const tstring &className, const tstring &title, DWORD style, int x, int y, int w, int d, LONG_PTR id)
{
	return this->_create(parent, exStyle, className.c_str(), title.c_str(), style, x, y, w, d, id, 0);
}

void WidgetBase::setBackColor(COLORREF color, bool immediate)
{
	if (this->_brush != NULL)
		DeleteObject(this->_brush);
	this->_backColor = color;
	if (color == (DWORD)-1)
	{
		this->addExStyle(WS_EX_TRANSPARENT);
		this->_brush = (HBRUSH)GetStockObject(NULL_BRUSH);
	}
	else
	{
		this->removeExStyle(WS_EX_TRANSPARENT);
		this->_brush = CreateSolidBrush(color);
	}
	if (immediate)
		Base::reDraw(true);
}

COLORREF WidgetBase::getBackColor()
{
	return this->_backColor;
}

LRESULT WidgetBase::_OnCtlColor(HWND hwnd, HDC hdc)
{
	if (this->_backColor == -1)
	{
		SetTextColor(hdc, this->_frontColor);
		SetBkMode(hdc, TRANSPARENT);
		HWND parent = GetParent(hwnd);
		HDC hdcOwner = GetDC(parent);
		RECT rc;
		GetWindowRect(hwnd, &rc);
		POINT pt;
		pt.x = rc.left;
		pt.y = rc.top;
		ScreenToClient(parent, &pt);
		BitBlt(hdc, 0, 0, rc.right - rc.left, rc.bottom - rc.top, hdcOwner, pt.x, pt.y, SRCCOPY);
		ReleaseDC(parent, hdcOwner);
		return (LRESULT)(this->_brush);
	}
	else
	{
		SetTextColor(hdc, this->_frontColor);
		SetBkColor(hdc, this->_backColor);
		return (LRESULT)this->_brush;
	}
	return 0;
}

////////////////
Window::Window()
{

}

Window::~Window()
{
}

Window* Window::create(Window* parent, const tstring &title, int x, int y, int w, int d, bool show, FUNCVH createEndFunc)
{
	Window* ptr = new Window;
	ptr->_show = show;
	ptr->wcreate(NULL, title, createEndFunc, false, x, y, w, d);
	ptr->init(Enum::WidgetType::Window, parent, ptr->_hwnd, ptr, x, y);
	return ptr;
}

WindowBase::WindowMenuPage::WindowMenuPage()
{
	this->_pageHandle = NULL;
	this->_funcMap.clear();
}

WindowBase::WindowMenu::WindowMenu()
{
	this->_menuHandle = NULL;
	this->_menuMap.clear();
}

int Window::createMenuPage(int id, const tstring &title, bool isTopMenu)
{
	if (this->_menu._menuHandle == NULL)
	{
		HMENU hMenu, hSubMenu;
		HICON hIcon, hIconSm;
		hMenu = CreateMenu();
		this->_menu._menuHandle = hMenu;
	}

	int itemId = (int)Base::assignId();

	if (id == 0)
	{
		HMENU menuItem = CreatePopupMenu();
		if (isTopMenu && AppendMenu(this->_menu._menuHandle, MF_STRING | MF_POPUP, (UINT_PTR)menuItem, title.c_str()) == 0)
			return 0;
		SetMenu(this->_hwnd, (HMENU)this->_menu._menuHandle);
		WindowMenuPage page;
		page._funcMap.clear();
		page._pageHandle = menuItem;
		this->_menu._menuMap[itemId] = page;
	}
	else
	{
		MenuPageMap::iterator result;
		MenuPageMap *parentMap = NULL;
		int ret = findWindowMenuPage(this->_menu._menuMap, id, &parentMap, result);
		if (ret == 0)
			return 0;
		HMENU menuItem = CreatePopupMenu();
		if (isTopMenu && AppendMenu(this->_menu._menuHandle, MF_STRING | MF_POPUP, (UINT_PTR)menuItem, title.c_str()) == 0)
			return 0;
		WindowMenuPage page;
		page._funcMap.clear();
		page._pageHandle = menuItem;
		result->second._menuMap[itemId] = page;

	}
	return itemId;
}

int Window::findWindowMenuPage(MenuPageMap& pageMap, int pageId, MenuPageMap** parentMap, MenuPageMap::iterator& result)
{
	for (MenuPageMap::iterator it = pageMap.begin(); it != pageMap.end(); it++)
	{
		if (it->first == pageId)
		{
			result = it;
			*parentMap = &pageMap;
			return 1;
		}
		if (it->second._menuMap.size() > 0)
		{
			if (findWindowMenuPage(it->second._menuMap, pageId, parentMap, result) == 1)
				return 1;
		}
	}
	return 0;
}

int Window::createMenuPageItem(const int pageId, const tstring &title, FUNCV callBackMouseClick)
{
	int itemId = (int)Base::assignId();
	WindowMenuPage& thisPage = this->_menu._menuMap[pageId];
	AppendMenu(thisPage._pageHandle, MF_STRING, itemId, title.c_str());
	thisPage._funcMap[itemId] = callBackMouseClick;
	SetMenu(this->_hwnd, this->_menu._menuHandle);
	return itemId;
}

int Window::deleteMenuPage(int pageId)
{
	MenuPageMap::iterator result;
	MenuPageMap* parentMap = NULL;
	int ret = findWindowMenuPage(this->_menu._menuMap, pageId, &parentMap, result);
	if (ret == 0)
		return 0;
	DeleteMenu(this->_menu._menuHandle, *(UINT*)&result->second._pageHandle, MF_BYCOMMAND);
	parentMap->erase(result);
	DrawMenuBar(this->_hwnd);

	return 1;
}

int Window::deleteMenuPageItem(int itemId)
{
	for (std::map<int, WindowMenuPage>::iterator it = this->_menu._menuMap.begin();
		it != this->_menu._menuMap.end(); it++)
	{
		std::map<int, FUNCV>::iterator iit = it->second._funcMap.find(itemId);
		if (iit != it->second._funcMap.end())
		{
			it->second._funcMap.erase(iit);
			break;
		}
	}
	return DeleteMenu((HMENU)this->_menu._menuHandle, itemId, MF_BYCOMMAND) != 0;
}

int Window::showMenuPage(int pageId, int x, int y)
{
	MenuPageMap::iterator result;
	MenuPageMap* parentMap = NULL;
	int ret = findWindowMenuPage(this->_menu._menuMap, pageId, &parentMap, result);
	if (ret == 0)
		return 0;
	result->second._pageHandle;
	if (x == -1)
	{
		POINT pt;
		GetCursorPos(&pt);
		//ScreenToClient(this->_hwnd, &pt);
		x = pt.x;
		y = pt.y;
	}
	TrackPopupMenu(result->second._pageHandle, TPM_LEFTALIGN, x, y, 0, this->_hwnd, NULL);
	return 1;
}

bool Window::wcreate(HWND parentHwnd, const tstring &title, FUNCVH createEndFunc,
	bool isDialog, int x, int y, int w, int d, DWORD style, DWORD exStyle,
	bool visible, const tstring &className, HACCEL hotkeyHandle, bool showInTaskBar, bool responseDialogMsg)
{
	if (global::instanceHandle == 0)
	{
		global::instanceHandle = GetModuleHandle(NULL);

		/*
		try {
		std::locale loc("zh-CN");
		std::locale::global(loc);
		}
		catch (std::runtime_error& e)
		{
		std::cerr << "Error: " << e.what() << std::endl;
		std::cerr << "Type:" << typeid(e).name() << std::endl;
		}
		*/

	}
	HWND phwnd = NULL;
	this->registerWindowClass(className.c_str(), _defaultWindowProc);
	if (createEndFunc)
		this->_createEndFunc = createEndFunc;
	if (parentHwnd == NULL && showInTaskBar == false)
		phwnd = CreateWindowEx(0, className.c_str(), _TS("").c_str(), 0, 0, 0, 250, 110, 0, 0,
			global::instanceHandle, 0);
	else
		phwnd = parentHwnd;

	HWND hwnd = this->WindowBase::_createWindow(phwnd, title.c_str(), style, exStyle, x, y, w, d,
		className.c_str());
	if (hwnd == NULL)
		return (false);
	this->Base::sendMsg(MY_WM_CREATE, 0, 0);
	if (visible)
	{
		ShowWindow(hwnd, SW_SHOW);
		UpdateWindow(hwnd);
	}

	MSG msg;
	if (isDialog)
	{
		if (parentHwnd != NULL && IsWindow(parentHwnd) != false)
			EnableWindow(parentHwnd, false);
		while (GetMessage(&msg, NULL, 0, 0))
		{
			if (TranslateAccelerator(hwnd, hotkeyHandle, &msg) == false)
				if (responseDialogMsg == false || IsDialogMessage(hwnd, &msg) == false)
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			if (IsWindow(hwnd) == false)
				break;
		}
		if (parentHwnd != NULL && IsWindow(parentHwnd) != false)
		{

			EnableWindow(parentHwnd, true);
			SetActiveWindow(parentHwnd);
		}
	}
	return (hwnd != NULL);
}

int Window::registerWindowClass(const tstring &className, WNDPROC proc, HICON, UINT classStyle)
{
	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = classStyle;
	wc.lpfnWndProc = proc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = global::instanceHandle;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDI_APPLICATION);
	wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = className.c_str();
	wc.hIconSm = LoadIcon(wc.hInstance, IDI_APPLICATION);
	return (RegisterClassEx(&wc));
}

LRESULT(CALLBACK Window::_defaultWindowProc)(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	Base* objPtr = global::objMap[hwnd];
	if (objPtr == NULL || objPtr->_wndProcOld == NULL)
		return (DefWindowProc(hwnd, msg, wparam, lparam));
	return (objPtr->_windowProc(hwnd, msg, wparam, lparam));
}

LRESULT Window::_windowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	RECT rect;
	LRESULT ret = 0;
	if (msg == MY_WM_CREATE)
	{
		if (this->_createEndFunc)
		{
			this->_createEndFunc(hwnd);
			GetClientRect(hwnd, &rect);
			//this->sendMsg(WM_SIZE, 0, (rect.right - rect.left) | ((rect.bottom - rect.top) << 16));//??
		}
	}
	else if (msg == WM_CLOSE)
	{
		if (this->_canBeClosed)
		{
			if (0 != (ret = this->_canBeClosed()))
				return ret;
		}
        PostQuitMessage(0);
		return 0;
//		DestroyWindow(hwnd);
	}
	else
	{
		ret = _handleEvents(hwnd, msg, wparam, lparam);
		if (ret)
			return 1;
	}
	return (DefWindowProc(hwnd, msg, wparam, lparam));
}

LRESULT Window::_OnCommand(HWND hwnd, LPARAM hwndCtl, int wNotifyCode, int wID)
{
	return LRESULT();
}
LRESULT Window::_OnSize(int x, int y)
{
	if (this->_hwnd && this->_evSizeChanged)
		return this->_evSizeChanged(x, y);
	return 0;
}

LRESULT Window::_OnLbuttonUp(HWND, LPARAM)
{
	return 0;
}

///////////
Button::Button()
{

}

Button::~Button()
{

}

Button* Button::create(Box * parent, const tstring & title, int x, int y, int w, int d, bool show)
{
	Button * ptr = new Button;
	ptr->_show = show;
	ptr->wcreate(parent->_hwnd, title, 0, 0, x, y, w, d);
	ptr->init(Enum::WidgetType::Button, parent, ptr->_hwnd, ptr, x, y);
	return ptr;
}
int Button::wcreate(HWND parent, const tstring &title, LONG_PTR id, DWORD style, int x, int y, int w, int d, DWORD exStyle)
{
	HWND hwnd;
	if (!id)
		id = Base::assignId();
	hwnd = _create(parent, exStyle, _TS("Button"), title.c_str(), WS_CHILD | WS_TABSTOP | WS_VISIBLE | style, x, y, w, d, id, 0);
	return hwnd != NULL;
}

void Button::evClick(FUNCV callback)
{
	this->_evClick = callback;
}

LRESULT Button::_OnCommand(HWND, LPARAM, int wNotifyCode, int)
{
	if (wNotifyCode == BN_CLICKED)
	{
		if (_evClick)
			_evClick();
	}
	return 0;
}

/////////////////////////
#if(_MSC_VER >=1200)
#pragma warning(disable:4100)
#endif
Label::Label()
{

}

Label::~Label()
{

}

Label* Label::create(Box * parent, const tstring & title, int x, int y, int w, int d, bool show)
{
	Label * ptr = new Label;
	ptr->_show = show;
	ptr->wcreate(parent->_hwnd, title, SS_CENTERIMAGE, x, y, w, d);
	ptr->init(Enum::WidgetType::Label, parent, ptr->_hwnd, ptr, x, y);
	return ptr;
}

bool Label::wcreate(HWND parent, const tstring &title, DWORD style, int x, int y, int w, int d, DWORD exStyle)
{
	HWND hwnd;
	hwnd = _create(parent, exStyle, _TS("Static"), title.c_str(), WS_CHILD | WS_TABSTOP | WS_VISIBLE | style, x, y, w, d, Base::assignId(), 0);
	return hwnd != NULL;
}

void Label::setText(const tstring & text)
{
	Base::setWindowText(text);
	if (this->_backColor == (DWORD)-1)
	{
		HWND parent = NULL;
		RECT rc = this->Base::getRect();
		parent = GetParent(this->_hwnd);
		InvalidateRect(parent, &rc, TRUE);
		UpdateWindow(parent);
	}
}

tstring Label::getText()
{
	return Base::getWindowText();
}
//////////////////////

Edit::Edit()
{

}

Edit::~Edit()
{

}

Edit* Edit::create(Box * parent, const tstring & text, int x, int y, int w, int d, bool show, int flag)
{
	Edit * ptr = new Edit;
	ptr->_show = show;
	int win_flag = 0;
	if (flag & INIT_OPTION_PASSWARD)
		win_flag |= ES_PASSWORD;
	if (flag & INIT_OPTION_TEXT_LEFT)
		win_flag |= ES_LEFT;
	if (flag & INIT_OPTION_TEXT_RIGHT)
		win_flag |= ES_RIGHT;
	if (flag & INIT_OPTION_READ_ONLY)
		win_flag |= ES_READONLY;
	ptr->wcreate(parent->_hwnd, text, 0, win_flag, x, y, w, d, 0);
	ptr->init(Enum::WidgetType::Edit, parent, ptr->_hwnd, ptr, x, y);
	return ptr;
}
bool Edit::wcreate(HWND parent, const tstring &text, LONG_PTR id, DWORD style, int x, int y, int w, int d, DWORD exStyle)
{
	HWND hwnd;
	if (!id)
		id = Base::assignId();
	if (!exStyle)
		exStyle |= WS_EX_CLIENTEDGE;
	hwnd = _create(parent, exStyle, _TS("Edit"), text.c_str(), WS_CHILD | WS_VISIBLE | style, x, y, w, d, id, 0);
	return hwnd != NULL;
}

LRESULT Edit::_OnCommand(HWND hwnd, LPARAM hwndCtl, int wNotifyCode, int wID)
{
	if (wNotifyCode == EN_CHANGE)
	{
		if (this->_evTextChanged)
			this->_evTextChanged();
	}
	return 0;
}

bool Edit::setText(const tstring & text)
{
	return Base::setWindowText(text);
}

tstring Edit::getText()
{
	return Base::getWindowText();
}

void Edit::evTextChanged(FUNCV ev)
{
	this->_evTextChanged = ev;
}

int Edit::openOtherOption(int flag)
{
	int win_flag = 0;
	if (flag & OTHER_OPTION_TEXT_LEFT)
		win_flag |= ES_LEFT;
	if (flag & OTHER_OPTION_TEXT_RIGHT)
		win_flag |= ES_RIGHT;
	this->addStyle(win_flag);
	return 0;
}

int Edit::closeOtherOption(int flag)
{
	int win_flag = 0;
	if (flag & OTHER_OPTION_TEXT_LEFT)
		win_flag |= ES_LEFT;
	if (flag & OTHER_OPTION_TEXT_RIGHT)
		win_flag |= ES_RIGHT;
	this->removeStyle(win_flag);
	return 0;
}

/////////////////////////////////////////


MultiLineEdit::MultiLineEdit()
{

}

MultiLineEdit::~MultiLineEdit()
{

}

MultiLineEdit* MultiLineEdit::create(Box * parent, const tstring & text, int x, int y, int w, int d, bool show, int flag)
{
	int win_flag = 0;
	if (flag & INIT_OPTION_HSCROLL)
		win_flag |= WS_HSCROLL;
	if (flag & INIT_OPTION_VSCROLL)
		win_flag |= WS_VSCROLL;
	if (flag & INIT_OPTION_NO_WRAP)
		win_flag |= ES_AUTOHSCROLL;
	if (flag & INIT_OPTION_TEXT_LEFT)
		win_flag |= ES_LEFT;
	if (flag & INIT_OPTION_TEXT_RIGHT)
		win_flag |= ES_RIGHT;
	if (flag & INIT_OPTION_READ_ONLY)
		win_flag |= ES_READONLY;
	MultiLineEdit * ptr = new MultiLineEdit;
	ptr->_show = show;
	ptr->wcreate(parent->_hwnd, text, 0, ES_MULTILINE | ES_WANTRETURN | ES_AUTOVSCROLL | win_flag, x, y, w, d, 0);
	ptr->init(Enum::WidgetType::MultiLineEdit, parent, ptr->_hwnd, ptr, x, y);
	return ptr;
}

bool MultiLineEdit::wcreate(HWND parent, const tstring &text, LONG_PTR id, DWORD style, int x, int y, int w, int d, DWORD exStyle)
{
	HWND hwnd;
	if (!id)
		id = Base::assignId();
	if (!exStyle)
		exStyle |= WS_EX_CLIENTEDGE;
	hwnd = _create(parent, exStyle, _TS("Edit"), text.c_str(), WS_CHILD | WS_VISIBLE | style, x, y, w, d, id, 0);
	return hwnd != NULL;
}

LRESULT MultiLineEdit::_OnCommand(HWND hwnd, LPARAM hwndCtl, int wNotifyCode, int wID)
{
	if (wNotifyCode == EN_CHANGE)
	{
		if (this->_evTextChanged)
			this->_evTextChanged();
	}
	return 0;
}

bool MultiLineEdit::setText(const tstring & text)
{
	return Base::setWindowText(text);
}

tstring MultiLineEdit::getText()
{
	return Base::getWindowText();
}

void MultiLineEdit::evTextChanged(FUNCV ev)
{
	this->_evTextChanged = ev;
}

int MultiLineEdit::openOtherOption(int flag)
{
	int win_flag = 0;
	if (flag & OTHER_OPTION_HSCROLL)
		win_flag |= WS_HSCROLL;
	if (flag & OTHER_OPTION_VSCROLL)
		win_flag |= WS_VSCROLL;
	this->addStyle(win_flag);
	return 0;
}

int MultiLineEdit::closeOtherOption(int flag)
{
	int win_flag = 0;
	if (flag & OTHER_OPTION_HSCROLL)
		win_flag |= WS_HSCROLL;
	if (flag & OTHER_OPTION_VSCROLL)
		win_flag |= WS_VSCROLL;

	this->removeStyle(win_flag);
	return 0;
}

/////////////////////////////////////////
#if(_MSC_VER >=1200)
#pragma warning(disable:4100)
#endif
Box::Box()
{
	this->_childList.clear();
}

Box::~Box()
{
	while (this->_childList.size())
	{
		delete (this->_childList.front());
	}
}

void Box::lostChild(Base * child)
{
	std::list<Base*>::iterator it = find(this->_childList.begin(), this->_childList.end(), child);
	this->_childList.erase(it);

}

void Box::createChildInit(Base * child, int x, int y)
{
}

GroupBox::GroupBox()
{
}


GroupBox::~GroupBox()
{
	std::cout << "group";
}

void GroupBox::_baseReset(HWND hwnd)
{
	this->WidgetBase::_baseReset(hwnd);
}

GroupBox* GroupBox::create(Box * parent, const tstring & title, int x, int y, int w, int d, bool show)
{
	GroupBox* ptr = new GroupBox;
	ptr->_show = show;
	ptr->wcreate(parent->_hwnd, title, 0, x, y, w, d);
	ptr->init(Enum::WidgetType::GroupBox, parent, ptr->_hwnd, ptr, x, y);
	return ptr;
}

bool GroupBox::wcreate(HWND parent, const tstring & title, DWORD style, int x, int y, int w, int d, DWORD exStyle)
{
	HWND hwnd;
	hwnd = _create(parent, exStyle, _TS("Button"), title.c_str(), WS_CHILD | BS_GROUPBOX | WS_VISIBLE | style, x, y, w, d, Base::assignId(), 0);
	if (hwnd)
		this->setBackColor(GetSysColor(COLOR_BTNFACE), true);
	return hwnd != NULL;
}

LRESULT GroupBox::_OnErasebkgnd(HWND hwnd, HDC hdc)
{
	RECT rc;
	GetClientRect(hwnd, &rc);
	FillRect(hdc, &rc, this->_brush);
	if (this->_brush)
	{
		RECT rc;
		GetClientRect(hwnd, &rc);
		FillRect(hdc, &rc, this->_brush);
	}
	return 1;
}

LRESULT GroupBox::_OnCtlColor(HWND hwnd, HDC hdc)
{
	SetTextColor(hdc, this->_frontColor);
	SetBkColor(hdc, this->_backColor);
	return (LRESULT)this->_brush;
}

NoteBox::NoteBox()
{
	_notePageVec.clear();
	_hide = false;
	_evPageWillChange = NULL;
	_evPageChanged = NULL;
	_evClick = NULL;
	INITCOMMONCONTROLSEX control;
	control.dwSize = sizeof(control);
	control.dwICC = ICC_TAB_CLASSES;
	InitCommonControlsEx(&control);
}

NoteBox::~NoteBox()
{
}

void NoteBox::_baseReset(HWND hwnd)
{
	this->WidgetBase::_baseReset(hwnd);
}

NoteBox * NoteBox::create(Box * parent, const tstring & title, int x, int y, int w, int d, bool show)
{
	NoteBox* ptr = new NoteBox;
	ptr->_show = show;
	ptr->wcreate(parent->_hwnd, title, 0, x, y, w, d);
	ptr->init(Enum::WidgetType::NoteBox, parent, ptr->_hwnd, ptr, x, y);
	int index = ptr->addPage(-1, title);
	if (index == -1)
		return NULL;
	ptr->selectPage(index);
	return ptr;
}

bool NoteBox::wcreate(HWND parent, const tstring & title, DWORD style, int x, int y, int w, int d, DWORD exStyle)
{
	HWND hwnd;
	hwnd = _create(parent, exStyle, _TS("SysTabControl32"), title.c_str(), WS_CHILD | WS_VISIBLE | style, x, y, w, d, Base::assignId(), 0);
	return hwnd != NULL;
}

bool NoteBox::addWidget(WidgetBase * widget, int pageIndex, bool show, int x, int y)
{
	if (pageIndex < 0 || pageIndex >= getPageAmount())
		return false;
	if (widget->_hwnd == NULL)
		return false;
	NoteWidget noteWidget;
	noteWidget.pageIndex = pageIndex;
	noteWidget.show = show;
	noteWidget.widget = widget;
	widget->setParent(this);
	//	if (pageIndex < 0 || pageIndex >= _noteWidgetMapVec.size())
	//	return false;
	this->_notePageVec[pageIndex].childMap[widget] = noteWidget;
	LONG_PTR style = this->getStyle();
	if ((style & TCS_VERTICAL) == 0)
		if ((style & TCS_BOTTOM) == 0)
			widget->move(x, y + getTabHight());
		else
			widget->move(x, y);
	else
		if ((style & TCS_RIGHT) == 0)
			widget->move(x + getTabWidth(), y);
		else
			widget->move(x, y);
	this->refreshWidget();
	return true;
}

int NoteBox::getPageAmount()
{
	return (int)sendMsg(TCM_GETITEMCOUNT, 0, 0);
}

int NoteBox::getTabHight()
{
	RECT rc;
	this->sendMsg(TCM_GETITEMRECT, 0, (LPARAM)&rc);
	int lineNum = (int)this->sendMsg(TCM_GETROWCOUNT, 0, 0);
	return (lineNum*(rc.bottom - rc.top));
}

int NoteBox::getTabWidth()
{
	RECT rc;
	this->sendMsg(TCM_GETITEMRECT, 0, (LPARAM)&rc);
	int lineNum = (int)this->sendMsg(TCM_GETROWCOUNT, 0, 0);
	return (lineNum*(rc.right - rc.left));
}

int NoteBox::getSelectIndex()
{
	return (int)sendMsg(TCM_GETCURSEL, 0, 0);
}

void NoteBox::refreshWidget()
{
	int currentIndex = getSelectIndex();
	for (size_t i = 0; i < _notePageVec.size(); i++)
		for (std::map<Base*, NoteWidget>::iterator it = _notePageVec[i].childMap.begin(); it != _notePageVec[i].childMap.end(); it++)
		{
			NoteWidget& noteWidget = (*it).second;
			if (noteWidget.pageIndex == currentIndex)
			{
				if (noteWidget.show == true)
				{
					ShowWindow(noteWidget.widget->_hwnd, SW_SHOW);
				}
				else
				{
					ShowWindow(noteWidget.widget->_hwnd, SW_HIDE);
				}
			}
			else
			{
				ShowWindow(noteWidget.widget->_hwnd, SW_HIDE);
			}
		}
}

int NoteBox::addPage(int index, const tstring & title, int lparam, int iImage)
{
	if (index >= (int)_notePageVec.size())
		return -1;
	TCITEM page;
	TCHAR str[MAX_PATH] = { 0 };
	_tcscpy(str, title.c_str());
	bool last = false;
	if (index == -1)
		last = true;
	if (last)
		index = (int)sendMsg(TCM_GETITEMCOUNT, 0, 0);
	page.mask = (TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM);
	page.pszText = str;
	page.lParam = lparam;
	page.cchTextMax = MAX_PATH;
	page.iImage = iImage;
	if (-1 == sendMsg(TCM_INSERTITEM, index, (LPARAM)&page))
		return -1;
	NotePageInfo pageInfo;
	pageInfo.index = index;
	pageInfo.childMap = NoteWidgetMap();
	if (last)
	{
		_notePageVec.push_back(pageInfo);
	}
	else
	{
		for (NoteWidgetMapVec::iterator it = _notePageVec.begin(); it != _notePageVec.end(); it++)
		{
			if (it->index >= index)
			{
				tmpSetPageIndex(*it, it->index + 1);
			}
		}
		_notePageVec.insert(_notePageVec.begin() + index, pageInfo);
	}
	reDraw(true);
	return index;
}

int NoteBox::removePage(int index)
{
	int oldSelectIndex = getSelectIndex();
	if (sendMsg(TCM_DELETEITEM, index, 0) != 0)
	{
		NoteWidgetMap& childMap = _notePageVec[index].childMap;
		while (childMap.size()>0)
		{
			NoteWidgetMap::iterator it = childMap.begin();
			delete it->second.widget;
		}
		_notePageVec.erase(_notePageVec.begin() + index);
		for (NoteWidgetMapVec::iterator it = _notePageVec.begin(); it != _notePageVec.end(); it++)
		{
			if (it->index >= index)
			{
				tmpSetPageIndex(*it, it->index - 1);
			}
		}
	}
	if (oldSelectIndex == index)
	{
		if (index > 0)
		{
			if ((int)this->_notePageVec.size()>index)
				selectPage(index);
			else
				selectPage(index - 1);
		}
		else
		{
			if (this->_notePageVec.size() > 0)
				selectPage(index);
		}
	}

	if (this->_notePageVec.size() == 0)
	{
		addPage(-1);
		selectPage(0);
	}
	return 0;
}

int NoteBox::selectPage(int index)
{
	sendMsg(TCM_SETCURSEL, index, 0);
	this->refreshWidget();
	return 0;
}

void NoteBox::tmpSetPageIndex(NotePageInfo& page, int index)
{
	page.index = index;
	for (NoteWidgetMap::iterator it = page.childMap.begin(); it != page.childMap.end(); it++)
	{
		it->second.pageIndex = index;
	}
}
LRESULT NoteBox::_OnCtlColor(HWND hwnd, HDC hdc)
{
	return WidgetBase::_OnCtlColor(hwnd, hdc);
}
void NoteBox::lostChild(Base * child)
{
	Box::lostChild(child);
	for (size_t i = 0; i < _notePageVec.size(); i++)
	{
		if (_notePageVec[i].childMap.find(child) != _notePageVec[i].childMap.end())
		{
			_notePageVec[i].childMap.erase(child);
			break;
		}
	}
	refreshWidget();
}

LRESULT NoteBox::_OnPaint(HWND hwnd)
{
	if (_hide)
	{
		PAINTSTRUCT ps;
		RECT rc;
		HDC hdc = BeginPaint(hwnd, &ps);
		GetClientRect(hwnd, &rc);
		FillRect(hdc, &rc, this->_brush);
		EndPaint((HWND)&hdc, &ps);
	}
	return 0;
}

LRESULT NoteBox::_OnNotify(HWND hwnd, HWND hwndFrom, int code, UINT_PTR idfrom, LPARAM lparam)
{
	if (_hide)
		return 1;
	if (code == TCN_SELCHANGE)
	{
		refreshWidget();
		if (this->_evPageChanged)
			_evPageChanged();
	}
	else if (code == TCN_SELCHANGING)
	{
		if (_evPageWillChange)
			_evPageWillChange();
	}
	else if (code == NM_CLICK)
	{
		if (_evClick)
		{
			_evClick();
			return 1;
		}
	}
	return 0;
}

void NoteBox::createChildInit(Base * child, int x, int y)
{
	WidgetBase * widgetChild = dynamic_cast<WidgetBase *>(child);
	if (widgetChild == NULL)
		return;
	this->addWidget(widgetChild, 0, child->_show, x, y);
}

///////////////////////
Image::Image()
{

}

Image::~Image()
{

}

Image* Image::create(Box * parent, const tstring & title, int x, int y, int w, int d, bool show)
{

	Image * ptr = new Image;
	ptr->_show = show;
	ptr->m_pImage = Gdiplus::Image::FromFile(enc::t2w(title).c_str());
	if (ptr->m_pImage == NULL)
		return NULL;
	if (w == -1 || d == -1)
	{
		w = ptr->m_pImage->GetWidth();
		d = ptr->m_pImage->GetHeight();
	}

	ptr->wcreate(parent->_hwnd, _T(""), 0, x, y, w, d);
	ptr->init(Enum::WidgetType::Image, parent, ptr->_hwnd, ptr, x, y);

	/*
	std::ifstream in("C:\\Users\\ftp\\Desktop\\1.png");
	if (!in.is_open()) return NULL;
	in.seekg(0, std::ios_base::end);
	size_t len = (size_t)in.tellg().seekpos();

	in.seekg(0, std::ios::beg);
	HGLOBAL hMem = GlobalAlloc(GMEM_FIXED, len);
	BYTE* pMem = (BYTE*)GlobalLock(hMem);
	in.read((char *)pMem, len);
	//memcpy(pMem, buf, len);
	IStream* pStream;
	HRESULT hr = CreateStreamOnHGlobal(pMem, FALSE, &pStream);
	pStream->Release();
	GlobalUnlock(hMem);
	GlobalFree(hMem);
	ptr->m_pImage = Gdiplus::Image::FromStream(pStream);
	*/


	return ptr;
}

bool Image::wcreate(HWND parent, const tstring &title, DWORD style, int x, int y, int w, int d, DWORD exStyle)
{
	HWND hwnd;
	hwnd = _create(parent, exStyle | WS_EX_TRANSPARENT, _TS("Static"), title.c_str(), WS_CHILD | WS_VISIBLE | style, x, y, w, d, Base::assignId(), 0);
	return hwnd != NULL;
}

void Image::setText(const tstring & text)
{
	Base::setWindowText(text);
	if (this->_backColor == (DWORD)-1)
	{
		HWND parent = NULL;
		RECT rc = this->Base::getRect();
		parent = GetParent(this->_hwnd);
		InvalidateRect(parent, &rc, TRUE);
		UpdateWindow(parent);
	}
}

tstring Image::getText()
{
	return Base::getWindowText();
}

LRESULT Image::_OnPaint(HWND hwnd)
{
	PAINTSTRUCT ps;
	BeginPaint(hwnd, &ps);
	HDC hdc = GetDC(hwnd);
	RECT rc;
	GetClientRect(hwnd, &rc);
	Gdiplus::Graphics graphics(hdc);
	graphics.DrawImage(this->m_pImage, Gdiplus::Rect(0, 0, this->m_pImage->GetWidth(), this->m_pImage->GetHeight()),
		0, 0, this->m_pImage->GetWidth(), this->m_pImage->GetHeight(),
		Gdiplus::UnitPixel);
	EndPaint(hwnd, &ps);
	return 1;
}

LRESULT Image::_OnCtlColor(HWND hwnd, HDC hdc)
{
	return 0;
}

LRESULT Image::_OnErasebkgnd(HWND hwnd, HDC hdc)
{
	return 1;
	PAINTSTRUCT ps;
	BeginPaint(hwnd, &ps);
	RECT rc;
	GetClientRect(hwnd, &rc);
	Gdiplus::Graphics graphics(hdc);
	graphics.DrawImage(this->m_pImage, Gdiplus::Rect(0, 0, this->m_pImage->GetWidth(), this->m_pImage->GetHeight()),
		0, 0, this->m_pImage->GetWidth(), this->m_pImage->GetHeight(),
		Gdiplus::UnitPixel);
	EndPaint(hwnd, &ps);
	return 1;
}

LRESULT Image::_OnSize(int x, int y)
{

	return 0;
}
//////////////////////

List::List()
{
	INITCOMMONCONTROLSEX control;
	control.dwSize = sizeof(control);
	control.dwICC = ICC_TAB_CLASSES;
	InitCommonControlsEx(&control);
}

List::~List()
{

}

List* List::create(Box * parent, int column, int x, int y, int w, int d, bool show)
{
	List * ptr = new List;
	ptr->_show = show;
	ptr->wcreate(parent->_hwnd, LVS_REPORT, x, y, w, d, 0);
	ptr->init(Enum::WidgetType::List, parent, ptr->_hwnd, ptr, x, y);
	for (int i = 0; i<column; i++)
		ptr->___insertColumn();
    int defaultWidth = std::max<int>(100,int( w / column));

	for (int i = 0; i< column - 1; i++)
	{
		ptr->setColumnWidth(i, defaultWidth);
	}
	if (defaultWidth != 100)
		ptr->setColumnWidth(column - 1, w - defaultWidth * (column - 1));
	else
		ptr->setColumnWidth(column - 1, defaultWidth);
	return ptr;
}

bool List::wcreate(HWND parent, DWORD style, int x, int y, int w, int d, DWORD exStyle)
{
	HWND hwnd = _create(parent, exStyle, _TS("SysListView32"), _TS("ListView"), WS_CHILD | WS_VISIBLE | style, x, y, w, d, Base::assignId(), 0);
	if (hwnd != NULL)
		this->sendMsg(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_SUBITEMIMAGES | LVS_EX_INFOTIP);
	return hwnd != NULL;
}

LRESULT List::_OnNotify(HWND hwnd, HWND hwndFrom, int code, UINT_PTR idfrom, LPARAM lparam)
{
	NMLISTVIEW* lvView;
	NMLVDISPINFO* lvDispInfo;
	LPNMITEMACTIVATE itemActivate;
	int ret = 0;
	if (code == LVN_ITEMCHANGED)
	{
		lvView = (NMLISTVIEW*)lparam;
		if ((lvView->uNewState & LVIS_SELECTED) == LVIS_SELECTED)
			if (_evItemChanged)
				_evItemChanged(lvView->iItem, lvView->lParam);
	}
	if (code == LVN_HOTTRACK)
	{
		lvView = (NMLISTVIEW*)lparam;
		if (_evItemMouseIn)
			_evItemMouseIn(lvView->iItem);
	}
	if (code == LVN_COLUMNCLICK)
	{
		lvView = (NMLISTVIEW*)lparam;
		if (_evColumnClick)
			_evColumnClick(lvView->iSubItem);
	}
	if (code == LVN_ITEMACTIVATE)
	{
		lvView = (NMLISTVIEW*)lparam;
		if (_evItemActivate)
			_evItemActivate(lvView->iItem);
	}
	if (code == LVN_BEGINLABELEDIT)
	{
		lvDispInfo = (NMLVDISPINFO*)lparam;
		if (_evItemActivate)
			_evItemActivate(lvDispInfo->item.iItem);
	}
	if (code == LVN_ENDLABELEDIT)
	{
		lvDispInfo = (NMLVDISPINFO*)lparam;
		if (_evItemEndEdit)
			_evItemEndEdit(lvDispInfo->item.iItem);
	}
	if (code == LVN_ITEMCHANGING)
	{
		lvView = (NMLISTVIEW*)lparam;
		if (((lvView->uNewState & 4096) != 0) || ((lvView->uNewState & 8192) != 0))
			if (_evItemChanging)
				_evItemChanging(lvView->iItem, lvView->uNewState / 4096 - 1);
	}
	if (code == NM_RCLICK)
	{
		itemActivate = (LPNMITEMACTIVATE)lparam;
		if (_evItemMouseRClick)
			_evItemMouseRClick(itemActivate->iItem, itemActivate->uKeyFlags);
	}
	if (code == NM_CLICK)
	{
		itemActivate = (LPNMITEMACTIVATE)lparam;
		if (_evItemMouseRClick)
			_evItemMouseRClick(itemActivate->iItem, itemActivate->uKeyFlags);
	}
	if (code == NM_DBLCLK)
	{
		itemActivate = (LPNMITEMACTIVATE)lparam;
		if (_evItemMouseRClick)
			_evItemMouseRClick(itemActivate->iItem, itemActivate->uKeyFlags);
	}
	return 0;
}

int List::insertRow(int rowIndex)
{
	if (rowIndex == -1)
		rowIndex = getRowNum();
	LVITEM item = { 0 };
	item.mask = LVIF_TEXT;
	item.iItem = rowIndex;
	item.pszText = &(_TS(""))[0];
	if (this->sendMsg(LVM_INSERTITEM, 0, (LPARAM)&item) == -1)
		return false;
	return true;
}

int List::deleteRow(int rowIndex)
{
	return (int)this->sendMsg(LVM_DELETEITEM, rowIndex, 0);
}

int List::deleteAll()
{
	return (int)sendMsg(LVM_DELETEALLITEMS, 0, 0);
}

int List::getRowNum()
{
	return (int)sendMsg(LVM_GETITEMCOUNT, 0, 0);
}

int List::___insertColumn(tstring title, int columIndex)
{
	int mask = 0;
	LVCOLUMN column;
	column.mask = mask | LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
	column.fmt = LVCFMT_LEFT;
	column.cx = this->getSize().cx;
	column.iImage = 0;
	column.pszText = &title[0];
	column.cchTextMax = (int)title.length();
	if (columIndex == -1)
		columIndex = getColumnNum();
	return (int)this->sendMsg(LVM_INSERTCOLUMN, columIndex, (LPARAM)&column);
}

int List::___deleteColumn(int columnIndex)
{
	return (int)this->sendMsg(LVM_DELETECOLUMN, columnIndex, 0);
}

tstring List::getColumnText(int columnIndex)
{
	tchar str[1024] = { 0 };
	LVCOLUMN column = { 0 };
	column.mask = LVCF_TEXT;
	column.pszText = str;
	column.cchTextMax = 1024;
	this->sendMsg(LVM_GETCOLUMN, columnIndex, (LPARAM)&column);
	return tstring(column.pszText);
}

int List::setColumnText(int columnIndex, tstring title)
{
	LVCOLUMN column = { 0 };
	column.mask = LVCF_TEXT;
	column.pszText = &title[0];
	column.cchTextMax = (int)title.length();
	return (int)this->sendMsg(LVM_SETCOLUMN, columnIndex, (LPARAM)&column) != 0;
}

tstring List::getText(int lineIndex, int columIndex)
{
	LVITEM item = { 0 };
	tchar str[1024] = { 0 };
	item.iSubItem = columIndex;
	item.pszText = str;
	item.cchTextMax = 1024;
	if (sendMsg(LVM_GETITEMTEXT, lineIndex, (LPARAM)&item) != 0)
	{
		return tstring(item.pszText);
	}
	return _TS("");
}

int List::setText(int lineIndex, int columnIndex, tstring title)
{
	LVITEM item;
	item.mask = LVIF_TEXT;
	item.iItem = lineIndex;
	item.iSubItem = columnIndex;
	item.pszText = &title[0];
	item.cchTextMax = (int)title.length();
	return this->sendMsg(LVM_SETITEM, columnIndex, (LPARAM)&item) != 0;
}

int List::getColumnWidth(int columnIndex)
{
	return (int)sendMsg(LVM_GETCOLUMNWIDTH, columnIndex, 0);
}

int List::setColumnWidth(int columnIndex, int width)
{
	return sendMsg(LVM_SETCOLUMNWIDTH, columnIndex, width) != 0;
}

int List::getColumnNum()
{
	HWND head = (HWND)sendMsg(LVM_GETHEADER, 0, 0);
	return (int)SendMessage(head, HDM_GETITEMCOUNT, 0, 0);
}

