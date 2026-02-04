#include <Huenicorn/Stream/Streamer.hpp>

#include <Huenicorn/Stream/Impl/MbedTls/MbedTlsClient.hpp>
#include <Huenicorn/Logger.hpp>


namespace Huenicorn::Stream
{
  const std::string Streamer::Port = "2100";

  Streamer::Streamer(const Hue::Auth::Credentials& credentials, const std::string& bridgeAddress)
  {
    m_dtlsClient = std::make_unique<MbedTlsClient>(DtlsConfig{credentials, bridgeAddress, Port});
    try{
      m_dtlsClient->init();
    }
    catch(const std::exception& e){
      Logger::error(e.what());
    }
  }


  void Streamer::setEntertainmentConfigurationId(const std::string& entertainmentConfigurationId)
  {
    m_header.setEntertainmentConfigurationId(entertainmentConfigurationId);
  }


  void Streamer::streamChannels(const ChannelStreams& channels)
  {
    std::vector<std::byte> requestBuffer;
    requestBuffer.insert(requestBuffer.end(), reinterpret_cast<std::byte*>(&m_header), reinterpret_cast<std::byte*>(&m_header) + sizeof(HuestreamHeader));

    for(const auto channel : channels){
      HuestreamPayload payload;
      payload.setChannelId(channel.id);
      payload.setR(static_cast<uint16_t>(0xffff * channel.r));
      payload.setG(static_cast<uint16_t>(0xffff * channel.g));
      payload.setB(static_cast<uint16_t>(0xffff * channel.b));

      requestBuffer.insert(requestBuffer.end(), reinterpret_cast<std::byte*>(&payload), reinterpret_cast<std::byte*>(&payload) + sizeof(HuestreamPayload));
    }

    m_dtlsClient->send(requestBuffer);
    m_header.sequenceId++;
  }
}
