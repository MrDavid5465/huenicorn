#pragma once


#include <Huenicorn/Network/Http/Server/HttpServer.hpp>
#include <Huenicorn/Network/Http/Server/WebrootUtils.hpp>


namespace Huenicorn::Core
{
  class CoreService;
}

namespace Huenicorn::Network::Http::Server
{
  class WebUIBackend
  {
  public:
    WebUIBackend(
      Huenicorn::Core::CoreService* coreService
    );

    ~WebUIBackend(){}

    bool start(
      uint32_t port,
      const std::string& boundBackendIP
    )
    {
      if(m_httpServer.start(boundBackendIP, port)){
        return true;
      }
      else{
        return false;
      }
    }


    bool stop()
    {
      return m_httpServer.stop();
    }

  private:
    Huenicorn::Core::CoreService* m_coreService;
    HttpServer m_httpServer;
  };
}
