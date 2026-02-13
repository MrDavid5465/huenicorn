#pragma once

#include <string>
#include <unordered_map>


namespace Huenicorn::Hue::Api
{
  /**
   * @brief Device data structure
   * 
   */
  struct Device
  {
    std::string id;
    std::string name;
  };

  using Devices = std::vector<Device>;
}
