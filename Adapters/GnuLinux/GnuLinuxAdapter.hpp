#pragma once

#include <Huenicorn/IPlatformAdapter.hpp>

namespace Huenicorn
{
  class Config;

  class GnuLinuxAdapter : public IPlatformAdapter
  {
  public:
    GnuLinuxAdapter():IPlatformAdapter{"Gnu/Linux"}{}
    virtual std::filesystem::path getConfigFilePath() const override;
    virtual std::string getUsername() const override;
    virtual void openWebBrowser(const std::string& url) const override;

  protected:
    virtual SharedGrabber _createGrabber(Config* config) const override;
  };
}
