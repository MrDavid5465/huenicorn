#pragma once

#include <Huenicorn/Serialization/JsonSerializer.hpp>
#include <Huenicorn/EntertainmentConfiguration.hpp>

#include <Huenicorn/Serialization/Channel.hpp>
#include <Huenicorn/Serialization/Device.hpp>

namespace Huenicorn::Serialization
{
  template<>
  struct JsonSerializer<Huenicorn::EntertainmentConfiguration>
  {
    // Deserialization
    static void from_json(const Json& jsonEntConf, EntertainmentConfiguration& entConf)
    {
      jsonEntConf.at("metadata").at("name").get_to(entConf.name);

      const auto& lightServices = jsonEntConf.at("light_services");
      for (const auto& lightService : lightServices) {
        std::string lightId = lightService.at("rid");
        Device device;
        device.id = lightId; // Initialize with default value
        entConf.devices.insert({lightId, device});
      }
    }


    // Serialization
    static void to_json(Json& jsonEntConf, const EntertainmentConfiguration& entConf)
    {
      jsonEntConf = {
        {"name", entConf.name},
        //{"devices", entConf.devices},
        //{"channels", entConf.channels},
      };
    }
  };


  template<>
  struct JsonSerializer<Huenicorn::EntertainmentConfigurations>
  {
    static void to_json(Json& jsonEntConfs, const EntertainmentConfigurations& entConfs)
    {
      jsonEntConfs = Json::array();
      for(const auto& entertainmentConfiguration : entConfs){
        auto& it = jsonEntConfs.emplace_back(entertainmentConfiguration.second);
        it["entertainmentConfigurationId"] = entertainmentConfiguration.first;
      }
    }
  };
}
