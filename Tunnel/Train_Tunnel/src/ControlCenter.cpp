#include <Arduino.h>

#include "ControlCenter.h"

#include "_Config.h"

#include "macro-logger/MacroLogger.h"

//const uint8_t k_pin_button_settings = 2;
//const uint8_t k_pin_button_toggle_tunnel_detection = 5;
const uint8_t k_pin_button_test = config::control_center::PIN_BUTTON_TEST;

ControlCenter::ControlCenter()
//: m_buttonSettings(k_pin_button_settings)
//, m_buttonEnableTunnelDetection(k_pin_button_toggle_tunnel_detection)
    : m_buttonTest(k_pin_button_test)
{
}

ControlCenter::~ControlCenter()
{

}

void ControlCenter::Setup()
{
    ButtonSettings settings;
    
    /*
    settings.enabledEvents = BUTTON_EVENT_MASK::EVENT_SWITCH | BUTTON_EVENT_MASK::EVENT_CLICKS | BUTTON_EVENT_MASK::EVENT_HOLD_RELEASE;
    m_buttonSettings.SetSettings(settings);
    //*/

    /*
    settings.enabledEvents = BUTTON_EVENT_MASK::EVENT_SWITCH;
    m_buttonEnableTunnelDetection.SetSettings(settings);
    //*/

    settings.enabledEvents = BUTTON_EVENT_MASK::EVENT_CLICKS;
    m_buttonTest.SetSettings(settings);
    
    //m_buttonSettings.SetListener(&ControlCenter::ButtonEventThunk, this);
    //m_buttonEnableTunnelDetection.SetListener(&ControlCenter::ButtonEventThunk, this);
    m_buttonTest.SetListener(&ControlCenter::ButtonEventThunk, this);
}

void ControlCenter::SetListener(ControlCenterHandler handler, void* context)
{
    m_handler = handler;
    m_context = context;
}

void ControlCenter::Update()
{
    //m_buttonSettings.Update();
    //m_buttonEnableTunnelDetection.Update();
    m_buttonTest.Update();
}

void ControlCenter::OnButtonEvent(uint32_t buttonId, const ButtonEventInfo& buttonEventInfo)
{
    char buttonEventTypeBuffer[24]; // adjust size if needed
    strncpy_P(buttonEventTypeBuffer, (PGM_P)ButtonEventTypeToText(buttonEventInfo.m_buttonEventType), sizeof(buttonEventTypeBuffer));
    buttonEventTypeBuffer[sizeof(buttonEventTypeBuffer) - 1] = '\0';

    //LOG_INFO("Button event: %s, Id: %d, clicks: %d", buttonEventTypeBuffer, buttonId, buttonEventInfo.m_numberOfClicks);
    LOG_INFO("Button event: %s, Id: %" PRIu32 ", clicks: %u",
        buttonEventTypeBuffer,
        buttonId,
        (unsigned int)buttonEventInfo.m_numberOfClicks);

    switch (buttonEventInfo.m_buttonEventType)
    {
    case BUTTON_EVENT_TYPE::BUTTON_SWITCH:
    {
        /*
        if (buttonId == m_buttonSettings.GetPin())
        {
            ControlCenterEventInfo controlCenterEventInfo;
            m_handler(m_context, ControlCenterEvent::TunnelLightDetectionChanged, controlCenterEventInfo);            
        }
        else if (buttonId == m_buttonEnableTunnelDetection.GetPin())
        {
            ControlCenterEventInfo controlCenterEventInfo;
            m_handler(m_context, ControlCenterEvent::TunnelTrainDetectionChanged, controlCenterEventInfo);
        }
        //*/
        break;
    }
    case BUTTON_EVENT_TYPE::BUTTON_CLICKS:
    {
        if (buttonId == m_buttonTest.GetPin())
        {
            if (buttonEventInfo.m_numberOfClicks == 1 || buttonEventInfo.m_numberOfClicks == 2)
            {
                ControlCenterEventInfo controlCenterEventInfo;
                m_handler(m_context, ControlCenterEvent::TestButtonClick, controlCenterEventInfo);
            }
        }
        break;
    }
    };
    /*
    Serial.print(F("Button event: "));
    PrintButtonEventType(buttonEventInfo.m_buttonEventType);
    Serial.print(F(" ID: "));
    Serial.print(buttonId);
    Serial.print(F(" clicks: "));
    Serial.println(buttonEventInfo.m_numberOfClicks);
    //*/
}

void ControlCenter::ButtonEventThunk(void* context, uint8_t buttonId, const ButtonEventInfo& buttonEventInfo)
{
    static_cast<ControlCenter*>(context)->OnButtonEvent(buttonId, buttonEventInfo);
}
