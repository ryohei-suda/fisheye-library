//
//  Reprojection.h
//  Reprojection
//
//  Created by Ryohei Suda on 2014/09/12.
//  Copyright (c) 2014年 RyoheiSuda. All rights reserved.
//

#ifndef __Reprojection__Reprojection__
#define __Reprojection__Reprojection__

#include <iostream>
#include <fstream>
#include <vector>
#include <opencv2/opencv.hpp>

class Reprojection {
public:
    int precision = 100;
    double f, f0;
    cv::Point2d center;
    cv::Size2i img_size;
    std::vector<double> a;
    std::vector<double> t2r; // theta to radius
    std::vector<double> r2t;
    double rad_step;
    
    void loadPrameters(std::string);
    void theta2radius();
    void saveTheta2Radius(std::string filename);
    void saveRadius2Theta(std::string filename);
    void calcMaps(double theta_x, double theta_y, double f_, cv::Mat& mapx, cv::Mat& mapy);};

#endif /* defined(__Reprojection__Reprojection__) */
