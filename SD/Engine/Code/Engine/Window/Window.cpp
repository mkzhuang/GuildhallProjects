#include "Engine/Window/Window.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/EventSystem.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

Window* Window::s_mainWindow = nullptr;


//-----------------------------------------------------------------------------------------------
// Handles Windows (Win32) messages/events; i.e. the OS is trying to tell us something happened.
// This function is called by Windows whenever we ask it for notifications
//
// #SD1ToDo: We will move this function to a more appropriate place later on...
//
LRESULT CALLBACK WindowsMessageHandlingProcedure(HWND windowHandle, UINT wmMessageCode, WPARAM wParam, LPARAM lParam)
{
	Window* windowContext = Window::GetWindowContext();
	GUARANTEE_OR_DIE(windowContext != nullptr, "WindowContext was null.");
	InputSystem* input = windowContext->GetConfig().m_inputSystem;

	switch (wmMessageCode)
	{
		// App close requested via "X" button, or right-click "Close Window" on task bar, or "Close" from system menu, or Alt-F4
		case WM_CLOSE:
		{
			FireEvent("QuitApp");
			//return 0; // "Consumes" this message (tells Windows "okay, we handled it")
		}

		//forward character input into input system for dev console command
		case WM_CHAR:
		{
			int charCode = static_cast<int>(wParam);
			if (input)
			{
				input->HandleCharInput(charCode);
			}
			break;
		}

		//treat special left mouse-button windows message as if it were an ordinary key down
		case WM_LBUTTONDOWN:
		{
			unsigned char keyCode = KEYCODE_LEFT_MOUSE;
			bool wasConsumed = false;

			if (input)
			{
				wasConsumed = input->HandleKeyPressed(keyCode);
				if (wasConsumed)
				{
					return 0;
				}
			}
			break;
		}

		//treat special left mouse-button windows message as if it were an ordinary key up
		case WM_LBUTTONUP:
		{
			unsigned char keyCode = KEYCODE_LEFT_MOUSE;
			bool wasConsumed = false;

			if (input)
			{
				wasConsumed = input->HandleKeyReleased(keyCode);
				if (wasConsumed)
				{
					return 0;
				}
			}
			break;
		}

		//treat special middle mouse-button windows message as if it were an ordinary key down
		case WM_MBUTTONDOWN:
		{
			unsigned char keyCode = KEYCODE_MIDDLE_MOUSE;
			bool wasConsumed = false;

			if (input)
			{
				wasConsumed = input->HandleKeyPressed(keyCode);
				if (wasConsumed)
				{
					return 0;
				}
			}
			break;
		}

		//treat special middle mouse-button windows message as if it were an ordinary key up
		case WM_MBUTTONUP:
		{
			unsigned char keyCode = KEYCODE_MIDDLE_MOUSE;
			bool wasConsumed = false;

			if (input)
			{
				wasConsumed = input->HandleKeyReleased(keyCode);
				if (wasConsumed)
				{
					return 0;
				}
			}
			break;
		}

		//treat special right mouse-button windows message as if it were an ordinary key down
		case WM_RBUTTONDOWN:
		{
			unsigned char keyCode = KEYCODE_RIGHT_MOUSE;
			bool wasConsumed = false;

			if (input)
			{
				wasConsumed = input->HandleKeyPressed(keyCode);
				if (wasConsumed)
				{
					return 0;
				}
			}
			break;
		}

		//treat special right mouse-button windows message as if it were an ordinary key up
		case WM_RBUTTONUP:
		{
			unsigned char keyCode = KEYCODE_RIGHT_MOUSE;
			bool wasConsumed = false;

			if (input)
			{
				wasConsumed = input->HandleKeyReleased(keyCode);
				if (wasConsumed)
				{
					return 0;
				}
			}
			break;
		}

		case WM_MOUSEWHEEL:
		{
			auto delta = GET_WHEEL_DELTA_WPARAM(wParam);
			bool wasConsumed = false;
			if (input)
			{
				if (delta > 0)
				{
					wasConsumed = input->SetMouseWheelState(MouseWheelState::WHEEL_UP);
				}
				else if (delta < 0)
				{
					wasConsumed = input->SetMouseWheelState(MouseWheelState::WHEEL_DOWN);
				}

				if (wasConsumed)
				{
					return 0;
				}
			}
			break;
		}

		// Raw physical keyboard "key-was-just-depressed" event (case-insensitive, not translated)
		case WM_KEYDOWN:
		{
			unsigned char keyCode = (unsigned char)wParam;
			bool wasConsumed = false;

			if (input)
			{
				wasConsumed = input->HandleKeyPressed(keyCode);
				if (wasConsumed)
				{
					return 0;
				}
			}
			break;
		}

		// Raw physical keyboard "key-was-just-released" event (case-insensitive, not translated)
		case WM_KEYUP:
		{
			unsigned char keyCode = (unsigned char)wParam;
			bool wasConsumed = false;

			if (input)
			{
				wasConsumed = input->HandleKeyReleased(keyCode);
				if (wasConsumed)
				{
					return 0;
				}
			}
			break;
		}

		case WM_ACTIVATE:
		{
			if (input)
			{
				if (wParam <= 0)
				{
					input->SetMouseMode(false, false, false);
				}
			}
			break;
		}
	}

	// Send back to Windows any unhandled/unconsumed messages we want other apps to see (e.g. play/pause in music apps, etc.)
	return DefWindowProc(windowHandle, wmMessageCode, wParam, lParam);
}


Window::Window(WindowConfig const& config)
	:m_config(config)
{
	s_mainWindow = this;
}


Window::~Window()
{

}


