#pragma once

#include <stdint.h>

#include "_Config.h"

#include "ControlCenter.h"
#include "Tunnel.h"
#include "TrackSection.h"

class SystemController
{
public:
	SystemController();
	~SystemController();

	void Setup();

	void Update();

private:

	void SetupControlCenter();
	void SetupTunnel();
	void SetupTrackSections();

	void OnControlCenterEvent(ControlCenterEvent eventType, const ControlCenterEventInfo& buttonEventInfo);
	static void ControlCenterEventThunk(void* context, ControlCenterEvent eventType, const ControlCenterEventInfo& buttonEventInfo);

	void OnTunnelEvent(const Tunnel::TunnelEventInfo& eventInfo);
	static void TunnelEventThunk(void* context, const Tunnel::TunnelEventInfo& eventInfo);

	void OnTrackSectionEvent(const TrackSectionEventInfo& info);
	static void TrackSectionEventThunk(void* context, const TrackSectionEventInfo& info);

	ControlCenter m_controlCenter;
	Tunnel m_tunnel;
	TrackSection m_trackSections[config::track_sections::MAX_TRACK_SECTIONS];
};