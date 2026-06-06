#pragma once

#include <Huenicorn/Network/Http/Server/HttpServer.hpp>
#include <Huenicorn/Network/Http/Server/WebrootUtils.hpp>


namespace Huenicorn::Core
{
  class CoreService;
}

namespace Huenicorn::Network::Http::Server
{
  class SetupBackend
  {
  public:
    SetupBackend(
      Huenicorn::Core::CoreService* coreService
    );

    ~SetupBackend(){}


    bool execute(
      uint32_t port,
      const std::string& boundBackendIP
    )
    {
      m_httpServer.start(boundBackendIP, port);

      return m_setupCompleted;
    }

  private:
    Huenicorn::Core::CoreService* m_coreService;
    HttpServer m_httpServer;
    bool m_setupCompleted{false};
  };
}
