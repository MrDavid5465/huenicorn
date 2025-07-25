#pragma once

#include <filesystem>
#include <memory>

#include <Huenicorn/IGrabber.hpp>
#include <Huenicorn/DummyGrabber.hpp>
#include <Huenicorn/Logger.hpp>

namespace Huenicorn
{
  using UniqueGrabber = std::unique_ptr<IGrabber>;

  /**
   * @brief Wrapper interface for platform-specific functions and grabber selection
   * 
   */
  class IPlatformAdapter
  {
  public:
    /**
     * @brief Constructor
     * 
     */
    IPlatformAdapter(const std::string& platformName):
    m_platformName(platformName)
    {}


    /**
     * @brief Destructor
     * 
     */
    virtual ~IPlatformAdapter(){}


    /**
     * @brief Getter for the platform name
     * 
     */
    const std::string& getPlatformName() const
    {
      return m_platformName;
    }


    /**
     * @brief Getter for config file configuration path
     * 
     * @return std::filesystem::path Path to config path
     */
    virtual std::filesystem::path getConfigFilePath() const = 0;

    /**
     * @brief Getter for OS username
     * 
     * @return std::string OS username
     */
    virtual std::string getUsername() const = 0;


    /**
     * @brief Method to open default web browser at given URL
     * 
     * @param url Page to open in web browser
     */
    virtual void openWebBrowser(const std::string& url) const = 0;


    /**
     * @brief Getter to instanciate / store valid grabber
     * 
     * @param config Huenicorn current configuration
     * @return UniqueGrabber Grabber instance
     */
    inline IGrabber* getGrabber(Config* config)
    {
      if(!m_grabber){
        try{
          m_grabber = _createGrabber(config);
          m_grabber->init();
        }
        catch(const std::exception& e){
          // Fallback to DummyGrabber
          Logger::warn(e.what());
          Logger::warn("Could not start propper grabber. Now falling back to DummyGrabber.");

          m_grabber = std::make_unique<DummyGrabber>(config);
        }
      }

      return m_grabber.get();
    }


  protected:
    /**
     * @brief Factory method for grabber
     * 
     * @param config Huenicorn current configuration
     * @return UniqueGrabber New grabber
     */
    virtual UniqueGrabber _createGrabber(Config* config) = 0;

    // Attributes
    const std::string m_platformName;
    UniqueGrabber m_grabber;
  };
}
