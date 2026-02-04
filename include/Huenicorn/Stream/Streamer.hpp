#pragma once

#include <string>
#include <vector>

#include <Huenicorn/Channel.hpp>
#include <Huenicorn/Hue/Auth/Credentials.hpp>
#include <Huenicorn/Stream/IDtlsClient.hpp>
#include <Huenicorn/Stream/HuestreamHeader.hpp>
#include <Huenicorn/Stream/HuestreamPayload.hpp>


namespace Huenicorn::Stream
{
  /**
   * @brief Wrapper around UDP requests to submit color data to the bridge
   * 
   */
  class Streamer
  {
    static const std::string Port;


  public:
    // Constructor
    /**
     * @brief Streamer constructor
     * 
     * @param credentials Hue bridge credentials
     * @param bridgeAddress Hue bridge address
     */
    Streamer(const Hue::Auth::Credentials& credentials, const std::string& bridgeAddress);


    // Setters
    /**
     * @brief Sets the entertainment configuration header field
     * 
     * @param entertainmentConfigurationId ID of the entertainment configuration to set to the request header
     */
    void setEntertainmentConfigurationId(const std::string& entertainmentConfigurationId);


    // Methods
    /**
     * @brief Submit the channels data to the stream
     * 
     * @param channels 
     */
    void streamChannels(const ChannelStreams& channels);


  private:
    // Attributes
    std::unique_ptr<Stream::IDtlsClient> m_dtlsClient;
    HuestreamHeader m_header;
  };
}
