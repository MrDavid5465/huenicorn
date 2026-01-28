#pragma once

#include <unordered_map>

#include <Huenicorn/Channel.hpp>
#include <Huenicorn/Device.hpp>


namespace Huenicorn
{
  // Type definitions
  struct EntertainmentConfiguration;
  using EntertainmentConfigurations = std::unordered_map<std::string, EntertainmentConfiguration>;
  using EntertainmentConfigurationsIterator = EntertainmentConfigurations::iterator;
  using EntertainmentConfigurationEntry = std::pair<std::string, EntertainmentConfiguration>;


  /**
   * @brief Wrapper around Hue Entertainment Configuration
   * 
   */
  struct EntertainmentConfiguration
  {
    std::string name;
    Devices devices;
    Channels channels;
  };
}
