#include <Huenicorn/Platforms/MacOS/MacOSAdapter.hpp>

#include <memory>

#include <pwd.h>
#include <unistd.h>

#include <Huenicorn/Logger.hpp>

namespace Huenicorn
{
  std::filesystem::path MacOSAdapter::getConfigFilePath() const
  {
    const char* homeDir = getenv("HOME");
    if(!homeDir){
      passwd* pwd = getpwuid(getuid());
      homeDir = pwd->pw_dir;
    }

    return std::filesystem::path(homeDir) / "Library/Application Support/huenicorn";
  }


  std::string MacOSAdapter::getUsername() const
  {
    return getenv("USER");
  }


  SharedGrabber MacOSAdapter::_createGrabber(Config* config) const
  {
    (void)config;
    return nullptr;
  }


  void MacOSAdapter::openWebBrowser(const std::string& url) const
  {
    if(system(std::string("open " + url).c_str())){
      Logger::error("Failed to open browser");
    }
  }
};