void Window::Startup()
	
{
	CreateOSWindow();
}


void Window::BeginFrame()
{
	RunMessagePump();
}


void Window::EndFrame()
{

}


void Window::Shutdown()
{

}


WindowConfig const& Window::GetConfig() const
{
	return m_config;
}


Window* Window::GetWindowContext()
{
	return s_mainWindow;
}


Vec2 Window::GetNormalizedCursorPos() const
{
	HWND windowHandle = HWND(m_osWindowHandle);
	POINT cursorCoords;
	RECT clientRect;
	::GetCursorPos(&cursorCoords);
	::ScreenToClient(windowHandle, &cursorCoords);
	::GetClientRect(windowHandle, &clientRect);
	float cursorX = float(cursorCoords.x) / float(clientRect.right);
	float cursorY = float(cursorCoords.y) / float(clientRect.bottom);
	return Vec2(cursorX, 1.f - cursorY);
}


IntVec2 Window::GetClientDimensions() const
{
	return m_windowDimensions;
}


float Window::GetAspect() const
{
	return m_config.m_clientAspect;
}


bool Window::HasFocus() const
{
	return GetOSWindowHandle() == ::GetActiveWindow();
}


void Window::CreateOSWindow()
{
	// Define a window style/class
	HMODULE applicationInstanceHandle = ::GetModuleHandle(NULL);
	WNDCLASSEX windowClassDescription;
	memset(&windowClassDescription, 0, sizeof(windowClassDescription));
	windowClassDescription.cbSize = sizeof(windowClassDescription);
	windowClassDescription.style = CS_OWNDC; // Redraw on move, request own Display Context
	windowClassDescription.lpfnWndProc = static_cast<WNDPROC>(WindowsMessageHandlingProcedure); // Register our Windows message-handling function
	windowClassDescription.hInstance = GetModuleHandle(NULL);
	windowClassDescription.hIcon = NULL;
	windowClassDescription.hCursor = NULL;
	windowClassDescription.lpszClassName = TEXT("Simple Window Class");
	RegisterClassEx(&windowClassDescription);

	// #SD1ToDo: Add support for fullscreen mode (requires different window style flags than windowed mode)
	const DWORD windowStyleFlags = WS_CAPTION | WS_BORDER | WS_SYSMENU | WS_OVERLAPPED; //WS_THICKFRAME
	const DWORD windowStyleExFlags = WS_EX_APPWINDOW;

	// Get desktop rect, dimensions, aspect
	RECT desktopRect;
	HWND desktopWindowHandle = ::GetDesktopWindow();
	::GetClientRect(desktopWindowHandle, &desktopRect);
	float desktopWidth = (float)(desktopRect.right - desktopRect.left);
	float desktopHeight = (float)(desktopRect.bottom - desktopRect.top);
	float desktopAspect = desktopWidth / desktopHeight;
	m_windowDimensions = IntVec2(static_cast<int>(desktopWidth), static_cast<int>(desktopHeight));

	// Calculate maximum client size (as some % of desktop size)
	constexpr float maxClientFractionOfDesktop = 0.90f;
	float clientWidth = desktopWidth * maxClientFractionOfDesktop;
	float clientHeight = desktopHeight * maxClientFractionOfDesktop;
	if (m_config.m_clientAspect > desktopAspect)
	{
		// Client window has a wider aspect than desktop; shrink client height to match its width
		clientHeight = clientWidth / m_config.m_clientAspect;
	}
	else
	{
		// Client window has a taller aspect than desktop; shrink client width to match its height
		clientWidth = clientHeight * m_config.m_clientAspect;
	}

	// Calculate client rect bounds by centering the client area
	float clientMarginX = 0.5f * (desktopWidth - clientWidth);
	float clientMarginY = 0.5f * (desktopHeight - clientHeight);
	RECT clientRect;
	clientRect.left = (int)clientMarginX;
	clientRect.right = clientRect.left + (int)clientWidth;
	clientRect.top = (int)clientMarginY;
	clientRect.bottom = clientRect.top + (int)clientHeight;

	// Calculate the outer dimensions of the physical window, including frame et. al.
	RECT windowRect = clientRect;
	AdjustWindowRectEx(&windowRect, windowStyleFlags, FALSE, windowStyleExFlags);

	WCHAR windowTitle[1024];
	MultiByteToWideChar(GetACP(), 0, m_config.m_windowTitle.c_str(), -1, windowTitle, sizeof(windowTitle) / sizeof(windowTitle[0]));
	
	HWND hwnd = CreateWindowEx(
		windowStyleExFlags,
		windowClassDescription.lpszClassName,
		windowTitle,
		windowStyleFlags,
		windowRect.left,
		windowRect.top,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL,
		NULL,
		(HINSTANCE)applicationInstanceHandle,
		NULL);

	m_osWindowHandle = hwnd;
	ShowWindow(HWND(m_osWindowHandle), SW_SHOW);
	SetForegroundWindow(HWND(m_osWindowHandle));
	SetFocus(HWND(m_osWindowHandle));

	HCURSOR cursor = LoadCursor(NULL, IDC_ARROW);
	SetCursor(cursor);
}


void Window::RunMessagePump()
{
	MSG queuedMessage;
	for (;; )
	{
		const BOOL wasMessagePresent = PeekMessage(&queuedMessage, NULL, 0, 0, PM_REMOVE);
		if (!wasMessagePresent)
		{
			break;
		}

		TranslateMessage(&queuedMessage);
		DispatchMessage(&queuedMessage); // This tells Windows to call our "WindowsMessageHandlingProcedure" (a.k.a. "WinProc") function
	}
}


