#pragma once

#include <cstdint>
#include <string>


namespace Huenicorn::Stream
{
  struct HuestreamHeader
  {
    char protocolName[9] = {'H', 'u', 'e', 'S', 't', 'r', 'e', 'a', 'm'};
    char version[2] = {0x02, 0x00};
    char sequenceId = 0;
    char reserved1[2] = {0, 0};
    char colorSpace = 0x00;
    char reserved2 = 0;
    char entertainmentConfigurationId[36];


    /**
     * @brief Sets the colorSpace field
     * 
     * @param _colorSpace 0 for RGB, 1 for XY
     */
    void setColorSpace(
      char _colorSpace
    );


    /**
     * @brief Sets the entertainmentConfigurationId field
     * 
     * @param _entertainmentConfigurationId Id of the entertainment configuration to set
     */
    void setEntertainmentConfigurationId(
      const std::string& entertainmentConfigurationId
    );
  };
}
