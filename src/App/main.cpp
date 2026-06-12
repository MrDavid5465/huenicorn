#include <csignal>
#include <memory>
#include <thread>

#include <Huenicorn/App/CommandLine.hpp>
#include <Huenicorn/Core/Runtime.hpp>
#include <Huenicorn/Core/Logger.hpp>
#include <Huenicorn/Platform/Selector.hpp>
#include <Huenicorn/Version.hpp>


/**
 * @brief Wrapper around threaded application
 * 
 */
class Application
{
public:
  void start()
  {
    m_core = std::make_unique<Huenicorn::Core::Runtime>(Huenicorn::Version, Huenicorn::Platform::adapter.getConfigFilePath());
    m_applicationThread.emplace([&](){
      m_core->start();
    });

    m_applicationThread.value().join();
    m_applicationThread.reset();
    m_core.reset();
  }


  void stop()
  {
    if(!m_core){
      return;
    }

    m_core->stop();
  }


private:
  std::unique_ptr<Huenicorn::Core::Runtime> m_core;
  std::optional<std::thread> m_applicationThread;
};


Application app;


void signalHandler(
  int signal
)
{
  if(signal == SIGTERM || signal == SIGINT){
    Huenicorn::Core::Logger::log("Closing application");
    app.stop();
  }
}


int main(
  int argc,
  char* argv[]
)
{
  Huenicorn::App::CommandLine::Params params;
  Huenicorn::App::CommandLine::getOpt(argc, argv, params);
  if(params.shouldStop){
    return 0;
  }

  Huenicorn::Core::Logger::log("Starting Huenicorn version ", Huenicorn::Version, " for ", Huenicorn::Platform::adapter.getPlatformName());

  signal(SIGTERM, signalHandler);
  signal(SIGINT, signalHandler);

  app.start();
  Huenicorn::Core::Logger::log("Huenicorn terminated properly");

  return 0;
}
