#include <string>
#include <thread>
#include <chrono>

#include <Huenicorn/PlatformSelector.hpp>
#include <Huenicorn/Config.hpp>
#include <Huenicorn/ImageData.hpp>
#include <Huenicorn/Logger.hpp>
#include <Huenicorn/DummyGrabber.hpp>

using namespace Huenicorn;
using namespace std::chrono_literals;


void testGrabber(const std::filesystem::path& frameDirRoot)
{
  Config config("/tmp");
  config.setSubsampleWidth(20);
  auto grabber = Huenicorn::platformAdapter.getGrabber(&config);
  Logger::log("Started ", grabber->name());

  {
    const auto& monitors = grabber->monitors();
    unsigned primaryId;

    int i = 0;
    for(const auto& monitor : monitors){
      Logger::log(monitor->name);

      if(monitor.get()->isPrimary){
        primaryId = i;
      }

      i++;
    }
    grabber->selectMonitor(primaryId);
  }

  try{
    const auto& displayResolution = grabber->displayResolution();
    Logger::log("Screen res ", displayResolution.x, " ", displayResolution.y);
  }
  catch(const std::exception& e){
    Logger::error(e.what());
  }

  ImageData imageData;

  std::filesystem::create_directory(frameDirRoot);

  int i = 0;
  Huenicorn::Timing::TimePoint start = Huenicorn::Timing::ClockType::now();
  Huenicorn::Timing::TimePoint now = Huenicorn::Timing::ClockType::now();

  int frameDurations = 0;
  int count = 0;

  while(std::chrono::duration_cast<std::chrono::seconds>(now - start).count() < 20){
    Huenicorn::Timing::TimePoint iterationStart = Huenicorn::Timing::ClockType::now();
    const auto& monitors = grabber->monitors();

    if(monitors.size()){
      //grabber->selectMonitor(i % monitors.size()); // Let's get mean !
    }

    grabber->grabFrameSubsample(imageData);

    if(imageData.hasData()){
      std::string str_i = std::to_string(i++);
      std::filesystem::path filePath = frameDirRoot;
      filePath /= ("frame_" + str_i + ".png");
      Logger::log(filePath);
      cv::imwrite(filePath, imageData.imageMatrix);
    }

    //Logger::log("Update");
    std::this_thread::sleep_for(0.1s);
    now = Huenicorn::Timing::ClockType::now();
    frameDurations += std::chrono::duration_cast<std::chrono::milliseconds>(now - iterationStart).count();
    count++;
  
    // 150.708 FPS with new way of checking
    // 130.447 FPS without monitor safety
    // 65.9715 with safety...
  }

  Logger::log("Average FPS : ", (1.0f / (static_cast<float>(frameDurations) / count)) * 1000);

}


void testMonitorEvents()
{
  Config config("/tmp");
  config.setSubsampleWidth(20);
  auto grabber = Huenicorn::platformAdapter.getGrabber(&config);
  Logger::log("Started ", grabber->name());
  std::this_thread::sleep_for(20s);
}


int main(int argc, char* argv[])
{
  if(argc < 2){
    Huenicorn::Logger::error("Please provide path to images");
    return 0;
  }

  std::filesystem::path frameDirRoot(argv[1]);

  testGrabber(frameDirRoot);
  //testMonitorEvents();
  Logger::log("Done");

  return 0;
}
