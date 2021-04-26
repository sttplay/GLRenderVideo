#include "Engine.h"

Engine::Engine(HINSTANCE hInstance, std::string window_class)
{
	this->hInstance = hInstance;
	this->window_class = window_class;
	this->window_class_wide = std::wstring(window_class.begin(), window_class.end());

	RegisterWindowsClass();
}

Engine::~Engine()
{
	if (!UnregisterClass(
#ifdef UNICODE
		this->window_class_wide.c_str(),
#else
		this->window_class.c_str(),
#endif // UNICODE
		
		hInstance
	))
	{
		throw;
	}
}

bool Engine::Initialize(std::string window_title, int width, int height, int nCmdShow)
{
	this->window_title = window_title;
	this->window_title_wide = std::wstring(window_title.begin(), window_title.end());
	this->width = width;
	this->height = height;
	int nWindMetricsX = GetSystemMetrics(SM_CXSCREEN);
	int nWindMetricsY = GetSystemMetrics(SM_CYSCREEN);
	RECT wr;
	wr.left = (nWindMetricsX - width) / 2;
	wr.top = (nWindMetricsY - height) / 2;
	wr.right = wr.left + width;
	wr.bottom = wr.top + height;
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
	handle = CreateWindowEx(0,
#ifdef UNICODE
		this->window_class_wide.c_str(),
		this->window_title_wide.c_str(),
#else
		this->window_class.c_str(),
		this->window_title.c_str(),
#endif // UNICODE
		WS_OVERLAPPEDWINDOW,
		wr.left,
		wr.top,
		wr.right - wr.left,
		wr.bottom - wr.top,
		NULL,
		NULL,
		hInstance,
		this
	);
	if (!handle)
		throw;
	OnStart();
	ShowWindow(handle, nCmdShow);
	UpdateWindow(handle);
	SetForegroundWindow(handle);
	SetFocus(handle);
	return true;
}

bool Engine::RegisterWindowsClass()
{
	WNDCLASSEX wcex;
	ZeroMemory(&wcex, sizeof(WNDCLASSEX));
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wcex.lpfnWndProc = HandleMessageSetup;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(DKGRAY_BRUSH);
	wcex.lpszMenuName = NULL;
	wcex.hIconSm = NULL;
#ifdef UNICODE
	wcex.lpszClassName = this->window_class_wide.c_str();
#else
	wcex.lpszClassName = this->window_class.c_str();
#endif
	if (!RegisterClassEx(&wcex))
	{
		int error = GetLastError();
		throw;
		return false;
	}
	return true;
}

bool Engine::ProcessMessage()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	if (msg.message == WM_NULL)
	{
		if (!IsWindow(handle))
			return false;
	}
	if (!handle)
		return false;
	return true;
}

void Engine::Close()
{
	SendMessage(handle, WM_CLOSE, 0, 0);
}

LRESULT Engine::HandleMessageSetup(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_NCCREATE:
	{
		const CREATESTRUCT * pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
		Engine *engine = reinterpret_cast<Engine*>(pCreate->lpCreateParams);
		if (!engine)
			throw;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(engine));
		SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(HandleMsgRedirect));
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}

LRESULT Engine::HandleMsgRedirect(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	Engine *engine = reinterpret_cast<Engine*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	return engine->WinProc(hwnd, uMsg, wParam, lParam);
}

LRESULT Engine::WinProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
		DestroyWindow(hwnd);
		this->handle = NULL;
		OnClose();
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void Engine::OnStart()
{

}

void Engine::OnClose()
{

}
