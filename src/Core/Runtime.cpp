#include <Huenicorn/Core/Runtime.hpp>

#include <fstream>
#include <chrono>

#include <Huenicorn/Grabber/DummyGrabber.hpp>
#include <Huenicorn/Network/Http/Client/Client.hpp>
#include <Huenicorn/Imaging/ImageProcessing.hpp>
#include <Huenicorn/Imaging/Interpolation.hpp>
#include <Huenicorn/Core/Logger.hpp>
#include <Huenicorn/Platform/Selector.hpp>
#include <Huenicorn/Network/Http/Server/SetupBackend.hpp>
#include <Huenicorn/Network/Http/Server/WebUIBackend.hpp>

#include <Huenicorn/Serialization/Channel.hpp>
#include <Huenicorn/Serialization/Credentials.hpp>


using namespace std::chrono_literals;


namespace Huenicorn::Core
{
  Runtime::Runtime(const std::string& version, const std::filesystem::path& configRoot):
  m_version(version),
  m_configRoot(configRoot),
  m_config(m_configRoot)
  {}


  const std::string& Runtime::version() const
  {
    return m_version;
  }


  const std::filesystem::path Runtime::configFilePath() const
  {
    return m_config.configFilePath();
  }


  const Hue::Api::Channels& Runtime::channels() const
  {
    return m_channels;
  }


  const Hue::Api::EntertainmentConfigurations& Runtime::entertainmentConfigurations() const
  {
    return m_selector->entertainmentConfigurations();
  }


  const Hue::Api::EntertainmentConfiguration& Runtime::currentEntertainmentConfiguration() const
  {
    return m_selector->currentEntertainmentConfiguration();
  }


  std::optional<std::string> Runtime::currentEntertainmentConfigurationId() const
  {
    return m_selector->currentEntertainmentConfigurationId();
  }


  glm::ivec2 Runtime::displayResolution() const
  {
    return m_grabber->displayResolution();
  }


  std::vector<glm::ivec2> Runtime::subsampleResolutionCandidates() const
  {
    return m_grabber->subsampleResolutionCandidates();
  }


  unsigned Runtime::subsampleWidth() const
  {
    return m_config.subsampleWidth();
  }


  unsigned Runtime::refreshRate() const
  {
    return m_config.refreshRate();
  }


  unsigned Runtime::maxRefreshRate() const
  {
    return m_grabber->displayRefreshRate();
  }


  Imaging::Interpolation::Type Runtime::interpolation() const
  {
    return m_config.interpolation();
  }


  const Imaging::Interpolation::Interpolations& Runtime::availableInterpolations() const
  {
    return Imaging::Interpolation::availableInterpolations;
  }


  Serialization::Json Runtime::autodetectedBridge() const
  {
    auto detectedBridgeResponse = Network::Http::Client::sendRequest("https://discovery.meethue.com/", "GET");

    if(!detectedBridgeResponse.has_value()){
      return {{"succeeded", false}, {"error", "Could not reach discovery service. Please check your internet connection."}};
    }

    auto bridges = detectedBridgeResponse.value().asJson();

    return {{"succeeded", true}, {"bridges", bridges}};
  }


  Serialization::Json Runtime::registerNewUser()
  {
    std::string sessionUsername = Platform::adapter.getUsername();
    std::string deviceType = "huenicorn#" + sessionUsername;

    Serialization::Json request = {{"devicetype", deviceType}, {"generateclientkey", true}};
    auto response = Network::Http::Client::sendRequest(m_config.bridgeAddress().value() + "/api", "POST", request.dump());

    if(!response.has_value()){
      return {{"succeeded", false}, {"error", "unreachable bridge"}};
    }

    auto jsonResponse = response.value().asJson();

    if(jsonResponse.at(0).contains("error")){
      return {{"succeeded", false}, {"error", jsonResponse.at(0).at("error").at("description")}};
    }

    auto credentials = jsonResponse.at(0).at("success").get<Hue::Auth::Credentials>();
    m_config.setCredentials(credentials);

    return {{"succeeded", true}, {"username", credentials.username()}, {"clientkey", credentials.clientkey()}};
  }


  bool Runtime::setEntertainmentConfiguration(const std::string& entertainmentConfigurationId)
  {
    if(!m_selector->selectEntertainmentConfiguration(entertainmentConfigurationId)){
      return false;
    }

    _enableEntertainmentConfiguration(entertainmentConfigurationId);

    return true;
  }


