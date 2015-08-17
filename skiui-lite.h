#ifndef SKIUI_H
#define SKIUI_H

#define SKIUI_LITE_CXX98


#include <iostream>
#include <string>
#include <map>
#include <set>
#include <string>
#include <list>
#include <vector>
#include <bitset>
#include <time.h>
namespace Enum
{
	namespace WidgetType
	{
		enum
		{
			Window, GroupBox, NoteBox, Label, Button, Edit, MultiLineEdit, List, Image
		};
	}
}
#pragma comment(lib,"gdiplus.lib")  
#include <Windows.h>
#include <tchar.h>
#include <GdiPlus.h>
#define MY_WM_CREATE 1026
//#define WM_FORWARDMSG 895
#define TRAY_MSG 12313
#define TRAY_LBUTTONUP 1
#define TRAY_RBUTTONUP 2
#define TRAY_LBUTTONDBLCLK 3
#define TRAY_RBUTTONDBLCLK 4
#define TRAY_MOUSEMOVE 5
#define TRAY_SHOW 6
#define TRAY_HIDE 7
#define TRAY_CLICK 8
#include <functional>

#ifdef _UNICODE
typedef std::wstring tstring;
typedef wchar_t tchar;
extern tstring _TS(const std::wstring& tmp);
extern tstring _TS(const std::string& tmp);
extern tstring _TS(const wchar_t* tmp);
extern tstring _TS(const char* tmp);
#else
typedef std::string tstring;
typedef char tchar;
extern tstring _TS(const std::wstring& tmp);
extern tstring _TS(const std::string& tmp);
extern tstring _TS(const wchar_t* tmp);
extern tstring _TS(const char* tmp);
#endif
typedef LONG_PTR PTR;
#if !defined(SKIUI_LITE_CXX98)
typedef void(*FUNCV)();
typedef int(*FUNCILPMSG)(LPMSG);
typedef void(*FUNCVH)(HWND);
typedef int(*FUNCI)();
typedef LONG_PTR(*FUNCL)();
typedef LONG_PTR(*FUNCLL)(LONG_PTR);
typedef LONG_PTR(*FUNCLLL)(LONG_PTR, LONG_PTR);
typedef LONG_PTR(*FUNCLLLL)(LONG_PTR, LONG_PTR, LONG_PTR);
typedef LONG_PTR(*FUNCLLLLL)(LONG_PTR, LONG_PTR, LONG_PTR, LONG_PTR);
typedef LONG_PTR(*FUNCLLLLLL)(LONG_PTR, LONG_PTR, LONG_PTR, LONG_PTR, LONG_PTR);
typedef LONG_PTR(*FUNCLS)(tstring);
#else
using FUNCV = std::function<void()>;
using FUNCILPMSG = std::function<int(LPMSG)>;
using FUNCVH = std::function<void(HWND)>;
using FUNCI = std::function<int()>;
using FUNCL = std::function<LONG_PTR()>;
using FUNCLL = std::function<LONG_PTR(LONG_PTR)>;
using FUNCLLL = std::function<LONG_PTR(LONG_PTR, LONG_PTR)>;
using FUNCLLLL = std::function<LONG_PTR(LONG_PTR, LONG_PTR, LONG_PTR)>;
using FUNCLLLLL = std::function<LONG_PTR(LONG_PTR, LONG_PTR, LONG_PTR, LONG_PTR)>;
using FUNCLLLLLL = std::function<LONG_PTR(LONG_PTR, LONG_PTR, LONG_PTR, LONG_PTR, LONG_PTR)>;
using FUNCLS = std::function<LONG_PTR(tstring)>;
#endif

class Base;
class Box;
namespace  enc
{
	extern std::string w2a(const std::wstring& wideString);
	extern std::wstring a2w(const std::string& a);
	extern std::string t2a(const tstring& wideString);
	extern std::wstring t2w(const tstring& wideString);
}

#define ifReturn(CONDITION, RET)\
    if(!(CONDITION))\
    {\
        return RET;\
    }

inline void log(const tstring &str)
{
	std::wcout << str.c_str() << std::endl;
}

