#pragma once

#include <stdint.h>

#include "Button.h"
#include "Throttle.h"

class IControlCenterObserver
{
public:
	IControlCenterObserver() {};

	virtual void OnSwitchLightsOn() = 0;
	virtual void OnSwitchLightsOff() = 0;

	virtual void OnSwitchTunnelDetectionOn() = 0;
	virtual void OnSwitchTunnelDetectionOff() = 0;

	virtual void OnLightsOverrideChanged(bool areLightsOverridden) = 0;
	virtual void OnLightSensitivityChanged(unsigned short int lightDifferenceThreshold) = 0;
	virtual void OnChangeTrainDetectionMode() = 0;
};


class ControlCenter
	: public IButtonObserver
	, public IThrottleObserver
{
public:
	ControlCenter();
	~ControlCenter();

	void Update();

	void RegisterObserver(IControlCenterObserver* observer);

private:

	// Button Interface
	void OnButtonEvent(uint32_t buttonId, const ButtonEventInfo& buttonEventInfo) override final;

	// Throttle Interface
	void OnThrottleChange(uint32_t speed, uint32_t throttleId) override final;

private:
	Button*	m_buttonSettings;
	Button* m_buttonEnableTunnelDetection;
	Throttle* m_throttle;

	IControlCenterObserver*	m_observer;

	bool m_areLightsOverridden;
	bool m_isTunnelDetectionOn;
};

