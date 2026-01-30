#pragma once

#include <Huenicorn/Platform/IAdapter.hpp>


namespace Huenicorn::Platform
{
  class WindowsAdapter : public IAdapter
  {
  public:
    WindowsAdapter():IAdapter{"Windows"}{}
    virtual std::filesystem::path getConfigFilePath() const override;
    virtual std::string getUsername() const override;
    virtual void openWebBrowser(const std::string& url) const override;

  protected:
    virtual UniqueGrabber _createGrabber(Config* config) override;
  };
}
