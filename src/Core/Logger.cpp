#include <Huenicorn/Core/Logger.hpp>

#include <iostream>


namespace Huenicorn::Core
{
  namespace Logger
  {
    void write(LogLevel logLevel, const std::string& message)
    {
      switch(logLevel)
      {
        case LogLevel::Message:
          std::cout << message << "\n";
          break;

        case LogLevel::Warning:
          std::cout << "[Warning] : " << message << "\n";
          break;

        case LogLevel::Error:
          std::cerr << "[Error] : " << message << "\n";
          break;

  #ifndef NDEBUG
        case LogLevel::Debug:
          std::cout << "[Debug] : " << message << "\n";
          break;
  #endif // NDEBUG

        default:
          break;
      }
    }
  }
}
