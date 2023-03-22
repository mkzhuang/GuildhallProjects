#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Core/DevConsole.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
unsigned char const KEYCODE_F1				= VK_F1;
unsigned char const KEYCODE_F2				= VK_F2;
unsigned char const KEYCODE_F3				= VK_F3;
unsigned char const KEYCODE_F4				= VK_F4;
unsigned char const KEYCODE_F5				= VK_F5;
unsigned char const KEYCODE_F6				= VK_F6;
unsigned char const KEYCODE_F7				= VK_F7;
unsigned char const KEYCODE_F8				= VK_F8;
unsigned char const KEYCODE_F9				= VK_F9;
unsigned char const KEYCODE_F10				= VK_F10;
unsigned char const KEYCODE_F11				= VK_F11;
unsigned char const KEYCODE_ESC				= VK_ESCAPE;
unsigned char const KEYCODE_TILDE			= VK_OEM_3;
unsigned char const KEYCODE_UPARROW			= VK_UP;
unsigned char const KEYCODE_DOWNARROW		= VK_DOWN;
unsigned char const KEYCODE_LEFTARROW		= VK_LEFT;
unsigned char const KEYCODE_RIGHTARROW		= VK_RIGHT;
unsigned char const KEYCODE_SPACE			= VK_SPACE;
unsigned char const KEYCODE_ENTER			= VK_RETURN;
unsigned char const KEYCODE_HOME			= VK_HOME;
unsigned char const KEYCODE_END				= VK_END;
unsigned char const KEYCODE_DELETE			= VK_DELETE;
unsigned char const KEYCODE_BACKSPACE		= VK_BACK;
unsigned char const KEYCODE_SHIFT			= VK_SHIFT;
unsigned char const KEYCODE_LEFT_MOUSE		= VK_LBUTTON;
unsigned char const KEYCODE_RIGHT_MOUSE		= VK_RBUTTON;
unsigned char const KEYCODE_MIDDLE_MOUSE	= VK_MBUTTON;
unsigned char const KEYCODE_COMMA			= VK_OEM_COMMA;
unsigned char const KEYCODE_PERIOD			= VK_OEM_PERIOD;
//unsigned char const KEYCODE_TILDE			= 0xC0;
//unsigned char const KEYCODE_LEFTBRACKET	= 0xDB;
//unsigned char const KEYCODE_RIGHTBRACKET	= 0xDD;

InputSystem* g_theInput;

InputSystem::InputSystem(InputSystemConfig const& config)
	:m_config(config)
{
	
}


InputSystem::~InputSystem()
{

}


void InputSystem::Startup()
{
	for (int controllerIndex = 0; controllerIndex < NUM_XBOX_CONTROLLERS; controllerIndex++)
	{
		m_controllers[controllerIndex].SetControllerID(controllerIndex);
	}
	SetMouseMode(false, false, false);
}


void InputSystem::BeginFrame()
{
	for (int controllerIndex = 0; controllerIndex < NUM_XBOX_CONTROLLERS; controllerIndex++)
	{
		m_controllers[controllerIndex].Update();
	}
}


void InputSystem::EndFrame()
{
	for (int keyIndex = 0; keyIndex < NUM_KEYCODES; keyIndex++)
	{
		m_keyStates[keyIndex].m_wasPressedLastFrame = m_keyStates[keyIndex].m_isDownThisFrame;
	}
	SetMouseWheelState(MouseWheelState::WHEEL_IDLE);
}

void InputSystem::ShutDown()
{

}


InputSystemConfig const& InputSystem::GetConfig() const
{
	return m_config;
}


bool InputSystem::WasKeyJustPressed(unsigned char keyCode)
{
	KeyButtonState keyState = m_keyStates[keyCode];
	return keyState.m_isDownThisFrame && !keyState.m_wasPressedLastFrame;
}


bool InputSystem::WasKeyJustReleased(unsigned char keyCode)
{
	KeyButtonState keyState = m_keyStates[keyCode];
	return !keyState.m_isDownThisFrame && keyState.m_wasPressedLastFrame;
}


bool InputSystem::IsKeyDown(unsigned char keyCode)
{
	return m_keyStates[keyCode].m_isDownThisFrame;
}


bool InputSystem::HandleKeyPressed(unsigned char keyCode)
{
	m_keyStates[keyCode].m_isDownThisFrame = true;
	EventArgs args;
	args.SetValue("key", keyCode);
	FireEvent("KeyPressed", args);
	return true;
}


bool InputSystem::HandleKeyReleased(unsigned char keyCode)
{
	m_keyStates[keyCode].m_isDownThisFrame = false;
	return true;
}


