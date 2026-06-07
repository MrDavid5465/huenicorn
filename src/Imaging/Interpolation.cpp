#include <Huenicorn/Imaging/Interpolation.hpp>


namespace Huenicorn::Imaging
{
  namespace Interpolation
  {
    Interpolations availableInterpolations = {
      {"Nearest", Type::Nearest},
      {"Cubic", Type::Cubic},
      {"Area", Type::Area},
    };
  }
}
