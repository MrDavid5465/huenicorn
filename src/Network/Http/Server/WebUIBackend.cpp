#include <Huenicorn/Network/Http/Server/WebUIBackend.hpp>

#include <fstream>
#include <sstream>

#include <Huenicorn/HuenicornCore.hpp>
#include <Huenicorn/Logger.hpp>
#include <Huenicorn/Serialization/Channel.hpp>
#include <Huenicorn/Serialization/EntertainmentConfiguration.hpp>


namespace Huenicorn::Network::Http::Server
{
  using namespace Serialization;

  WebUIBackend::WebUIBackend(Huenicorn::HuenicornCore* huenicornCore):
  IRestServer("index.html"),
  m_huenicornCore(huenicornCore)
  {
    CROW_ROUTE(m_app, "/api/webUIStatus").methods(crow::HTTPMethod::GET)
    ([this](const crow::request& /*req*/, crow::response& res){
      _getWebUIStatus(res);
    });

    CROW_ROUTE(m_app, "/api/entertainmentConfigurations").methods(crow::HTTPMethod::GET)
    ([this](const crow::request& /*req*/, crow::response& res){
      _getEntertainmentConfigurations(res);
    });

    CROW_ROUTE(m_app, "/api/channel/<int>").methods(crow::HTTPMethod::GET)
    ([this](const crow::request& /*req*/, crow::response& res, int channelId){
      _getChannel(res, channelId);
    });

    CROW_ROUTE(m_app, "/api/channels").methods(crow::HTTPMethod::GET)
    ([this](const crow::request& /*req*/, crow::response& res){
      _getChannels(res);
    });

    CROW_ROUTE(m_app, "/api/displayInfo").methods(crow::HTTPMethod::GET)
    ([this](const crow::request& /*req*/, crow::response& res){
      _getDisplayInfo(res);
    });

    CROW_ROUTE(m_app, "/api/interpolationInfo").methods(crow::HTTPMethod::GET)
    ([this](const crow::request& /*req*/, crow::response& res){
      _getInterpolationInfo(res);
    });

    CROW_ROUTE(m_app, "/api/setEntertainmentConfiguration").methods(crow::HTTPMethod::PUT)
    ([this](const crow::request& req, crow::response& res){
      _setEntertainmentConfiguration(req, res);
    });

    CROW_ROUTE(m_app, "/api/setChannelUV/<int>").methods(crow::HTTPMethod::PUT)
    ([this](const crow::request& req, crow::response& res, int channelId){
      _setChannelUV(req, res, channelId);
    });

    CROW_ROUTE(m_app, "/api/setChannelGammaFactor/<int>").methods(crow::HTTPMethod::PUT)
    ([this](const crow::request& req, crow::response& res, int channelId){
      _setChannelGammaFactor(req, res, channelId);
    });

    CROW_ROUTE(m_app, "/api/setSubsampleWidth").methods(crow::HTTPMethod::PUT)
    ([this](const crow::request& req, crow::response& res){
      _setSubsampleWidth(req, res);
    });

    CROW_ROUTE(m_app, "/api/setRefreshRate").methods(crow::HTTPMethod::PUT)
    ([this](const crow::request& req, crow::response& res){
      _setRefreshRate(req, res);
    });

    CROW_ROUTE(m_app, "/api/setInterpolation").methods(crow::HTTPMethod::PUT)
    ([this](const crow::request& req, crow::response& res){
      _setInterpolation(req, res);
    });

    CROW_ROUTE(m_app, "/api/setChannelActivity/<int>").methods(crow::HTTPMethod::POST)
    ([this](const crow::request& req, crow::response& res, int channelId){
      _setChannelActivity(req, res, channelId);
    });

    CROW_ROUTE(m_app, "/api/saveProfile").methods(crow::HTTPMethod::POST)
    ([this](const crow::request& /*req*/, crow::response& res){
      _saveProfile(res);
    });

    CROW_ROUTE(m_app, "/api/stop").methods(crow::HTTPMethod::POST)
    ([this](const crow::request& /*req*/, crow::response& res){
      _stop(res);
    });

    m_webfileBlackList.insert("setup.html");
  }


  void WebUIBackend::_onStart()
  {
    std::stringstream ss;
    ss << "Huenicorn management panel is now available at http://localhost:" <<  m_app.port();
    Logger::log(ss.str());

    if(m_readyWebUIPromise.has_value()){
      m_readyWebUIPromise.value().set_value(true);
    }
  }


