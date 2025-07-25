#pragma once

namespace Huenicorn
{
  struct MonitorData
  {
    MonitorData(const std::string name, unsigned width, unsigned height):
    name(name),
    width(width),
    height(height)
    {}

    virtual ~MonitorData() = default;

    std::string name{""};
    unsigned width{0};
    unsigned height{0};
  };
}
