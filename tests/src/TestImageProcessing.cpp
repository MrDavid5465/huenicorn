#include <filesystem>
#include <fstream>
#include <unordered_map>

#include <Huenicorn/ImageProcessing.hpp>
#include <Huenicorn/ImageData.hpp>
#include <Huenicorn/Logger.hpp>
#include <Huenicorn/TimingDefinitions.hpp>


void testResize(const std::filesystem::path& samplesDirPath)
{
  std::unordered_map<Huenicorn::Interpolation::Type, std::filesystem::path> testOutputs = {
    {Huenicorn::Interpolation::Type::Nearest, "Nearest"},
    {Huenicorn::Interpolation::Type::Cubic, "Cubic"},
    {Huenicorn::Interpolation::Type::Area, "Area"}
  };

  std::filesystem::path samplesInputDirPath = samplesDirPath / "Inputs";
  std::filesystem::path samplesOutputDirPath = samplesDirPath / "Outputs";

  if(!std::filesystem::exists(samplesDirPath) || !std::filesystem::is_directory(samplesDirPath)){
    Huenicorn::Logger::error("Invalid folder ", samplesInputDirPath);
    return;
  }

  for(const auto& entry : std::filesystem::directory_iterator(samplesInputDirPath)){
    Huenicorn::ImageData imageData;
    imageData.imageMatrix = cv::imread(entry.path());

    std::filesystem::create_directory(samplesOutputDirPath);

    for(const auto& [type, interpolationOutDir] : testOutputs){
      std::filesystem::path fullpath = samplesOutputDirPath / std::filesystem::path(interpolationOutDir.string() + std::string("_") + entry.path().filename().string());

      Huenicorn::ImageData outputImageData;
      Huenicorn::ImageProcessing::rescale(imageData, outputImageData, imageData.width() / 10, type);

      Huenicorn::Logger::log("Saving ", fullpath);

      cv::imwrite(fullpath, outputImageData.imageMatrix);
    }
  }
}


void testCrop(const std::filesystem::path& samplesDirPath)
{
  std::filesystem::path samplesInputDirPath = samplesDirPath / "Inputs";
  std::filesystem::path samplesOutputDirPath = samplesDirPath / "Outputs";

  if(!std::filesystem::exists(samplesDirPath) || !std::filesystem::is_directory(samplesDirPath)){
    Huenicorn::Logger::error("Invalid folder ", samplesInputDirPath);
    return;
  }

  std::filesystem::create_directory(samplesOutputDirPath);

  for(const auto& entry : std::filesystem::directory_iterator(samplesInputDirPath)){
    Huenicorn::ImageData imageData;
    imageData.imageMatrix = cv::imread(entry.path());

    std::filesystem::path filename = entry.path().filename();
    Huenicorn::Logger::log(filename);

    std::filesystem::path interpolationOutDir = "crop";
    std::filesystem::path fullpath = samplesOutputDirPath / std::filesystem::path(interpolationOutDir.string() + std::string("_") + filename.string());

    Huenicorn::ImageData outputImageData;
    Huenicorn::ImageProcessing::getSubImage(imageData, outputImageData, glm::vec2(0, 0), glm::vec2(imageData.width() / 2, imageData.height() / 2));

    Huenicorn::Logger::log(fullpath);

    cv::imwrite(fullpath, outputImageData.imageMatrix);
  }
}


void testPerf(const std::filesystem::path& samplesDirPath)
{
  Huenicorn::ImageData imageData;

  std::filesystem::path samplesInputDirPath = samplesDirPath / "Inputs";

  const auto& entry = std::filesystem::directory_iterator(samplesInputDirPath);

  imageData.imageMatrix = cv::imread(entry->path(), cv::ImreadModes::IMREAD_COLOR_RGB);

  Huenicorn::Timing::TimePoint start = Huenicorn::Timing::ClockType::now();
  Huenicorn::Timing::TimePoint now = Huenicorn::Timing::ClockType::now();
  int i = 0;
  while(std::chrono::duration_cast<std::chrono::seconds>(now - start).count() < 3){
    Huenicorn::ImageData resampledImage;
    
    Huenicorn::ImageProcessing::rescale(imageData, resampledImage, imageData.width() / 10, Huenicorn::Interpolation::Type::Area); // 335 in 3 seconds (much better)
    //Huenicorn::ImageProcessing::rescale(imageData, resampledImage, imageData.width() / 10, Huenicorn::Interpolation::Type::Nearest); // 7472 in 3 seconds (Hahaha)
    //Huenicorn::ImageProcessing::rescale(imageData, resampledImage, imageData.width() / 10, Huenicorn::Interpolation::Type::Cubic); // 1490 in 3 seconds


    Huenicorn::ImageData croppedImage;
    Huenicorn::ImageProcessing::getSubImage(imageData, croppedImage, glm::vec2(0, 0), glm::vec2(imageData.width() / 20, imageData.height() / 20)); // 14054708 in 3 seconds

    now = Huenicorn::Timing::ClockType::now();
    i++;
  }

  Huenicorn::Logger::log(i);
}


int main(int argc, char* argv[])
{
  if(argc < 2){
    Huenicorn::Logger::error("Please provide path to images");
    return 0;
  }

  std::filesystem::path samplesDirPath(argv[1]);

  testResize(samplesDirPath);
  testCrop(samplesDirPath);

  testPerf(samplesDirPath);

  return 0;
}