bool InputSystem::HandleCharInput(int charCode)
{
	EventArgs args;
	args.SetValue("key", charCode);
	FireEvent("CharInput", args);

	if (charCode == 22) // charCode for paste
	{
		HANDLE clip;
		std::string clipboardString;
		if (::OpenClipboard((HWND)Window::GetWindowContext()->GetOSWindowHandle()))
		{
			clip = GetClipboardData(CF_TEXT);
			char* clipboardText = (char*)GlobalLock(clip);
			
			if (clipboardText)
			{
				clipboardString = std::string(clipboardText);
			}
			GlobalUnlock(clip);
			CloseClipboard();
		}
		EventArgs clipboardArgs;
		clipboardArgs.SetValue("clipboard", clipboardString);
		FireEvent("Paste", clipboardArgs);
	}
	return true;
}


void InputSystem::ConsumeKeyJustPressed(unsigned char keyCode)
{
	m_keyStates[keyCode].m_isDownThisFrame = false;
	m_keyStates[keyCode].m_wasPressedLastFrame = false;
}


void InputSystem::ConsumeAllKeys()
{
	for (int keyCode = 0; keyCode < NUM_KEYCODES; keyCode++)
	{
		ConsumeKeyJustPressed(static_cast<unsigned char>(keyCode));
	}
}


void InputSystem::ToggleHidden()
{
	SetMouseMode(!m_isHidden, m_isClipped, m_isRelative);
}


void InputSystem::ToggleClipped()
{
	SetMouseMode(m_isHidden, !m_isClipped, m_isRelative);
}


void InputSystem::ToggleRelative()
{
	SetMouseMode(m_isHidden, m_isClipped, !m_isRelative);
}


void InputSystem::SetMouseMode(bool isHidden, bool isClipped, bool isRelative)
{
	m_isHidden = isHidden;
	m_isClipped = isClipped;
	m_isRelative = isRelative;

	HWND windowHandle = (HWND) Window::GetWindowContext()->GetOSWindowHandle();
	RECT clientRect;
	::GetClientRect(windowHandle, &clientRect);
	int centerX = (clientRect.left + clientRect.right) / 2;
	int centerY = (clientRect.bottom + clientRect.top) / 2;
	POINT clientCenter;
	clientCenter.x = centerX;
	clientCenter.y = centerY;
	::ClientToScreen(windowHandle, &clientCenter);
	m_mouseState.m_clientCenter = Vec2(static_cast<float>(clientCenter.x), static_cast<float>(clientCenter.y));
	POINT clientOrigin;
	clientOrigin.x = clientRect.left;
	clientOrigin.y = clientRect.top;
	::ClientToScreen(windowHandle, &clientOrigin);
	m_mouseState.m_clientOrgin = Vec2(static_cast<float>(clientOrigin.x), static_cast<float>(clientOrigin.y));

	POINT cursorCoords;
	::GetCursorPos(&cursorCoords);
	m_mouseState.m_mousePosition = Vec2(static_cast<float>(cursorCoords.x), static_cast<float>(cursorCoords.y));

	// mouse hidden state
	if (m_isHidden)
	{
		while (::ShowCursor(FALSE) >= 0)
		{
			::ShowCursor(FALSE);
		}
	}
	else
	{
		while (::ShowCursor(TRUE) < 0)
		{
			::ShowCursor(TRUE);
		}
	}

	if (m_isClipped || m_isRelative)
	{
		clientRect.left += clientOrigin.x;
		clientRect.right += clientOrigin.x;
		clientRect.bottom += clientOrigin.y;
		clientRect.top += clientOrigin.y;
		::ClipCursor(&clientRect);
	}
	else
	{
		::ClipCursor(NULL);
	}

	// relative cursor
	Window* window = Window::GetWindowContext();
	if (m_isRelative && window != nullptr && window->HasFocus())
	{
		m_mouseState.m_prevMousePosition = m_mouseState.m_mousePosition;
		::SetCursorPos((int)m_mouseState.m_clientCenter.x, (int)m_mouseState.m_clientCenter.y);
		m_mouseState.m_mousePosition = m_mouseState.m_clientCenter;
		if (m_isDeltaIgnored)
		{
			m_mouseState.m_mousePosition = m_mouseState.m_prevMousePosition;
			m_isDeltaIgnored = false;
		}
	}
	else
	{
		m_isDeltaIgnored = true;
		m_mouseState.m_prevMousePosition = m_mouseState.m_mousePosition;
	}
}


MouseWheelState InputSystem::GetMouseWheelState() const
{
	return m_mouseWheelState;
}


bool InputSystem::SetMouseWheelState(MouseWheelState state)
{
	m_mouseWheelState = state;
	return true;
}


Vec2 const InputSystem::GetMouseClientPosition()
{
	return m_mouseState.m_mousePosition - m_mouseState.m_clientOrgin;
}


Vec2 const InputSystem::GetMouseClientDelta()
{
	return m_mouseState.m_mousePosition - m_mouseState.m_prevMousePosition;
}


XboxController const& InputSystem::GetController(int controllerID)
{
	return m_controllers[controllerID];
}


