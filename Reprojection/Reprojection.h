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
#include "../libs/IncidentVector.h"
#include "../libs/OrthographicProjection.h"
#include "../libs/StereographicProjection.h"
#include "../libs/EquisolidAngleProjection.h"
#include "../libs/EquidistanceProjection.h"

class Reprojection {
public:
    static int precision;
    std::vector<double> t2r; // theta to radius
    std::vector<double> r2t;
    double rad_step;
    std::string projection;
    
    void loadPrameters(std::string);
    void theta2radius();
    void saveTheta2Radius(std::string filename);
    void saveRadius2Theta(std::string filename);
    void calcMaps(int x, int y, double theta_x, double theta_y, double theta_z, double f_, cv::Mat& mapx, cv::Mat& mapy);};

#endif /* defined(__Reprojection__Reprojection__) */
