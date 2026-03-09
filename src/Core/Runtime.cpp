#include <Huenicorn/Core/Runtime.hpp>

#include <fstream>

#include <Huenicorn/Grabber/DummyGrabber.hpp>
#include <Huenicorn/Network/Http/Client/Client.hpp>
#include <Huenicorn/Imaging/ImageProcessing.hpp>
#include <Huenicorn/Imaging/Interpolation.hpp>
#include <Huenicorn/Core/Logger.hpp>
#include <Huenicorn/Platform/Selector.hpp>
#include <Huenicorn/Network/Http/Server/WebUIBackend.hpp>

#include <Huenicorn/Serialization/Channel.hpp>
#include <Huenicorn/Serialization/Credentials.hpp>

#include <Huenicorn/Imaging/UV.hpp>

#include <Huenicorn/Core/CoreService.hpp>
#include <Huenicorn/Hue/Api/Channel.hpp>

#include <Huenicorn/Hue/Api/ApiTools.hpp>


using namespace std::chrono_literals;


namespace Huenicorn::Core
{
  Runtime::Runtime(
    const std::string& version,
    const std::filesystem::path& configRoot
  ):
  m_version(version),
  m_configRoot(configRoot),
  m_config(m_configRoot),
  m_coreService(std::make_unique<CoreService>(*this, m_config))
  {

  }


  const std::string& Runtime::version() const
  {
    return m_version;
  }


  void Runtime::start()
  {
    if(!m_coreService->ensureInitialSetup()){
      return;
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

    bool spawnBrowser = (!m_coreService->getProfile().has_value() && !m_openedSetup);
    _initWebUI(spawnBrowser);

    _startStreamingLoop();
  }


  void Runtime::stop()
  {
    m_keepLooping = false;
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
    auto optJsonProfile = m_coreService->getProfile();

    if(optJsonProfile.has_value()){
      const auto& jsonProfile = optJsonProfile.value();
      if(jsonProfile.contains("entertainmentConfigurationId")){
        entertainmentConfigurationId = jsonProfile.at("entertainmentConfigurationId");
      }
    }

    _enableEntertainmentConfiguration(entertainmentConfigurationId);

    return true;
  }


  bool Runtime::_initGrabber()
  {
    m_grabber = Platform::adapter.getGrabber(&m_config);
    return true;
  }


  void Runtime::_initWebUI(
    bool spawnBrowser
  )
  {
    std::promise<bool> readyWebUIPromise;
    auto readyWebUIFuture = readyWebUIPromise.get_future();

    unsigned restServerPort = m_config.restServerPort();
    const std::string& boundBackendIP = m_config.boundBackendIP();
    m_webUIService.server = std::make_unique<Network::Http::Server::WebUIBackend>(m_coreService.get());
    m_webUIService.thread.emplace([&](){
      m_webUIService.server->start(restServerPort, boundBackendIP, std::move(readyWebUIPromise));
    });

    readyWebUIFuture.wait();

    if(spawnBrowser){
      std::stringstream serviceUrlStream;
      serviceUrlStream << "http://127.0.0.1:" << m_config.restServerPort();
      std::string serviceURL = serviceUrlStream.str();
      Logger::log("Management WebUI is ready and available at ", serviceURL);

      Platform::adapter.openWebBrowser(serviceURL);
    }
  }


  void Runtime::_initChannels(
    const Serialization::Json& jsonChannels
  )
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


  void Runtime::_enableEntertainmentConfiguration(
    const std::string& entertainmentConfigurationId
  )
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

    auto pp = m_coreService->profilePath();
    Serialization::Json jsonChannels = Serialization::Json::object();

    if(std::filesystem::is_regular_file(pp)){
      Serialization::Json jsonProfile = Serialization::Json::parse(std::ifstream(pp));
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

    const auto subsampleWidth = m_config.subsampleWidth();

    if(!m_frameData.isSubsampled){
      Imaging::ImageProcessing::rescale(m_frameData, subframeImageData, subsampleWidth, m_config.interpolation());
    }

    if(m_frameData.format == Imaging::PixelFormat::RGBA){
      Imaging::ImageProcessing::rgbaToRgb(subframeImageData, subframeImageData);
    }

    for(auto& [channelId, channel] : m_channels){
      if(channel.state == Hue::Api::Channel::State::Inactive){
        continue;
      }

      auto& channelStream = m_channelStreams.at(channelId);

      if(channel.state == Hue::Api::Channel::State::PendingShutdown){
        channelStream.id = channelId;
        channelStream.r = 0;
        channelStream.g = 0;
        channelStream.b = 0;
        channel.acknowledgeShutdown();
      }
      else{
        Imaging::ImageData crop;
        Imaging::ImageProcessing::getSubImage(subframeImageData, crop, channel.uvs);
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
