//
//  Pattern.cpp
//  CalibrationPattern
//
//  Created by Ryohei Suda on 2014/09/23.
//  Copyright (c) 2014年 RyoheiSuda. All rights reserved.
//

#include "Pattern.h"

Pattern::Pattern(int height, int width, int line_width, int slope)
{
    setParameters(height, width, line_width, slope);
}

void Pattern::setParameters(int height, int width, int line_width, int slope)
{
    setHeight(height);
    setWidth(width);
    setLineWidth(line_width);
    setSlope(slope);
}

void Pattern::setHeight(int height)
{
    this->height = height;
}

void Pattern::setWidth(int width)
{
    this->width = width;
}

void Pattern::setLineWidth(int line_width)
{
    this->line_width = line_width;
}

void Pattern::setSlope(int slope)
{
    this->slope = slope;
}

void Pattern::generate()
{
    // Make vertical pattern
    patterns[0] = cv::Mat::ones(height, width, CV_8UC1) * 255;
    for(int i = line_width/2; i < width+line_width; i += 2*line_width){
        cv::line(patterns[0], cv::Point2i(i, 0), cv::Point2i(i, height), cv::Scalar(0), line_width);
    }
    patterns[1] = ~patterns[0];
    cv::blur(patterns[0], patterns[0], cv::Size2i(slope+1,1));
    cv::blur(patterns[1], patterns[1], cv::Size2i(slope+1,1));
    
    // Make horizontal pattern
   patterns[2] = cv::Mat::ones(height, width, CV_8UC1) * 255;
    for(int i = line_width/2; i < height+line_width; i += 2*line_width){
        cv::line(patterns[2], cv::Point2i(0, i), cv::Point2i(width, i), cv::Scalar(0), line_width);
    }
    patterns[3] = ~patterns[2];
    cv::blur(patterns[2], patterns[2], cv::Size2i(1,slope+1));
    cv::blur(patterns[3], patterns[3], cv::Size2i(1,slope+1));

}

void Pattern::save(std::string dir)
{
    for (int i = 0; i < 4; ++i) {
        std::string name = dir + "pattern" + std::to_string(i) + ".png";
        cv::imwrite(name, patterns[i]);
    }
}