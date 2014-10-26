//
//  Pattern.cpp
//  CalibrationPattern
//
//  Created by Ryohei Suda on 2014/09/23.
//  Copyright (c) 2014年 RyoheiSuda. All rights reserved.
//

#include "Pattern.h"

Pattern::Pattern(int height, int width, int line_width)
{
    setParameters(height, width, line_width);
}

void Pattern::setParameters(int height, int width, int line_width)
{
    this->height = height;
    this->width = width;
    this->line_width = line_width;
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

void Pattern::generate()
{
    // Make vertical pattern
    patterns[0] = cv::Mat::ones(height, width, CV_8UC1) * 255;
    for(int i = line_width/2; i < width; i += 2*line_width){
        cv::line(patterns[0], cv::Point2d(i, 0), cv::Point2d(i, height), cv::Scalar(0), line_width);
    }
    patterns[1] = ~patterns[0];
    
    // Make horizontal pattern
   patterns[2] = cv::Mat::ones(height, width, CV_8UC1) * 255;
    for(int i = line_width/2; i < height; i += 2*line_width){
        cv::line(patterns[2], cv::Point2d(0, i), cv::Point2d(width, i), cv::Scalar(0), line_width);
    }
    patterns[3] = ~patterns[2];

}

void Pattern::save(std::string dir)
{
    for (int i = 0; i < 4; ++i) {
        std::string name = dir + "pattern" + std::to_string(i) + ".png";
        cv::imwrite(name, patterns[i]);
    }
}