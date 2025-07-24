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

SharedGrabber initGrabber(Config& config)
{
  try{
    auto grabber = Huenicorn::platformAdapter.getGrabber(&config);

    if(grabber){
      return grabber;
    }

    // Falling back on dummy grabber
    if(!grabber){
      grabber = std::make_unique<DummyGrabber>(&config);
      return grabber;
    }
  }
  catch(const std::exception& e){
    Logger::error(e.what());
    return nullptr;
  }

  return nullptr;
}


int main(int argc, char* argv[])
{
  if(argc < 2){
    Huenicorn::Logger::error("Please provide path to images");
    return 0;
  }

  Config config("/tmp");
  config.setSubsampleWidth(20);
  std::filesystem::path frameDirRoot(argv[1]);
  auto grabber = initGrabber(config);

  if(!grabber){
    return 0;
  }
  
  Logger::log("Started ", grabber->name());
  const auto& displayResolution = grabber->displayResolution();
  Logger::log("Screen res ", displayResolution.x, " ", displayResolution.y);

  ImageData imageData;
  Huenicorn::Timing::TimePoint start = Huenicorn::Timing::ClockType::now();
  Huenicorn::Timing::TimePoint now = Huenicorn::Timing::ClockType::now();
  
  std::filesystem::create_directory(frameDirRoot);

  int i = 0;
  while(std::chrono::duration_cast<std::chrono::seconds>(now - start).count() < 3){
    grabber->grabFrameSubsample(imageData);

    if(imageData.hasData()){
      std::string str_i = std::to_string(i++);
      std::filesystem::path filePath = frameDirRoot;
      filePath /= ("frame_" + str_i + ".png");
      Logger::log(filePath);
      cv::imwrite(filePath, imageData.imageMatrix);
    }

    Logger::log("Update");
    std::this_thread::sleep_for(0.2s);
    now = Huenicorn::Timing::ClockType::now();
  }

  Logger::log("Done");
}