  void WebUIBackend::_getVersion(crow::response& res) const
  {
    Json jsonResponse = {
      {"version", m_huenicornCore->version()},
    };

    std::string response = jsonResponse.dump();
    res.set_header("Content-Type", "application/json");
    res.write(response);
    res.end();
  }


  void WebUIBackend::_getWebUIStatus(crow::response& res) const
  {
    Json jsonResponse = {
      {"ready", true},
    };

    std::string response = jsonResponse.dump();
    res.set_header("Content-Type", "application/json");
    res.write(response);
    res.end();
  }


  void WebUIBackend::_getEntertainmentConfigurations(crow::response& res) const
  {
    const auto& entertainmentConfigurations = m_huenicornCore->entertainmentConfigurations();
    std::string currentEntertainmentConfigurationId = m_huenicornCore->currentEntertainmentConfigurationId().value();

    Json jsonResponse = {
      {"entertainmentConfigurations", entertainmentConfigurations},
      {"currentEntertainmentConfigurationId", currentEntertainmentConfigurationId}
    };

    std::string response = jsonResponse.dump();
    res.set_header("Content-Type", "application/json");
    res.write(response);
    res.end();
  }


  void WebUIBackend::_getChannel(crow::response& res, uint8_t channelId) const
  {
    std::string response = Json(m_huenicornCore->channels().at(channelId)).dump();

    res.set_header("Content-Type", "application/json");
    res.write(response);
    res.end();
  }


  void WebUIBackend::_getChannels(crow::response& res) const
  {
    std::string response = Json(m_huenicornCore->channels()).dump();

    res.set_header("Content-Type", "application/json");
    res.write(response);
    res.end();
  }


  void WebUIBackend::_getDisplayInfo(crow::response& res) const
  {
    auto displayResolution = m_huenicornCore->displayResolution();

    Json jsonSubsampleCandidates = Json::array();
    for(const auto& candidate : m_huenicornCore->subsampleResolutionCandidates()){
      jsonSubsampleCandidates.push_back({
        {"x", candidate.x},
        {"y", candidate.y}
      });
    }

    Json jsonDisplayInfo{
      {"x", displayResolution.x},
      {"y", displayResolution.y},
      {"subsampleWidth", m_huenicornCore->subsampleWidth()},
      {"subsampleResolutionCandidates", jsonSubsampleCandidates},
      {"selectedRefreshRate", m_huenicornCore->refreshRate()},
      {"maxRefreshRate", m_huenicornCore->maxRefreshRate()}
    };

    std::string response = jsonDisplayInfo.dump();
    res.set_header("Content-Type", "application/json");
    res.write(response);
    res.end();
  }


  void WebUIBackend::_getInterpolationInfo(crow::response& res) const
  {
    Json jsonAvailableInterpolations = Json::array();
    for(const auto& [key, value] : m_huenicornCore->availableInterpolations()){
      jsonAvailableInterpolations.push_back({
        {key, value},
      });
    }

    Json jsonInterpolationInfo = {
      {"available", jsonAvailableInterpolations},
      {"current", m_huenicornCore->interpolation()}
    };

    std::string response = jsonInterpolationInfo.dump();
    res.set_header("Content-Type", "application/json");
    res.write(response);
    res.end();
  }


  void WebUIBackend::_setEntertainmentConfiguration(const crow::request& req, crow::response& res) const
  {
    const std::string& data = req.body;
    std::string entertainmentConfigurationId = Json::parse(data);

    bool succeeded = m_huenicornCore->setEntertainmentConfiguration(entertainmentConfigurationId);

    Json jsonResponse = {
      {"succeeded", succeeded},
      {"entertainmentConfigurationId", entertainmentConfigurationId},
      {"channels", Json(m_huenicornCore->channels())}
    };

    std::string response = jsonResponse.dump();
    res.set_header("Content-Type", "application/json");
    res.write(response);
    res.end();
  }


  void WebUIBackend::_setChannelUV(const crow::request& req, crow::response& res, uint8_t channelId) const
  {
    const std::string& data = req.body;
    Json jsonUV = Json::parse(data);

    float x = jsonUV.at("x");
    float y = jsonUV.at("y");
    UVCorner uvCorner = static_cast<UVCorner>(jsonUV.at("type").get<int>());

    const auto& clampedUVs = m_huenicornCore->setChannelUV(channelId, {x, y}, uvCorner);

    // TODO : Serialize from JsonSerializer
    Json jsonResponse = {
      {"uvA", {{"x", clampedUVs.min.x}, {"y", clampedUVs.min.y}}},
      {"uvB", {{"x", clampedUVs.max.x}, {"y", clampedUVs.max.y}}}
    };

    std::string response = jsonResponse.dump();
    res.set_header("Content-Type", "application/json");
    res.write(response);
    res.end();
  }


