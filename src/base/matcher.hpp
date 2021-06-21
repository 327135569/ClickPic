#pragma once
#include "opencv2/core/types.hpp"
#include "opencv2/imgproc.hpp"
#include "utils/timecalc.hpp"
#include <cstddef>
#include <opencv2/opencv.hpp>
#include <optional>
#include <stdexcept>
#include <stdio.h>

class Matcher {
  public:
    Matcher() {}
    ~Matcher() {}

    std::optional<cv::Point> matchTemplate(const std::string &name,
                                           const cv::Mat &matchscreen,
                                           const cv::Mat &matchpic) {

        // printf("1type %d 2type %d\n", matchscreen.type(), matchpic.type());

        TimeCalc calc("cost time");

        double minVal;
        double maxVal = 0;
        cv::Point minLoc;
        cv::Point maxLoc;
        cv::Mat result;
        cv::Point r;

        //匹配方式
        int match_method = cv::TM_CCOEFF_NORMED;

        if (matchpic.rows > matchscreen.rows ||
            matchpic.cols > matchscreen.cols) {
            throw std::runtime_error("Fatal: target big than screen " + name);
        }
        cv::matchTemplate(matchscreen, matchpic, result, match_method);
        cv::minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat());

        if (match_method == cv::TM_SQDIFF ||
            match_method == cv::TM_SQDIFF_NORMED) {
            r.x = minLoc.x;
            r.y = minLoc.y;
        } else {
            r.x = maxLoc.x;
            r.y = maxLoc.y;
        }

        r.x += matchpic.cols / 2;
        r.y += matchpic.rows / 2;

        printf("Match result %s %f %d %d %s\n", name.c_str(), maxVal, r.x, r.y,
               calc.finish().c_str());

        if (maxVal > 0.9) {
            return r;
        }

        return std::nullopt;
    }
};