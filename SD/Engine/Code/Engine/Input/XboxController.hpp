#pragma once
#include "Engine/Input/AnalogJoystick.hpp"
#include "Engine/Input/KeyButtonState.hpp"

enum XboxButtonID
{
	ERROR_STATE = -1,

	DPAD_UP,
	DPAD_DOWN,
	DPAD_LEFT,
	DPAD_RIGHT,
	START,
	BACK,
	LEFT_THUMB,
	RIGHT_THUMB,
	LEFT_SHOULDER,
	RIGHT_SHOULDER,
	BUTTON_A,
	BUTTON_B,
	BUTTON_X,
	BUTTON_Y,

	BUTTON_NUM
};

class XboxController
{
	friend class InputSystem;

public:
	XboxController();
	~XboxController();
	bool IsConnected() const;
	void SetControllerID(int id);
	int GetControllerID() const;
	AnalogJoystick const& GetLeftStick() const;
	AnalogJoystick const& GetRightStick() const;
	float GetLeftTrigger() const;
	float GetRightTrigger() const;
	KeyButtonState const& GetButton(XboxButtonID buttonID) const;
	bool IsButtonDown(XboxButtonID buttonID) const;
	bool WasButtonJustPressed(XboxButtonID buttonID) const;
	bool WasButtonJustReleased(XboxButtonID buttonID) const;

private:
	void Update();
	void Reset();
	void UpdateJoyStick(AnalogJoystick& out_joystick, short rawX, short rawY);
	void UpdateTrigger(float& out_triggerValue, unsigned char rawValue);
	void UpdateButton(XboxButtonID buttonID, unsigned short buttonFlags, unsigned short buttonFlag);

private:
	int m_id = -1;
	bool m_isConnected = false;
	float m_leftTrigger = 0.f;
	float m_rightTrigger = 0.f;
	KeyButtonState m_buttons[(int) XboxButtonID::BUTTON_NUM];
	AnalogJoystick m_leftStick;
	AnalogJoystick m_rightStick;
};