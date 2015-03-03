//
//  Pattern.h
//  CalibrationPattern
//
//  Created by Ryohei Suda on 2014/09/23.
//  Copyright (c) 2014年 RyoheiSuda. All rights reserved.
//

#ifndef __CalibrationPattern__Pattern__
#define __CalibrationPattern__Pattern__

#include <iostream>
#include <opencv2/opencv.hpp>

class Pattern
{

private:
    int height, width, line_width;
    int slope;
    cv::Mat patterns[4];
    
public:
    Pattern(int height, int width, int line_width, int slope);
    void setParameters(int height, int width, int line_width, int slope);
    void setHeight(int height);
    void setWidth(int width);
    void setLineWidth(int line_width);
    void setSlope(int slope);
    void generate();
    void save(std::string dir);
};

#endif /* defined(__CalibrationPattern__Pattern__) */
