#pragma once

#include <stdint.h>

#include "Button.h"
#include "Tunnel.h"

enum class ControlCenterEvent : uint8_t
{
    TunnelLightDetectionChanged,
    TunnelTrainDetectionChanged,
    TestButtonClick
};

struct ControlCenterEventInfo
{
    Tunnel::TUNNEL_LIGHT_MODE tunnelLightMode;
    Tunnel::TRAIN_DETECTION_MODE tunnelTrainDetectionMode;
};

using ControlCenterHandler = void(*)(void* context, ControlCenterEvent eventType, const ControlCenterEventInfo& controlCenterEventInfo);

class ControlCenter
//: public IButtonObserver
{
public:
    ControlCenter();
    ~ControlCenter();

    void Setup();
    void SetListener(ControlCenterHandler handler, void* context);

    void Update();

private:
    // Button Interface
    void OnButtonEvent(uint32_t buttonId, const ButtonEventInfo& buttonEventInfo);
    static void ButtonEventThunk(void* context, uint8_t buttonId, const ButtonEventInfo& buttonEventInfo);

private:
    //Button m_buttonSettings;
    //Button m_buttonEnableTunnelDetection;
    Button m_buttonTest;

    void* m_context = nullptr;
    ControlCenterHandler m_handler = nullptr;
};