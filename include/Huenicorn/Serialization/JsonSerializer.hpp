#pragma once

#include <Huenicorn/Serialization/Json.hpp>


namespace Huenicorn::Serialization
{
  template <typename T>
  struct JsonSerializer;

  template <typename T>
  concept JsonWritable =
    requires (Json& j, const T& t) {
      JsonSerializer<T>::to_json(j, t);
    };

  template <typename T>
  concept JsonReadable =
    requires (const Json& j, T& t) {
      JsonSerializer<T>::from_json(j, t);
    };
}


namespace nlohmann
{
  template <Huenicorn::Serialization::JsonWritable T>
  struct adl_serializer<T>
  {
    static void to_json(json& j, const T& value)
    {
      Huenicorn::Serialization::JsonSerializer<T>::to_json(j, value);
    }

    static void from_json(const json& j, T& value)
      requires Huenicorn::Serialization::JsonReadable<T>
    {
      Huenicorn::Serialization::JsonSerializer<T>::from_json(j, value);
    }
  };
}