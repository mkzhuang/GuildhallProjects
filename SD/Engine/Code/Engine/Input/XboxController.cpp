#include "Engine/Input/XboxController.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h> 
#include <Xinput.h>

#pragma comment( lib, "xinput9_1_0" ) 

XboxController::XboxController()
{

}


XboxController::~XboxController()
{

}


bool XboxController::IsConnected() const
{
	return m_isConnected;
}


void XboxController::SetControllerID(int id)
{
	m_id = id;
}


int XboxController::GetControllerID() const
{
	return m_id;
}


AnalogJoystick const& XboxController::GetLeftStick() const
{
	return m_leftStick;
}


AnalogJoystick const& XboxController::GetRightStick() const
{
	return m_rightStick;
}


float XboxController::GetLeftTrigger() const
{
	return m_leftTrigger;
}


float XboxController::GetRightTrigger() const
{
	return m_rightTrigger;
}


KeyButtonState const& XboxController::GetButton(XboxButtonID buttonID) const
{
	return m_buttons[(int)buttonID];
}


bool XboxController::IsButtonDown(XboxButtonID buttonID) const
{
	return m_buttons[(int)buttonID].m_isDownThisFrame;
}


bool XboxController::WasButtonJustPressed(XboxButtonID buttonID) const
{
	KeyButtonState button = m_buttons[(int)buttonID];
	return button.m_isDownThisFrame && !button.m_wasPressedLastFrame;
}


bool XboxController::WasButtonJustReleased(XboxButtonID buttonID) const
{
	KeyButtonState button = m_buttons[(int)buttonID];
	return !button.m_isDownThisFrame && button.m_wasPressedLastFrame;
}


void XboxController::Update()
{
	XINPUT_STATE xboxControllerState;
	DWORD errorStatus = XInputGetState(m_id, &xboxControllerState);
	if (errorStatus == ERROR_SUCCESS)
	{
		m_isConnected = true;

		XINPUT_GAMEPAD controller = xboxControllerState.Gamepad;
		UpdateJoyStick(m_leftStick, controller.sThumbLX, controller.sThumbLY);
		UpdateJoyStick(m_rightStick, controller.sThumbRX, controller.sThumbRY);

		UpdateTrigger(m_leftTrigger, controller.bLeftTrigger);
		UpdateTrigger(m_rightTrigger, controller.bRightTrigger);

		UpdateButton(XboxButtonID::DPAD_UP, controller.wButtons, XINPUT_GAMEPAD_DPAD_UP);
		UpdateButton(XboxButtonID::DPAD_DOWN, controller.wButtons, XINPUT_GAMEPAD_DPAD_DOWN);
		UpdateButton(XboxButtonID::DPAD_LEFT, controller.wButtons, XINPUT_GAMEPAD_DPAD_LEFT);
		UpdateButton(XboxButtonID::DPAD_RIGHT, controller.wButtons, XINPUT_GAMEPAD_DPAD_RIGHT);
		UpdateButton(XboxButtonID::START, controller.wButtons, XINPUT_GAMEPAD_START);
		UpdateButton(XboxButtonID::BACK, controller.wButtons, XINPUT_GAMEPAD_BACK);
		UpdateButton(XboxButtonID::LEFT_THUMB, controller.wButtons, XINPUT_GAMEPAD_LEFT_THUMB);
		UpdateButton(XboxButtonID::RIGHT_THUMB, controller.wButtons, XINPUT_GAMEPAD_RIGHT_THUMB);
		UpdateButton(XboxButtonID::LEFT_SHOULDER, controller.wButtons, XINPUT_GAMEPAD_LEFT_SHOULDER);
		UpdateButton(XboxButtonID::RIGHT_SHOULDER, controller.wButtons, XINPUT_GAMEPAD_RIGHT_SHOULDER);
		UpdateButton(XboxButtonID::BUTTON_A, controller.wButtons, XINPUT_GAMEPAD_A);
		UpdateButton(XboxButtonID::BUTTON_B, controller.wButtons, XINPUT_GAMEPAD_B);
		UpdateButton(XboxButtonID::BUTTON_X, controller.wButtons, XINPUT_GAMEPAD_X);
		UpdateButton(XboxButtonID::BUTTON_Y, controller.wButtons, XINPUT_GAMEPAD_Y);
	}
	else {
		Reset();
	}
}


void XboxController::Reset()
{
	m_isConnected = false;
	m_leftTrigger = 0.f;
	m_rightTrigger = 0.f;
	m_leftStick.Reset();
	m_rightStick.Reset();
}


void XboxController::UpdateJoyStick(AnalogJoystick& out_joystick, short rawX, short rawY)
{
	float x = RangeMap(rawX, SHORT_MIN, SHORT_MAX, -1.f, 1.f);
	float y = RangeMap(rawY, SHORT_MIN, SHORT_MAX, -1.f, 1.f);
	out_joystick.UpdatePosition(x, y);
}


void XboxController::UpdateTrigger(float& out_triggerValue, unsigned char rawValue)
{
	out_triggerValue = RangeMap(rawValue, UNSIGNED_CHAR_MIN, UNSIGNED_CHAR_MAX, 0.f, 1.f);
}


void XboxController::UpdateButton(XboxButtonID buttonID, unsigned short buttonFlags, unsigned short buttonFlag)
{
	KeyButtonState& button = m_buttons[(int) buttonID];
	button.m_wasPressedLastFrame = button.m_isDownThisFrame;
	button.m_isDownThisFrame = ((buttonFlags & buttonFlag) == buttonFlag);
}


