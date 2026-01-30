#pragma once

#include <opencv2/opencv.hpp>

namespace Huenicorn::Imaging
{
  struct ImageData
  {
    cv::Mat imageMatrix;

    inline int width() const
    {
      return imageMatrix.cols;
    }

    inline int height() const
    {
      return imageMatrix.rows;
    }
  
    inline bool hasData() const
    {
      return imageMatrix.data != nullptr && width() > 0 && height() > 0;
    }
  };
}
//*/
