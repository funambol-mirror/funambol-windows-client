#include "WindowsDeviceConfig.h"

WindowsDeviceConfig::WindowsDeviceConfig(const WindowsDeviceConfig& wdc, DeviceConfig & c)
: dc(c)
{
    assign(wdc);
    assign(dc);
}


WindowsDeviceConfig::WindowsDeviceConfig(const WindowsDeviceConfig& wdc)
: dc(wdc.getCommonConfig())
{
    DeviceConfig::assign(wdc);
}

WindowsDeviceConfig::WindowsDeviceConfig(DeviceConfig& c)
: dc(c)
{
    DeviceConfig::assign(c);
}

WindowsDeviceConfig::~WindowsDeviceConfig()
{}
