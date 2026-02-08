#pragma once

#include <glm/vec2.hpp>


namespace Huenicorn::Imaging
{
  using UV = glm::vec2;

  /**
   * @brief Normalized screen coordinates
   * 
   */
  struct UVs
  {
    UV min;
    UV max;
  };


  /**
   * @brief Flag to identify corner
   * 
   */
  enum UVCorner
  {
    TopLeft = 0,
    TopRight = 1,
    BottomLeft = 2,
    BottomRight = 3
  };
}
