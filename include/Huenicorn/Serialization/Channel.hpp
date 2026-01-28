#pragma once

#include <Huenicorn/Serialization/JsonSerializer.hpp>
#include <Huenicorn/Channel.hpp>
#include <Huenicorn/Serialization/Device.hpp>
#include <Huenicorn/Serialization/UV.hpp>

#include <Huenicorn/Logger.hpp>


namespace Huenicorn::Serialization
{
  template<>
  struct JsonSerializer<Huenicorn::Channel>
  {
    // Serialization
    static void to_json(Json& jsonChannel, const Channel& channel)
    {
      jsonChannel = {
        {"active", channel.state() == Channel::State::Active},
        {"uvs", Json(channel.uvs())},
        {"gammaFactor", channel.gammaFactor()},
        {"devices", Json(channel.devices())}
      };
    }
  };


  template<>
  struct JsonSerializer<Huenicorn::Channels>
  {
    static void to_json(Json& jsonChannels, const Channels& channels)
    {
      jsonChannels = nlohmann::json::array();
      for(const auto& channel : channels){
        auto& it = jsonChannels.emplace_back(Json(channel.second));
        it["channelId"] = channel.first;
      }
    }
  };
}
