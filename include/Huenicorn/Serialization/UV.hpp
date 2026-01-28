#pragma once

#include <Huenicorn/Serialization/JsonSerializer.hpp>
#include <Huenicorn/UV.hpp>


namespace Huenicorn::Serialization
{
  template<>
  struct JsonSerializer<Huenicorn::UV>
  {
    static void to_json(Json& jsonUv, const UV& uv)
    {
      jsonUv = {
        {"x", uv.x},
        {"y", uv.y}
      };
    }
  };


  template<>
  struct JsonSerializer<Huenicorn::UVs>
  {
    static void to_json(Json& jsonUvs, const UVs& uvs)
    {
      jsonUvs = {
        {"uvA", uvs.min},
        {"uvB", uvs.max}
      };
    }
  };
}
