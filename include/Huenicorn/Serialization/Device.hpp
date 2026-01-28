#pragma once

#include <vector>

#include <Huenicorn/Serialization/JsonSerializer.hpp>
#include <Huenicorn/Device.hpp>


namespace Huenicorn::Serialization
{
  template<>
  struct JsonSerializer<Huenicorn::Device>
  {
    // Deserialization
    static void from_json(const Json& jsonDevice, Device& device)
    {
      jsonDevice.at("name").get_to(device.name);
      jsonDevice.at("archetype").get_to(device.type);
    }

    // Serialization
    static void to_json(Json& jsonDevice, const Device& device)
    {
      jsonDevice = {
        {"id", device.id},
        {"name", device.name},
        {"type", device.type}
      };
    }
  };
}