  const Imaging::UVs& Runtime::setChannelUV(uint8_t channelId, Imaging::UV&& uv, Imaging::UVCorner uvCorner)
  {
    return m_channels.at(channelId).setUV(std::move(uv), uvCorner);
  }


  bool Runtime::setChannelGammaFactor(uint8_t channelId, float gammaExponent)
  {
    if(m_channels.find(channelId) == m_channels.end()){
      return false;
    }

    m_channels.at(channelId).setGammaFactor(gammaExponent);
    return true;
  }


  void Runtime::setSubsampleWidth(unsigned subsampleWidth)
  {
    m_config.setSubsampleWidth(subsampleWidth);
  }


  void Runtime::setRefreshRate(unsigned refreshRate)
  {
    refreshRate = std::min(refreshRate, m_grabber->displayRefreshRate());

    m_config.setRefreshRate(refreshRate);
    refreshRate = m_config.refreshRate();

    m_loopRegulator->setTickInterval(Timing::fromHertz(refreshRate));
  }


  void Runtime::setInterpolation(unsigned interpolation)
  {
    m_config.setInterpolation(static_cast<Imaging::Interpolation::Type>(interpolation));
  }


  void Runtime::start()
  {
    if(!m_config.initialSetupOk()){
      if(!_runInitialSetup()){
        return;
      }
    }

    if(!_initGrabber()){
      Logger::error("Could not start any grabber");
      return;
    }

    Logger::log("Started ", m_grabber->name());

    if(!_initSettings()){
      Logger::error("Could not load suitable entertainment configuration.");
      return;

      // TODO : Add tool to create entertainment configurations inside Huenicorn
      // so the official application would no longer be a requirement
    }

    _initWebUI();


    // Spawn UI if no profiles are found
    auto optJsonProfile = _getProfile();
    if(!optJsonProfile.has_value() && !m_openedSetup){
      std::thread spawnBrowser([this](){_spawnBrowser();});
      spawnBrowser.detach();
    }

    _startStreamingLoop();
  }


  void Runtime::stop()
  {
    m_keepLooping = false;
  }


  bool Runtime::validateBridgeAddress(const std::string& bridgeAddress)
  {
    std::string sanitizedAddress = bridgeAddress;

    while(sanitizedAddress.back() == '/'){
      sanitizedAddress.pop_back();
    }

    auto response = Network::Http::Client::sendRequest(sanitizedAddress + "/api", "GET", "");
    if(!response.has_value()){
      return false;
    }

    m_config.setBridgeAddress(sanitizedAddress);

    return true;
  }


  bool Runtime::validateCredentials(const Hue::Auth::Credentials& credentials)
  {
    auto response = Network::Http::Client::sendRequest(m_config.bridgeAddress().value() + "/api/" + credentials.username(), "GET", "");
    if(!response.has_value()){
      return false;
    }

    try{
      auto jsonResponse = response.value().asJson();

      if(jsonResponse.is_array() && jsonResponse.at(0).contains("error")){
        return false;
      }
    }
    catch(const std::exception& e){
      Logger::error(e.what());
      return false;
    }

    m_config.setCredentials(credentials);
    Logger::log("Successfully registered credentials");
    return true;
  }


  bool Runtime::setChannelActivity(uint8_t channelId, bool active)
  {
    if(m_channels.find(channelId) == m_channels.end()){
      return false;
    }

    m_channels.at(channelId).setActive(active);

    _updateStreamChannelsSize();

    return true;
  }


  void Runtime::saveProfile()
  {
    if(!m_selector->validSelection()){
      Logger::error("There is currently no valid entertainment configuration selected.");
      return;
    }

    Serialization::Json profile;
    if(m_selector->validSelection()){
      profile = Serialization::Json{
        {"entertainmentConfigurationId", m_selector->currentEntertainmentConfigurationId().value()},
        {"channels", m_channels}
      };
    }

    if(!m_config.profileName().has_value()){
      m_config.setProfileName("profile");
    }

    std::ofstream profileFile(_profilePath(), std::ofstream::out);
    profileFile << profile.dump(2) << "\n";
    profileFile.close();
  }


  std::filesystem::path Runtime::_profilePath() const
  {
    if(!m_config.profileName().has_value()){
      return {};
    }

    return m_configRoot / std::filesystem::path(m_config.profileName().value()).replace_extension("json");
  }


  std::optional<Serialization::Json> Runtime::_getProfile()
  {
    std::filesystem::path profilePath = _profilePath();
    Serialization::Json jsonProfile = Serialization::Json::object();

    if(!profilePath.empty() && std::filesystem::exists(profilePath) && std::filesystem::is_regular_file(profilePath)){
      std::ifstream profileFile(profilePath);
      return Serialization::Json::parse(profileFile);
    }

    return std::nullopt;
  }


