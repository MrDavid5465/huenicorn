#pragma once

#include <Huenicorn/IPlatformAdapter.hpp>

namespace Huenicorn
{
  class WindowsAdapter : public IPlatformAdapter
  {
  public:
    WindowsAdapter():IPlatformAdapter{"Windows"}{}
    virtual std::filesystem::path getConfigFilePath() const override;
    virtual std::string getUsername() const override;
    virtual void openWebBrowser(const std::string& url) const override;

  protected:
    virtual SharedGrabber _createGrabber(Config* config) const override;
  };
}
