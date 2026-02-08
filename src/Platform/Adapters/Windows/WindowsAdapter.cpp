#include <Huenicorn/Platform/Adapters/Windows/WindowsAdapter.hpp>

#include <memory>

#include <windows.h>
#include <lmcons.h>

#include <Huenicorn/Logger.hpp>


namespace Huenicorn::Platform
{
  std::filesystem::path WindowsAdapter::getConfigFilePath() const
  {
    const char* appData = getenv("APPDATA");
    if(!appData){
      throw std::runtime_error("Environment variable APPDATA is not set");
    }

    return std::filesystem::path(appData) / "Huenicorn";
  }


  std::string WindowsAdapter::getUsername() const
  {
    char username[UNLEN + 1];
    DWORD usernameLen = UNLEN + 1;

    if(!GetUserNameA(username, &usernameLen)){
      throw std::runtime_error("Failed to get the current username");
    }

    return std::string(username);
  }


  UniqueGrabber WindowsAdapter::_createGrabber(
    Core::Config* config
  )
  {
    (void)config;
    return nullptr;
  }


  void WindowsAdapter::openWebBrowser(
    const std::string& url
  ) const
  {
    if((uintptr_t)ShellExecuteA(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL) <= 32){
      Logger::error("Failed to open browser");
    }
  }
}