  void WebUIBackend::_setChannelGammaFactor(const crow::request& req, crow::response& res, uint8_t channelId) const
  {
    const std::string& data = req.body;
    Json jsonGammaFactorData = Json::parse(data);
    float gammaFactor = jsonGammaFactorData.at("gammaFactor");

    if(!m_huenicornCore->setChannelGammaFactor(channelId, gammaFactor)){
      std::string response = Json{
        {"succeeded", false},
        {"error", "invalid channel id"}
      }.dump();
      
      res.set_header("Content-Type", "application/json");
      res.write(response);
      res.end();
      return;
    }

    Json jsonResponse = Json{
      {"succeeded", true},
      {"gammaFactor", gammaFactor}
    };

    std::string response = jsonResponse.dump();
    res.set_header("Content-Type", "application/json");
    res.write(response);
    res.end();
  }


  void WebUIBackend::_setSubsampleWidth(const crow::request& req, crow::response& res) const
  {
    const std::string& data = req.body;
    int subsampleWidth = Json::parse(data).get<int>();
    m_huenicornCore->setSubsampleWidth(subsampleWidth);

    glm::ivec2 displayResolution = m_huenicornCore->displayResolution();
    Json jsonDisplay{
      {"x", displayResolution.x},
      {"y", displayResolution.y},
      {"subsampleWidth", m_huenicornCore->subsampleWidth()}
    };

    std::string response = jsonDisplay.dump();
    res.set_header("Content-Type", "application/json");
    res.write(response);
    res.end();
  }


  void WebUIBackend::_setRefreshRate(const crow::request& req, crow::response& res) const
  {
    const std::string& data = req.body;
    unsigned refreshRate = Json::parse(data).get<unsigned>();
    m_huenicornCore->setRefreshRate(refreshRate);

    Json jsonRefreshRate{
      {"refreshRate", m_huenicornCore->refreshRate()}
    };

    std::string response = jsonRefreshRate.dump();
    res.set_header("Content-Type", "application/json");
    res.write(response);
    res.end();
  }


  void WebUIBackend::_setInterpolation(const crow::request& req, crow::response& res) const
  {
    const std::string& data = req.body;
    unsigned interpolation = Json::parse(data).get<unsigned>();

    m_huenicornCore->setInterpolation(interpolation);

    Json jsonInterpolation{
      {"interpolation", m_huenicornCore->interpolation()}
    };

    std::string response = jsonInterpolation.dump();
    res.set_header("Content-Type", "application/json");
    res.write(response);
    res.end();
  }


  void WebUIBackend::_setChannelActivity(const crow::request& req, crow::response& res, uint8_t channelId) const
  {
    const std::string& data = req.body;
    Json jsonChannelData = Json::parse(data);
    bool active = jsonChannelData.at("active");

    if(!m_huenicornCore->setChannelActivity(channelId, active)){
      std::string response = Json{
        {"succeeded", false},
        {"error", "invalid channel id"}
      }.dump();
      
      res.set_header("Content-Type", "application/json");
      res.write(response);
      res.end();
      return;
    }

    Json jsonResponse = Json{
      {"succeeded", true},
      {"channels", Json(m_huenicornCore->channels())},
    };
    
    if(active){
      jsonResponse["newActiveChannelId"] = channelId;
    }

    std::string response = jsonResponse.dump();
    res.set_header("Content-Type", "application/json");
    res.write(response);
    res.end();
  }


  void WebUIBackend::_saveProfile(crow::response& res) const
  {
    m_huenicornCore->saveProfile();

    Json jsonResponse = {
      "succeeded", true
    };

    std::string response = jsonResponse.dump();
    res.set_header("Content-Type", "application/json");
    res.write(response);
    res.end();
  }


  void WebUIBackend::_stop(crow::response& res) const
  {
    Json jsonResponse = {{
      "succeeded", true
    }};

    std::string response = jsonResponse.dump();
    res.set_header("Content-Type", "application/json");
    res.write(response);
    res.end();

    m_huenicornCore->stop();
  }
}
