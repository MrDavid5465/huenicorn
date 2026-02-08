#pragma once

#include <Huenicorn/Serialization/JsonSerializer.hpp>
#include <Huenicorn/Hue/Api/EntertainmentConfiguration.hpp>

#include <Huenicorn/Serialization/Channel.hpp>
#include <Huenicorn/Serialization/Device.hpp>

namespace Huenicorn::Serialization
{
  template<>
  struct JsonSerializer<Hue::Api::EntertainmentConfiguration>
  {
    // Deserialization
    static void from_json(
      const Json& jsonEntConf,
      Hue::Api::EntertainmentConfiguration& entConf
    )
    {
      jsonEntConf.at("metadata").at("name").get_to(entConf.name);

      const auto& lightServices = jsonEntConf.at("light_services");
      for (const auto& lightService : lightServices) {
        std::string lightId = lightService.at("rid");
        Hue::Api::Device device;
        device.id = lightId; // Initialize with default value
        entConf.devices.insert({lightId, device});
      }
    }


    // Serialization
    static void to_json(
      Json& jsonEntConf,
      const Hue::Api::EntertainmentConfiguration& entConf
    )
    {
      jsonEntConf = {
        {"name", entConf.name},
        //{"devices", entConf.devices},
        //{"channels", entConf.channels},
      };
    }
  };


  template<>
  struct JsonSerializer<Hue::Api::EntertainmentConfigurations>
  {
    static void to_json(
      Json& jsonEntConfs,
      const Hue::Api::EntertainmentConfigurations& entConfs
    )
    {
      jsonEntConfs = Json::array();
      for(const auto& entertainmentConfiguration : entConfs){
        auto& it = jsonEntConfs.emplace_back(entertainmentConfiguration.second);
        it["entertainmentConfigurationId"] = entertainmentConfiguration.first;
      }
    }
  };
}
