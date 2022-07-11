#include "Window.h"
#include <assert.h>

LRESULT WINAPI Window::WndMsgSetup(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (Msg == WM_NCCREATE)
	{
		const CREATESTRUCT* pCreateStruct = reinterpret_cast<CREATESTRUCT*>(lParam);
		Window const* pWindow = static_cast<Window*>(pCreateStruct->lpCreateParams);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWindow));
		SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndMsgForward));
	}
	return DefWindowProc(hWnd, Msg, wParam, lParam);
}

LRESULT WINAPI Window::WndMsgForward(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	Window* pWindow = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	return pWindow->WindowMessageProceedure(hWnd, Msg, wParam, lParam);
}

LRESULT Window::WindowMessageProceedure(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg)
	{
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		{
			if (kbd.AutorepeatIsEnabled() || !(lParam & 0x40000000))
			{
				kbd.OnKeyDown((unsigned char)wParam);
			}
			break;
		}
		case WM_SYSKEYUP:
		case WM_KEYUP:
		{
			kbd.OnKeyUp((unsigned char)wParam);
			break;
		}
		case WM_CHAR:
		{
			kbd.OnChar((unsigned char)wParam);
			break;
		}
		case WM_MOUSEMOVE:
		{
			const auto coords = MAKEPOINTS(lParam);
			if (coords.x >= 0 && coords.x < (int)width && coords.y >= 0 && coords.y < (int)height)
			{
				if (!mouse.IsInWindow())
				{
					mouse.OnMouseEnter();
					SetCapture(hWnd);
				}
				mouse.OnMouseMove(coords.x, coords.y);
			}
			else
			{
				if (mouse.LeftIsClicked() || mouse.RightIsClicked())
				{
					mouse.OnMouseMove(coords.x, coords.y);
				}
				else
				{
					mouse.OnMouseLeave();
					ReleaseCapture();
				}
			}
			break;
		}
		case WM_LBUTTONDOWN:
		{
			const auto coords = MAKEPOINTS(lParam);
			mouse.OnLeftClick(coords.x, coords.y);
			break;
		}
		case WM_LBUTTONDBLCLK:
		{
			const auto coords = MAKEPOINTS(lParam);
			mouse.OnLeftDoubleClick(coords.x, coords.y);
			break;
		}
		case WM_LBUTTONUP:
		{
			const auto coords = MAKEPOINTS(lParam);
			mouse.OnLeftRelease(coords.x, coords.y);
			break;
		}
		case WM_RBUTTONDOWN:
		{
			const auto coords = MAKEPOINTS(lParam);
			mouse.OnRightClick(coords.x, coords.y);
			break;
		}
		case WM_RBUTTONDBLCLK:
		{
			const auto coords = MAKEPOINTS(lParam);
			mouse.OnRightDoubleClick(coords.x, coords.y);
			break;
		}
		case WM_RBUTTONUP:
		{
			const auto coords = MAKEPOINTS(lParam);
			mouse.OnRightRelease(coords.x, coords.y);
			break;
		}
		case WM_MBUTTONDOWN:
		{
			const auto coords = MAKEPOINTS(lParam);
			mouse.OnMiddleClick(coords.x, coords.y);
			break;
		}
		case WM_MBUTTONUP:
		{
			const auto coords = MAKEPOINTS(lParam);
			mouse.OnMiddleRelease(coords.x, coords.y);
			break;
		}
		case WM_MOUSEWHEEL:
		{
			const auto z = GET_WHEEL_DELTA_WPARAM(wParam);
			mouse.OnWheelScroll(z);
			break;
		}
		case WM_KILLFOCUS:
		{
			kbd.ClearKeystates();
			break;
		}
		case WM_CLOSE:
		{
			PostQuitMessage(0);
			return 0;
		}
	}
	return DefWindowProc(hWnd, Msg, wParam, lParam);
}

Window::Window(unsigned int w, unsigned int h, std::string _title, std::vector<uint2> display_layer_dims)
	:
	width(w),
	height(h),
	title(_title),
	kbd(),
	mouse()
{
	RECT window = {};
	window.left = 0u;
	window.right = width;
	window.top = 0u;
	window.bottom = height;
	if (AdjustWindowRect(&window, WS_MINIMIZEBOX | WS_SYSMENU | WS_CAPTION, FALSE) == 0)
	{
		throw WNDEXCPT_NOTE("Failed to adjust window dimensions!");
	}
	hWnd = CreateWindowEx(
		0u, wndcls.GetName(), title.c_str(), WS_MINIMIZEBOX | WS_SYSMENU | WS_CAPTION, CW_USEDEFAULT, CW_USEDEFAULT,
		window.right - window.left, window.bottom - window.top, nullptr, nullptr, wndcls.GetInstance(), this);
	if (!hWnd)
	{
		throw WNDEXCPT_NOTE("Failed to create window!");
	}
	ShowWindow(hWnd, SW_SHOW);
	assert(!display_layer_dims.empty());
#ifdef _DEBUG
	for (unsigned int i = 0u; i < display_layer_dims.size(); ++i)
	{
		assert(display_layer_dims[i].x % 32u == 0u);
	}
#endif
	pGFX = std::make_unique<Graphics>(hWnd, width, height, display_layer_dims);
}

const unsigned int& Window::GetWidth() const
{
	return width;
}

const unsigned int& Window::GetHeight() const
{
	return height;
}

std::string Window::GetWindowTitle() const
{
	return title;
}

void Window::SetWindowTitle(const std::string& _title)
{
	title = _title;
	SetWindowText(hWnd, title.c_str());
}

Window::~Window()
{
	DestroyWindow(hWnd);
}

std::optional<int> Window::ProcessMessages()
{
	MSG msg;
	if (PeekMessage(&msg, nullptr, 0u, 0u, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
		{
			return (int)msg.wParam;
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return std::optional<int>();
}
