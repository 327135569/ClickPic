#pragma once
#include "opencv2/core/mat.hpp"
#include "opencv2/opencv.hpp"
#include <optional>
class Screen {
  public:
    virtual std::optional<cv::Mat> fetch() = 0;
};