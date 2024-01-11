#pragma once

#include <stdint.h>

#include "ControlCenter.h"
#include "Tunnel.h"

class Train
	: public IControlCenterObserver
	, public Tunnel::ITunnelObserver
{
public:
	Train();
	~Train();

	void SetupControlCenter();
	void SetupTunnel();

	void Update();

	// Control Center Observer Interface
	void OnSwitchLightsOn() override;
	void OnSwitchLightsOff() override;

	void OnLightsOverrideChanged(bool areLightsOverridden) override;
	void OnLightSensitivityChanged(unsigned short int lightDifferenceThreshold) override;
	void OnChangeTrainDetectionMode() override;

	// Tunnel Observer Interface
	void OnTunnelEvent(const Tunnel::TunnelEventInfo& eventInfo) override;

private:
	ControlCenter* m_controlCenter;
	Tunnel* m_tunnel;
};