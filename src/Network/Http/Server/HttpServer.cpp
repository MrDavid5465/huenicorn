#include <Huenicorn/Network/Http/Server/HttpServer.hpp>

#include <Huenicorn/Network/Http/Server/Impl/HttpLibServerImpl.hpp>


namespace Huenicorn::Network::Http::Server
{
  HttpServer::HttpServer()
  {
  }


  HttpServer::~HttpServer()
  {
  }


  bool HttpServer::start(
    const std::string& boundAddress,
    unsigned port
  )
  {
    m_httpServerImpl = std::make_unique<Impl>(m_routes);

    if(!m_httpServerImpl->bind(boundAddress, port)){
      return false;
    }

    return m_httpServerImpl->listen();

  }


  bool HttpServer::stop()
  {
    return m_httpServerImpl->stop();
  }

  void HttpServer::addRoute(
    HttpMethod method,
    const std::string& path,
    Handler handler
  )
  {
    m_routes.push_back({method, path, handler});
  }
}
