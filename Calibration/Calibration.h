//
//  Calibration.h
//  Calibration
//
//  Created by Ryohei Suda on 2014/09/11.
//  Copyright (c) 2014年 RyoheiSuda. All rights reserved.
//

#ifndef __Calibration__Calibration__
#define __Calibration__Calibration__

#include <iostream>
#include <cmath>
#include <vector>
#include <fstream>
#include <sstream>
//#include <Eigen/Core>
#include <opencv2/opencv.hpp>
#include "IncidentVector.h"
#include "Pair.h"
#include "tinyxml2.h"

class Calibration {
  
public:
    //TODO Remove overlap variables with IncidentVecotr.h
//    cv::Point2d center;
//    cv::Size2i img_size;
//    double f; // focal length is pixel unit
    double C = 0.0001;
//    double const f0 = 150; // Scale constant;
//    std::vector<double> a;
    std::vector<Pair> edges;
    double J0, gamma[3];
    
    void setParameters(std::vector<Pair>& edges, double& f, double& f0, cv::Point2d& center, cv::Size2i& img_size, int a_size);
    void loadData(std::string);
    void save(std::string);
    
    void calibrate(); // Do calibrate
    
    
    //大文字の後に付いてるcは微分を表している
    double J1(); // 共線性
    double J1c(int);
    double J1cc(int, int);
    double J2(); // 平行性
    double J2c(int);
    double J2cc(int, int);
    double J3(); // 直交性
    double J3c(int);
    double J3cc(int, int);
};

#endif /* defined(__Calibration__Calibration__) */
