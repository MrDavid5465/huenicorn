#include <Huenicorn/Network/Http/Server/SetupBackend.hpp>

#include <chrono>

#include <Huenicorn/Core/Runtime.hpp>
#include <Huenicorn/Platform/Selector.hpp>
#include <Huenicorn/Serialization/Credentials.hpp>
#include <Huenicorn/Core/Logger.hpp>


using namespace std::chrono_literals;

namespace Huenicorn::Network::Http::Server
{
  SetupBackend::SetupBackend(Huenicorn::Core::Runtime* runtime):
  IRestServer("setup.html"),
  m_runtime(runtime)
  {
    CROW_ROUTE(m_app, "/api/finishSetup").methods(crow::HTTPMethod::POST)
    ([this](const crow::request& /*req*/, crow::response& res){
      _finish(res);
    });

    CROW_ROUTE(m_app, "/api/abort").methods(crow::HTTPMethod::POST)
    ([this](const crow::request& /*req*/, crow::response& res){
      _abort(res);
    });

    CROW_ROUTE(m_app, "/api/autodetectBridge").methods(crow::HTTPMethod::GET)
    ([this](const crow::request& /*req*/, crow::response& res){
      _autodetectBridge(res);
    });
  
    CROW_ROUTE(m_app, "/api/configFilePath").methods(crow::HTTPMethod::GET)
    ([this](const crow::request& /*req*/, crow::response& res){
      _configFilePath(res);
    });

    CROW_ROUTE(m_app, "/api/validateBridgeAddress").methods(crow::HTTPMethod::PUT)
    ([this](const crow::request& req, crow::response& res){
      _validateBridgeAddress(req, res);
    });

    CROW_ROUTE(m_app, "/api/validateCredentials").methods(crow::HTTPMethod::PUT)
    ([this](const crow::request& req, crow::response& res){
      _validateCredentials(req, res);
    });

    CROW_ROUTE(m_app, "/api/registerNewUser").methods(crow::HTTPMethod::PUT)
    ([this](const crow::request& /*req*/, crow::response& res){
      _registerNewUser(res);
    });

    m_webfileBlackList.insert("index.html");
  }


  SetupBackend::~SetupBackend()
  {}


  bool SetupBackend::aborted() const
  {
    return m_aborted;
  }


  void SetupBackend::_onStart()
  {
    std::thread spawnBrowserThread([this](){_spawnBrowser();});
    spawnBrowserThread.detach();
  }


  void SetupBackend::_spawnBrowser()
  {
    while (!running()){
      std::this_thread::sleep_for(100ms);
    }

    std::stringstream serviceUrlStream;
    serviceUrlStream << "http://127.0.0.1:" << m_app.port();
    std::string serviceURL = serviceUrlStream.str();
    Core::Logger::log("Setup WebUI is ready and available at ", serviceURL);

    Platform::adapter.openWebBrowser(serviceURL);
  }


  void SetupBackend::_getVersion(crow::response& res) const
  {
    Serialization::Json jsonResponse = {
      {"version", m_runtime->version()},
    };

    std::string response = jsonResponse.dump();
    res.set_header("Content-Type", "application/json");
    res.write(response);
    res.end();
  }


  void SetupBackend::_finish(crow::response& res)
  {
    std::string response = "{}";
    res.set_header("Content-Type", "application/json");
    res.write(response);
    res.end();

    stop();
  }


  void SetupBackend::_abort(crow::response& res)
  {
    std::string response = "{}";
    res.set_header("Content-Type", "application/json");
    res.write(response);
    res.end();

    m_aborted = true;

    stop();
  }


  void SetupBackend::_autodetectBridge(crow::response& res)
  {
    Serialization::Json jsonResponse = m_runtime->autodetectedBridge();
    std::string response = jsonResponse.dump();
    res.set_header("Content-Type", "application/json");
    res.write(response);
    res.end();
  }


  void SetupBackend::_configFilePath(crow::response& res)
  {
    Serialization::Json jsonResponse = {{"configFilePath", m_runtime->configFilePath()}};
    std::string response = jsonResponse.dump();
    res.set_header("Content-Type", "application/json");
    res.write(response);
    res.end();
  }


  void SetupBackend::_validateBridgeAddress(const crow::request& req, crow::response& res)
  {
    const std::string& data = req.body;
    Serialization::Json jsonBridgeAddressData = Serialization::Json::parse(data);

    std::string bridgeAddress = jsonBridgeAddressData.at("bridgeAddress");

    Serialization::Json jsonResponse = {{"succeeded", m_runtime->validateBridgeAddress(bridgeAddress)}};

    std::string response = jsonResponse.dump();
    res.set_header("Content-Type", "application/json");
    res.write(response);
    res.end();
  }


  void SetupBackend::_validateCredentials(const crow::request& req, crow::response& res)
  {
    const std::string& data = req.body;
    Serialization::Json jsonCredentials = Serialization::Json::parse(data);

    auto credentials = jsonCredentials.get<Hue::Auth::Credentials>();

    Serialization::Json jsonResponse = {{"succeeded", m_runtime->validateCredentials(credentials)}};

    std::string response = jsonResponse.dump();
    res.set_header("Content-Type", "application/json");
    res.write(response);
    res.end();
  }


  void SetupBackend::_registerNewUser(crow::response& res)
  {
    Serialization::Json jsonResponse = m_runtime->registerNewUser();
    std::string response = jsonResponse.dump();
    res.set_header("Content-Type", "application/json");
    res.write(response);
    res.end();
  }
}
