#pragma once

#include <stdint.h>

#include "ControlCenter.h"
#include "Tunnel.h"

#include "HBridge.h"

typedef HBridgeMotorController TrackSection;

class Train
	: public IControlCenterObserver
	, public Tunnel::ITunnelObserver
{
public:
	Train();
	~Train();

	void SetupControlCenter();
	void SetupTunnel();

	void SetupTrackSections();

	void Update();

	// Control Center Observer Interface
	void OnSwitchLightsOn() override;
	void OnSwitchLightsOff() override;

	void OnSwitchTunnelDetectionOn() override;
	void OnSwitchTunnelDetectionOff() override;

	void OnLightsOverrideChanged(bool areLightsOverridden) override;
	//void OnLightSensitivityChanged(unsigned short int lightDifferenceThreshold) override;
	void OnChangeTrainDetectionMode() override;

	void OnTestButtonClick() override;

	// Tunnel Observer Interface
	void OnTunnelEvent(const Tunnel::TunnelEventInfo& eventInfo) override;

private:
	ControlCenter* m_controlCenter;
	Tunnel* m_tunnel;

	unsigned short int lastLightDifferenceThresholdPrinted = 0;

	TrackSection m_trackSection1;
};