  bool Runtime::_initSettings()
  {
    if(m_config.refreshRate() == 0){
      m_config.setRefreshRate(m_grabber->displayRefreshRate());
    }

    if(m_config.subsampleWidth() == 0){
      auto displayResolution = m_grabber->displayResolution();
      float percentThreshold = 1.0;
      auto subsampleCandidates = m_grabber->subsampleResolutionCandidates();

      unsigned bestSubsampleWidth = subsampleCandidates.back().x;
      for(int i = subsampleCandidates.size(); i--;){
        unsigned candidate = subsampleCandidates.at(i).x;

        if((static_cast<float>(candidate) / displayResolution.x) * 100 >= percentThreshold){
          bestSubsampleWidth = candidate;
          break;
        }
      }

      m_config.setSubsampleWidth(bestSubsampleWidth);
    }

    const float warningThreshold = 50.0;

    float ratio = static_cast<float>(m_config.subsampleWidth()) / m_grabber->displayResolution().x;

    if(ratio >= warningThreshold / 100){
      Logger::warn("Subsample width is >= ", warningThreshold, "% of the display resolution. Color computation might be intensive.");
    }

    Logger::log("Configuration is ready. Feel free to modify it manually by editing ", std::quoted(m_config.configFilePath().string()));

    const auto& credentials = m_config.credentials().value();
    const std::string& bridgeAddress =  m_config.bridgeAddress().value();

    m_selector = std::make_unique<Hue::Api::EntertainmentConfigurationSelector>(credentials, bridgeAddress);

    std::string entertainmentConfigurationId = {};
    auto optJsonProfile = _getProfile();

    if(optJsonProfile.has_value()){
      const auto& jsonProfile = optJsonProfile.value();
      if(jsonProfile.contains("entertainmentConfigurationId")){
        entertainmentConfigurationId = jsonProfile.at("entertainmentConfigurationId");
      }
    }

    _enableEntertainmentConfiguration(entertainmentConfigurationId);

    return true;
  }


  bool Runtime::_runInitialSetup()
  {
    Logger::log("Starting setup backend");
    unsigned port = m_config.restServerPort();
    const std::string& boundBackendIP = m_config.boundBackendIP();

    Network::Http::Server::SetupBackend sb(this);
    sb.start(port, boundBackendIP);

    if(sb.aborted()){
      Logger::log("Initial setup was aborted");
      return false;
    }

    Logger::log("Finished setup");
    m_openedSetup = true;

    return true;
  }


  bool Runtime::_initGrabber()
  {
    m_grabber = Platform::adapter.getGrabber(&m_config);
    return true;
  }


  void Runtime::_initWebUI()
  {
    std::promise<bool> readyWebUIPromise;
    auto readyWebUIFuture = readyWebUIPromise.get_future();

    unsigned restServerPort = m_config.restServerPort();
    const std::string& boundBackendIP = m_config.boundBackendIP();
    m_webUIService.server = std::make_unique<Network::Http::Server::WebUIBackend>(this);
    m_webUIService.thread.emplace([&](){
      m_webUIService.server->start(restServerPort, boundBackendIP, std::move(readyWebUIPromise));
    });

    readyWebUIFuture.wait();
  }


  void Runtime::_initChannels(const Serialization::Json& jsonChannels)
  {
    const auto& username = m_config.credentials().value().username();
    const auto& bridgeAddress =  m_config.bridgeAddress().value();
    auto devices = Hue::Api::ApiTools::loadDevices(username, bridgeAddress);
    auto entertainmentConfigurationsChannels = Hue::Api::ApiTools::loadEntertainmentConfigurationsChannels(username, bridgeAddress);

    Hue::Api::Channels channels;

    for(const auto& [id, channel] : m_selector->currentEntertainmentConfiguration().channels){
      bool found = false;
      const auto& members = Hue::Api::ApiTools::matchDevices(entertainmentConfigurationsChannels.at(m_selector->currentEntertainmentConfigurationId().value()).at(id), devices);
      for(const auto& jsonProfileChannel : jsonChannels){
        if(jsonProfileChannel.at("channelId") == id){
          bool active = jsonProfileChannel.at("active");
          Serialization::Json jsonUVs = jsonProfileChannel.at("uvs");

          Imaging::UVs uvs = jsonUVs.get<Imaging::UVs>();

          float gammaFactor = jsonProfileChannel.at("gammaFactor");
          channels.emplace(id, Hue::Api::Channel{active, members, gammaFactor, uvs});

          found = true;
          break;
        }
      }

      if(!found){
        channels.emplace(id, Hue::Api::Channel{false, members, 0.0f});
      }
    }

    m_channels = std::move(channels);
    _updateStreamChannelsSize();
  }


