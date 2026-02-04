#pragma once

#include <span>
#include <string>

#include <Huenicorn/Hue/Auth/Credentials.hpp>


namespace Huenicorn::Stream
{
  struct DtlsConfig
  {
    Hue::Auth::Credentials credentials;
    std::string address;
    std::string port;
  };


  class IDtlsClient
  {
  public:
    virtual ~IDtlsClient() = default;

    virtual bool isConnected() const = 0;

    virtual void init() = 0;
    virtual void shutdown() = 0;


    virtual bool send(std::span<const std::byte> data) = 0;
  };
}
