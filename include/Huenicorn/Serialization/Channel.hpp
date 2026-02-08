#pragma once

#include <Huenicorn/Serialization/JsonSerializer.hpp>
#include <Huenicorn/Hue/Api/Channel.hpp>
#include <Huenicorn/Serialization/Device.hpp>
#include <Huenicorn/Serialization/UV.hpp>

#include <Huenicorn/Core/Logger.hpp>


namespace Huenicorn::Serialization
{
  template<>
  struct JsonSerializer<Hue::Api::Channel>
  {
    // Serialization
    static void to_json(
      Json& jsonChannel,
      const Hue::Api::Channel& channel
    )
    {
      jsonChannel = {
        {"active", channel.state() == Hue::Api::Channel::State::Active},
        {"uvs", Json(channel.uvs())},
        {"gammaFactor", channel.gammaFactor()},
        {"devices", Json(channel.devices())}
      };
    }
  };


  template<>
  struct JsonSerializer<Hue::Api::Channels>
  {
    static void to_json(
      Json& jsonChannels,
      const Hue::Api::Channels& channels
    )
    {
      jsonChannels = Json::array();
      for(const auto& channel : channels){
        auto& it = jsonChannels.emplace_back(Json(channel.second));
        it["channelId"] = channel.first;
      }
    }
  };
}
