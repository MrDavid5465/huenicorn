#pragma once

#include <vector>

#include <Huenicorn/Serialization/JsonSerializer.hpp>
#include <Huenicorn/Hue/Api/Device.hpp>


namespace Huenicorn::Serialization
{
  template<>
  struct JsonSerializer<Huenicorn::Hue::Api::Device>
  {
    // Deserialization
    static void from_json(
      const Json& jsonDevice,
      Hue::Api::Device& device
    )
    {
      jsonDevice.at("name").get_to(device.name);
    }

    // Serialization
    static void to_json(
      Json& jsonDevice,
      const Hue::Api::Device& device
    )
    {
      jsonDevice = {
        {"id", device.id},
        {"name", device.name},
      };
    }
  };
}
