#pragma once
#include "Engine/Input/XboxController.hpp"

extern unsigned char const KEYCODE_F1;
extern unsigned char const KEYCODE_F2;
extern unsigned char const KEYCODE_F3;
extern unsigned char const KEYCODE_F4;
extern unsigned char const KEYCODE_F5;
extern unsigned char const KEYCODE_F6;
extern unsigned char const KEYCODE_F7;
extern unsigned char const KEYCODE_F8;
extern unsigned char const KEYCODE_F9;
extern unsigned char const KEYCODE_F10;
extern unsigned char const KEYCODE_F11;
extern unsigned char const KEYCODE_ESC;
extern unsigned char const KEYCODE_TILDE;
extern unsigned char const KEYCODE_UPARROW;
extern unsigned char const KEYCODE_DOWNARROW;
extern unsigned char const KEYCODE_LEFTARROW;
extern unsigned char const KEYCODE_RIGHTARROW;
extern unsigned char const KEYCODE_SPACE;
extern unsigned char const KEYCODE_ENTER;
extern unsigned char const KEYCODE_HOME;
extern unsigned char const KEYCODE_END;
extern unsigned char const KEYCODE_DELETE;
extern unsigned char const KEYCODE_BACKSPACE;
extern unsigned char const KEYCODE_SHIFT;
extern unsigned char const KEYCODE_LEFT_MOUSE;
extern unsigned char const KEYCODE_RIGHT_MOUSE;
extern unsigned char const KEYCODE_MIDDLE_MOUSE;
extern unsigned char const KEYCODE_COMMA;
extern unsigned char const KEYCODE_PERIOD;

constexpr int NUM_KEYCODES = 256;
constexpr int NUM_XBOX_CONTROLLERS = 4;

enum class MouseWheelState
{
	WHEEL_DOWN = -1,
	WHEEL_IDLE = 0,
	WHEEL_UP = 1,
};

struct MouseState
{
	Vec2 m_mousePosition;
	Vec2 m_prevMousePosition;
	Vec2 m_clientCenter;
	Vec2 m_clientOrgin;
};

struct InputSystemConfig
{
};

class InputSystem
{
public:
	InputSystem(InputSystemConfig const& config);
	~InputSystem();
	void Startup();
	void BeginFrame();
	void EndFrame();
	void ShutDown();

	InputSystemConfig const& GetConfig() const;
	bool WasKeyJustPressed(unsigned char keyCode);
	bool WasKeyJustReleased(unsigned char keyCode);
	bool IsKeyDown(unsigned char keyCode);
	bool HandleKeyPressed(unsigned char keyCode);
	bool HandleKeyReleased(unsigned char keyCode);
	bool HandleCharInput(int charCode);
	void ConsumeKeyJustPressed(unsigned char keyCode);
	void ConsumeAllKeys();
	void ToggleHidden();
	void ToggleClipped();
	void ToggleRelative();
	void SetMouseMode(bool isHidden, bool isClipped, bool isRelative);
	MouseWheelState GetMouseWheelState() const;
	bool SetMouseWheelState(MouseWheelState state);
	Vec2 const GetMouseClientPosition();
	Vec2 const GetMouseClientDelta();
	XboxController const& GetController(int controllerID);

protected:
	InputSystemConfig m_config;
	KeyButtonState m_keyStates[NUM_KEYCODES];
	XboxController m_controllers[NUM_XBOX_CONTROLLERS];
	bool m_isHidden = false;
	bool m_isClipped = false;
	bool m_isRelative = false;
	bool m_isDeltaIgnored = false;
	MouseState m_mouseState;
	MouseWheelState m_mouseWheelState = MouseWheelState::WHEEL_IDLE;
};