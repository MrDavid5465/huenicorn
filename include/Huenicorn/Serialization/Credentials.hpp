#pragma once

#include <Huenicorn/Serialization/JsonSerializer.hpp>
#include <Huenicorn/Credentials.hpp>


namespace Huenicorn::Serialization
{
  template<>
  struct JsonSerializer<Huenicorn::Credentials>
  {
    // Deserialization
    static void from_json(const Json& jsonCredentials, Credentials& credentials)
    {
      jsonCredentials.at("username").get_to(credentials.m_username);
      jsonCredentials.at("clientkey").get_to(credentials.m_clientkey);
    }

    // Serialization
    static void to_json(Json& jsonCredentials, const Credentials& credentials)
    {
      jsonCredentials = Json{
        {"username", credentials.username()},
        {"clientkey", credentials.clientkey()}
      };
    }
  };
}