namespace global
{
	extern HINSTANCE instanceHandle;
	extern LONG_PTR widgetId;
	extern std::map<HWND, Base*> objMap;
	extern std::map<HWND, WNDPROC> msgProcMap;
	extern std::map<HWND, WNDPROC> widgetProcMap;
	extern HWND eventHandle;
	extern bool waitMouse;
	extern std::map<UINT_PTR, HWND> toolTip;
	int init(int *, char ***);
	int loop();
}
namespace  tim
{
	extern void __doEvents();
	extern int wait(UINT64 milliSecond);
	class timer
	{
	public:
		HANDLE hTimerQueue = 0;
		HANDLE hTimer = 0;
		WAITORTIMERCALLBACK func;
		int timerCycle;
		///<summary>创建时钟。</summary><param name = "func">响应函数。</param><param name = "timerCycle">默认1000ms做一次函数响应。</param><returns>成功:0，失败:-1。</returns>
		int CreateTimer(WAITORTIMERCALLBACK func, int timerCycle = 1000, void *param = NULL);
		///<summary>删除时钟。</summary></param><returns>成功:非0，失败:0。</returns>
		int deleteTimer();
	};
}
namespace other
{
	extern int getControlKeyState();
}
namespace WindowProc
{
	WPARAM msgLoop(HWND hwnd = NULL, HACCEL hotKeyHandle = NULL, bool ocxHotKey = false, FUNCILPMSG callback = NULL);
}
///////////////
class Base
{
public:
	Base();
	virtual ~Base();
	virtual LRESULT _windowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	bool reDraw(bool immediate = true);
	LRESULT sendMsg(UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT postMsg(UINT msg, WPARAM wparam, LPARAM lparam);

	HWND _create(HWND parentHwnd,
		DWORD exStyle,
		const tstring &className,
		const tstring &title,
		DWORD style, int x, int y, int w, int d,
		LONG_PTR hmenu, LPVOID lparam);
	static LRESULT(CALLBACK bindObjAndHwnd)(Base* objPtr, HWND hwnd);

	static void SetSubClassMsg(Base *objPtr, HWND hwnd);

	static LRESULT subClassMsg(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT subClassProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	virtual void _baseReset(HWND hwnd);
	void baseReset(HWND hwnd);
	virtual void windowCreated(HWND) {}
	static LONG_PTR assignId();

	tstring getWindowText();
	bool setWindowText(const tstring &text);
	RECT getClientRectWithoutXY();
	void move(int x, int y);
	void size(int w, int d);
	LONG addStyle(LONG style);
	LONG getStyle();
	LONG removeStyle(LONG style);
	LONG addExStyle(LONG exStyle);
	LONG getExStyle();
	LONG removeExStyle(LONG exStyle);
	void setParent(Box * box);
	RECT getRect();
	SIZE getSize();
	virtual void init(int widgetType, Box * parent, HWND thisHwnd, Base * thisPtr, int x, int y);
public:
	virtual LRESULT _OnCommand(HWND hwnd, LPARAM hwndCtl, int wNotifyCode, int wID);
	virtual LRESULT _OnNotify(HWND hwnd, HWND hwndFrom, int code, UINT_PTR idfrom, LPARAM lparam);
	virtual LRESULT _OnCtlColor(HWND hwnd, HDC hdc);
	virtual LRESULT _OnSetCursor(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	virtual LRESULT _OnPaint(HWND hwnd);
	virtual LRESULT _OnErasebkgnd(HWND hwnd, HDC hdc);
	virtual LRESULT _OnScroll(int nScrollCode, int nPos, LPARAM hwndScrollBar);
	virtual LRESULT _OnMouseMove(HWND hwnd, int x, int y);
	virtual LRESULT _OnDrawItem(WPARAM idCtl, LPARAM lParam);
	virtual LRESULT _OnMouseLeave(HWND hwnd);
	virtual LRESULT _OnLbuttonDown(HWND hwnd, LPARAM lParam);
	virtual LRESULT _OnLbuttonDblclk(HWND hwnd, LPARAM lParam);
	virtual LRESULT _OnKeyDown(HWND hwnd, LPARAM lParam);
	virtual LRESULT _OnMoving(HWND hwnd, LPARAM lParam);
	virtual LRESULT _OnMove(HWND hwnd, int left, int top);
	virtual LRESULT _EditStreamCallback(int dwCookie, int pbBuff, int cb, int pcb);
	virtual LRESULT _OnLbuttonUp(HWND hwnd, LPARAM lParam);
	virtual LRESULT _OnInvoke(int dispIdMember, int pDispParams, int pVarResult);
	virtual LRESULT _OnSize(int x, int y);
public:
	HWND _hwnd = NULL;
	Box * _parent;
	int _widgetType;
	bool _show;
	WNDPROC _wndProcOld = NULL;
	HCURSOR _mouseCursor = NULL;
	FUNCLLLL _evMouseLeftDown = NULL;
	FUNCLLLL _evMouseLeftUp = NULL;
	FUNCLLLL _evMouseDoubleClick = NULL;
	FUNCLLLL _evMouseRightDown = NULL;
	FUNCLLLL _evMouseRightUp = NULL;
	FUNCLLLL _evMouseMove = NULL;
	FUNCL _evGetFocus = NULL;
	FUNCL _evLoseFocus = NULL;
	FUNCLLL _evkeyDown = NULL;
	FUNCLLL _evkeyUp = NULL;
	FUNCLL _evChar = NULL;
	FUNCLLL _evMouseWheel = NULL;
	FUNCLS _evDropFiles = NULL;
	FUNCLLLL _mouseMoveIn = NULL;
	FUNCLLLL _mouseMoveOut = NULL;
	FUNCLLLLLL _evPaint = NULL;
	bool _isMoveIn = NULL;
	HGDIOBJ _fontHandle = NULL;
};

class WindowBase : virtual public Base
{
public:
	struct WindowMenuPage;
	typedef std::map<int, WindowBase::WindowMenuPage> MenuPageMap;
	struct WindowMenuPage
	{
		WindowMenuPage();
		HMENU _pageHandle;
		std::map<int, FUNCV> _funcMap;
		MenuPageMap _menuMap;
	};
	struct WindowMenu
	{
		WindowMenu();
		HMENU _menuHandle;
		MenuPageMap _menuMap;
	};
public:
	WindowBase();
	virtual ~WindowBase();
	HWND _createWindow(HWND parentWindow = 0, const tstring &title = _TS(""), DWORD windowStyle = 0
		, DWORD exStyle = 0, int x = 0, int y = 0, int w = 0, int d = 0, const tstring &className = _TS("WinClass"));
	void setBackColor(COLORREF color, bool immediate = true);

	COLORREF getBackColor();

	virtual void dialogCreated(HWND) {}
	virtual int returnFuc(HWND, UINT, WPARAM, LPARAM) { return 0; }
	LRESULT _handleEvents(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	virtual LRESULT _OnErasebkgnd(HWND hwnd, HDC hdc);
public:
	HBRUSH _brush = NULL;
	COLORREF _backColor = 0;

	HBITMAP _backImage = NULL;
	int _backShowMode = 0;
	HACCEL _hotKeyHwnd = NULL;
	PNOTIFYICONDATA _trayIconPtr = NULL;
	int _trayIcon = 0;
	UINT _customMsg = 0;
	FUNCL _evWillClose = NULL;
	FUNCLLL _evPositionChanged = NULL;
	FUNCLLL _evSizeChanged = NULL;
	FUNCL _evActivated = NULL;
	FUNCL _evDeActivation = NULL;
	FUNCL _evFirstActive = NULL;
	FUNCL _evShow = NULL;
	FUNCL _evHide = NULL;
	FUNCLL _evTray = NULL;
	FUNCLL _evMenu = NULL;
	int _keepTitleActive = 0;
	int _arbitraryMovement = 0;
	int _firstActiveFlag = 0;
	FUNCLLLL _hotKeyDown = NULL;
	WindowMenu _menu;
};

class WidgetBase : virtual public Base
{
public:
	WidgetBase();
	virtual ~WidgetBase();
	virtual void _baseReset(HWND hwnd);
	virtual void _widgetReset(HWND hwnd);
	HWND _createWidget(HWND parent, DWORD exStyle, const tstring &className, const tstring &title,
		DWORD style, int x, int y, int w, int d, LONG_PTR id);
	void setBackColor(COLORREF color, bool immediate);
	COLORREF getBackColor();

	virtual LRESULT _OnCtlColor(HWND hwnd, HDC hdc);
public:
	COLORREF _frontColor = 0;
	COLORREF _backColor = 0;
	HBRUSH _brush = NULL;
};

class Box : virtual public Base
{
public:
	Box();
	virtual ~Box();
	virtual void lostChild(Base * child);
	virtual void createChildInit(Base * child, int x, int y);
public:
	std::list<Base*> _childList;
};

class Window : public WindowBase, public Box
{

public:
	Window();
	virtual ~Window();
	static Window* create(Window* parent = NULL, const tstring &title = _TS(""), int x = CW_USEDEFAULT, int y = CW_USEDEFAULT, int w = 400, int d = 400, bool show = true, FUNCVH createEndFunc = NULL);
	bool wcreate(HWND parentHwnd = NULL, const tstring &title = _TS(""), FUNCVH createEndFunc = NULL,
		bool isDialog = false, int x = CW_USEDEFAULT, int y = CW_USEDEFAULT, int w = 400, int d = 400,
		DWORD windowStyle = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, DWORD exStyle = 0, bool visible = true,
		const tstring &className = _TS("window"), HACCEL hotkeyHandle = NULL, bool showInTaskBar = true, bool responseDialogMsg = false);

	static int registerWindowClass(const tstring & className, WNDPROC proc, HICON iconHandle = 0,
		UINT classStyle = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS);

	static LRESULT CALLBACK _defaultWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	virtual LRESULT _windowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	virtual LRESULT _OnCommand(HWND hwnd, LPARAM hwndCtl, int wNotifyCode, int wID);
	virtual LRESULT _OnSize(int x, int y);
	virtual LRESULT _OnLbuttonUp(HWND, LPARAM);

	int createMenuPage(int itemId, const tstring &pageTitle = _TS(""), bool isTopMenu = true);
	int createMenuPageItem(const int pageId = 0, const tstring &itemTitle = _TS(""), FUNCV callBackMouseClick = NULL);
	int findWindowMenuPage(MenuPageMap& pageMap, int pageId, MenuPageMap** parentMap, MenuPageMap::iterator& result);
	int deleteMenuPage(int itemId);
	int deleteMenuPageItem(int itemId);//RemoveMenu
	int showMenuPage(int pageId, int x = -1, int y = -1);
public:
	FUNCVH _createEndFunc = NULL;
	FUNCI _canBeClosed = NULL;

};

class GroupBox :public WidgetBase, public Box
{
public:
	GroupBox();
	virtual ~GroupBox();
	virtual void _baseReset(HWND hwnd);
	static GroupBox* create(Box *parent, const tstring &title = _TS(""), int x = 0, int y = 0, int w = 250, int d = 180, bool show = true);
	bool wcreate(HWND parent, const tstring &title = _TS(""), DWORD style = 0, int x = 0, int y = 0, int w = 80, int d = 25, DWORD exStyle = 0);
	virtual LRESULT _OnErasebkgnd(HWND hwnd, HDC hdc);
	virtual LRESULT _OnCtlColor(HWND hwnd, HDC hdc);
};

class NoteBox :public WidgetBase, public Box
{
public:
	struct NoteWidget
	{
		int pageIndex = 0;
		Base* widget = NULL;
		bool show = true;
	};
	typedef std::map<Base*, NoteWidget> NoteWidgetMap;
	struct NotePageInfo
	{
		int index;
		NoteWidgetMap childMap;
	};
	typedef std::vector<NotePageInfo> NoteWidgetMapVec;
public:
	NoteBox();
	virtual ~NoteBox();
	virtual void _baseReset(HWND hwnd);
	static NoteBox* create(Box *parent, const tstring &title = _TS(""), int x = 0, int y = 0, int w = 500, int d = 400, bool show = true);
	bool wcreate(HWND parent, const tstring &title = _TS(""), DWORD style = 0, int x = 0, int y = 0, int w = 80, int d = 25, DWORD exStyle = 0);
	bool addWidget(WidgetBase* widget, int pageIndex = 0, bool show = true, int x = 0, int y = 0);
	int getPageAmount();
	int getTabHight();
	int getTabWidth();
	int getSelectIndex();
	void refreshWidget();
	int addPage(int index = -1, const tstring &title = _TS(""), int lparam = 0, int iImage = -1);
	int removePage(int index = 0);
	int selectPage(int index = 0);
	virtual void lostChild(Base * child);
	virtual LRESULT _OnPaint(HWND hwnd);
	virtual LRESULT _OnNotify(HWND hwnd, HWND hwndFrom, int code, UINT_PTR idfrom, LPARAM lparam);
	virtual void createChildInit(Base * child, int x, int y);
	void tmpSetPageIndex(NotePageInfo& page, int index);
	virtual LRESULT _OnCtlColor(HWND hwnd, HDC hdc);
public:
	NoteWidgetMapVec _notePageVec;
	bool _hide;
public:
	FUNCV _evPageWillChange = NULL;
	FUNCV _evPageChanged = NULL;
	FUNCV _evClick = NULL;
};

class Button : public WidgetBase
{
public:
	Button();
	virtual ~Button();
	static Button* create(Box *parent, const tstring &title = _TS("button!"), int x = 0, int y = 0, int w = 60, int d = 30, bool show = true);
	int wcreate(HWND parent, const tstring &title = _TS("button"), LONG_PTR id = 0, DWORD style = 0, int x = 0, int y = 0, int w = 80, int d = 30, DWORD exStyle = 0);
	void evClick(FUNCV callback);
	virtual LRESULT _OnCommand(HWND hwnd, LPARAM hwndCtl, int wNotifyCode, int wID);
public:



    FUNCV  _evClick = NULL;
	int _imageHandle = 0;
	int _iconHandle = 0;
};
/////////////////
class Label :public WidgetBase
{
public:
	Label();
	virtual ~Label();
	static Label* create(Box *parent, const tstring &title = _TS(""), int x = 0, int y = 0, int w = 80, int d = 25, bool show = true);
	bool wcreate(HWND parent, const tstring &title = _TS(""), DWORD style = SS_CENTERIMAGE, int x = 0, int y = 0, int w = 80, int d = 25, DWORD exStyle = 0);
	void setText(const tstring &text);
	tstring getText();
};

///////////////////
class Edit :public WidgetBase
{
public:
	enum InitOption
	{
		INIT_OPTION_TEXT_LEFT = 0x0001,
		INIT_OPTION_TEXT_RIGHT = 0x0002,
		INIT_OPTION_READ_ONLY = 0x0004,
		INIT_OPTION_PASSWARD = 0x0008
	};
	enum OtherOption
	{
		OTHER_OPTION_TEXT_LEFT = 0x0001,
		OTHER_OPTION_TEXT_RIGHT = 0x0002
	};
public:
	Edit();
	virtual ~Edit();
	static Edit* create(Box * parent, const tstring & text = _TS(""), int x = 0, int y = 0, int w = 80, int d = 30, bool show = true, int option = INIT_OPTION_TEXT_LEFT);
    bool wcreate(HWND parent, const tstring &text = _TS(""), LONG_PTR id = 0, DWORD style = WS_CLIPSIBLINGS, int x = 0, int y = 0, int w = 80, int d = 25, DWORD exStyle = 0);
	virtual LRESULT _OnCommand(HWND hwnd, LPARAM hwndCtl, int wNotifyCode, int wID);
	bool setText(const tstring &text);
	tstring getText();
	void evTextChanged(FUNCV ev);
	int openOtherOption(int option = Edit::OTHER_OPTION_TEXT_LEFT);
	int closeOtherOption(int option = Edit::OTHER_OPTION_TEXT_LEFT);
public:
	FUNCV _evTextChanged = NULL;
};

///////////////////
class MultiLineEdit :public WidgetBase
{
public:
	enum InitOption
	{
		INIT_OPTION_HSCROLL = 0x0001,
		INIT_OPTION_VSCROLL = 0x0002,
		INIT_OPTION_NO_WRAP = 0x0004,
		INIT_OPTION_TEXT_LEFT = 0x0008,
		INIT_OPTION_TEXT_RIGHT = 0x0010,
		INIT_OPTION_READ_ONLY = 0x0020
	};
	enum OtherOption
	{
		OTHER_OPTION_HSCROLL = 0x0001,
		OTHER_OPTION_VSCROLL = 0x0002
	};
public:
	MultiLineEdit();
	virtual ~MultiLineEdit();
	static MultiLineEdit* create(Box * parent, const tstring & text = _TS(""), int x = 0, int y = 0, int w = 200, int d = 150, bool show = true, int option = INIT_OPTION_TEXT_LEFT);
	bool wcreate(HWND parent, const tstring &text = _TS(""), LONG_PTR id = 0, DWORD style = 0, int x = 0, int y = 0, int w = 80, int d = 25, DWORD exStyle = 0);
	virtual LRESULT _OnCommand(HWND hwnd, LPARAM hwndCtl, int wNotifyCode, int wID);
	bool setText(const tstring &text);
	tstring getText();
	void evTextChanged(FUNCV ev);
	int openOtherOption(int option);
	int closeOtherOption(int option);
public:
	FUNCV _evTextChanged = NULL;
};

class Image :public WidgetBase
{
public:
	Image();
	virtual ~Image();
	static Image* create(Box *parent, const tstring &title = _TS(""), int x = 0, int y = 0, int w = -1, int d = -1, bool show = true);
	bool wcreate(HWND parent, const tstring &title = _TS(""), DWORD style = SS_CENTERIMAGE, int x = 0, int y = 0, int w = 80, int d = 25, DWORD exStyle = 0);
	void setText(const tstring &text);
	tstring getText();
public:
	virtual LRESULT _OnPaint(HWND hwnd);
	virtual LRESULT _OnCtlColor(HWND hwnd, HDC hdc);
	virtual LRESULT _OnErasebkgnd(HWND hwnd, HDC hdc);
	virtual LRESULT _OnSize(int x, int y);
public:
	Gdiplus::Image *m_pImage;
};
////
class List :public WidgetBase
{
public:
	List();
	~List();
	static List * create(Box * parent, int column = 1, int x = 0, int y = 0, int w = 200, int d = 300, bool show = true);
	bool wcreate(HWND parent, DWORD style = 0, int x = 0, int y = 0, int w = 120, int d = 200, DWORD exStyle = 0);
	virtual LRESULT _OnNotify(HWND hwnd, HWND hwndFrom, int code, UINT_PTR idfrom, LPARAM lparam);


	int insertRow(int rowIndex = -1);
	int deleteRow(int rowIndex);
	int deleteAll();
	int getRowNum();

	int getColumnNum();

	tstring getColumnText(int columnIndex);
	int setColumnText(int columnIndex, tstring title = _TS(""));

	tstring getText(int rowIndex, int columnIndex);
	int setText(int rowIndex, int columnIndex, tstring title = _TS(""));

	int getColumnWidth(int columnIndex);
	int setColumnWidth(int columnIndex, int width);

public:
	int ___deleteColumn(int columnIndex);
	int ___insertColumn(tstring title = _TS(""), int columnIndex = -1);
public:
	void(*_evItemChanged)(int iItem, LPARAM lParam) = NULL;
	void(*_evItemChanging)(int iItem, LPARAM lParam) = NULL;
	void(*_evItemMouseIn)(int iItem) = NULL;
	void(*_evColumnClick)(int iSubItem) = NULL;
	void(*_evItemActivate)(int iItem) = NULL;
	void(*_evItemBeginEdit)(int iItem) = NULL;
	void(*_evItemEndEdit)(int iItem) = NULL;
	void(*_evItemMouseRClick)(int iItem, UINT uKeyFlags) = NULL;
	void(*_evItemMouseLClick)(int iItem, UINT uKeyFlags) = NULL;
	void(*_evItemMouseDBClick)(int iItem, UINT uKeyFlags) = NULL;
};
#endif