  void Runtime::_spawnBrowser()
  {
    while (!m_webUIService.server->running()){
      std::this_thread::sleep_for(100ms);
    }

    std::stringstream serviceUrlStream;
    serviceUrlStream << "http://127.0.0.1:" << m_config.restServerPort();
    std::string serviceURL = serviceUrlStream.str();
    Logger::log("Management WebUI is ready and available at ", serviceURL);

    Platform::adapter.openWebBrowser(serviceURL);
  }


  void Runtime::_enableEntertainmentConfiguration(const std::string& entertainmentConfigurationId)
  {
    if(!m_selector->selectEntertainmentConfiguration(entertainmentConfigurationId)){
      return;
    }

    const auto& credentials = m_config.credentials().value();

    {
      std::lock_guard lock(m_streamerMutex);
      m_streamer = std::make_unique<Stream::Streamer>(credentials, m_config.bridgeAddress().value());
      m_streamer->setEntertainmentConfigurationId(m_selector->currentEntertainmentConfigurationId().value());
    }

    auto profilePath = _profilePath();
    Serialization::Json jsonChannels = Serialization::Json::object();

    if(std::filesystem::is_regular_file(profilePath)){
      Serialization::Json jsonProfile = Serialization::Json::parse(std::ifstream(_profilePath()));
      if(jsonProfile.contains("channels")){
        jsonChannels = jsonProfile.at("channels");
      }
    }

    _initChannels(jsonChannels);
  }


  void Runtime::_startStreamingLoop()
  {
    m_loopRegulator = std::make_unique<Timing::LoopRegulator>(Timing::fromHertz(m_config.refreshRate()));

    m_loopRegulator->start();

    m_keepLooping = true;
    while(m_keepLooping){
      _update();

      if(!m_loopRegulator->sync()){
        const auto& lastExcess = m_loopRegulator->lastExcess();
        float percentage = lastExcess.rate * 100;
        Logger::warn("Scheduled interval has been exceeded of ", lastExcess.extra.count(), "[s] (", percentage, "%).\n Please reduce refresh rate if this warning persists.");
      }
    }

    _shutdown();

    m_webUIService.server->stop();
    m_webUIService.thread.value().join();
  }


  void Runtime::_update()
  {
    m_grabber->grabFrameSubsample(m_frameData);
    if(!m_frameData.hasData()){
      // Grabbers with asynchronous capture may produce slower than the core consumes
      return;
    }

    Imaging::ImageData subframeImageData;

    for(auto& [channelId, channel] : m_channels){
      if(channel.state() == Hue::Api::Channel::State::Inactive){
        continue;
      }

      auto& channelStream = m_channelStreams.at(channelId);

      if(channel.state() == Hue::Api::Channel::State::PendingShutdown){
        channelStream.id = channelId;
        channelStream.r = 0;
        channelStream.g = 0;
        channelStream.b = 0;
        channel.acknowledgeShutdown();
      }
      else{
        const auto& uvs = channel.uvs();

        glm::ivec2 a{uvs.min.x * m_frameData.width(), uvs.min.y * m_frameData.height()};
        glm::ivec2 b{uvs.max.x * m_frameData.width(), uvs.max.y * m_frameData.height()};

        Imaging::ImageProcessing::getSubImage(m_frameData, subframeImageData, a, b);
        auto color = Imaging::ImageProcessing::getDominantColor(subframeImageData);

        glm::vec3 normalized = color.toNormalized();
        glm::vec3 correctedColor = glm::pow(normalized, glm::vec3(channel.gammaExponent()));

        channelStream.id = channelId;
        channelStream.r = correctedColor.r;
        channelStream.g = correctedColor.g;
        channelStream.b = correctedColor.b;
      }
    }

    {
      std::lock_guard lock(m_streamerMutex);
      if(m_streamer.get()){
        m_streamer->streamChannels(m_channelStreams);
      }
    }
  }


  void Runtime::_shutdown()
  {
    m_selector->disableStreaming();
  }


  void Runtime::_updateStreamChannelsSize()
  {
    m_channelStreams.resize(m_channels.size());
  }
}
