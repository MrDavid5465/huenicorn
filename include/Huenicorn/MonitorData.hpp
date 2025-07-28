#pragma once

#include <string>

namespace Huenicorn
{
  struct MonitorData
  {
    MonitorData(const std::string name, unsigned width, unsigned height, double refreshRate, bool isPrimary):
    name(name),
    width(width),
    height(height),
    refreshRate(refreshRate),
    isPrimary(isPrimary)
    {}

    virtual ~MonitorData() = default;

    std::string name{""};
    unsigned width{0};
    unsigned height{0};
    double refreshRate{0};
    bool isPrimary{false};
  };
}
