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
#include <chrono>
#include <memory>
#include <future>
//#include <Eigen/Core>
#include <opencv2/opencv.hpp>
#include "IncidentVector.h"
#include "EquidistanceProjection.h"
#include "EquisolidAngleProjection.h"
#include "StereographicProjection.h"
#include "OrthographicProjection.h"
#include "Pair.h"
#include "tinyxml2.h"

class Calibration {
  
private:
    static void calcPair(Pair& pair); // For threading
    static void calcPairCC(Pair& pair); // For threading
    static double calcF(Pair& pair); // For threading
    static double calcFc(Pair& pair, int c); // For threading
    static double calcFcc(Pair& pair, int c1, int c2); // For threading
    
public:
    //TODO Remove overlap variables with IncidentVecotr.h
//    cv::Point2d center;
//    cv::Size2i img_size;
//    double f; // focal length is pixel unit
//    double const f0 = 150; // Scale constant;
//    std::vector<double> a;
    std::vector<Pair> edges;
    
    void setParameters(std::vector<Pair>& edges, double& f, double& f0, cv::Point2d& center, cv::Size2i& img_size, int a_size);
    void loadData(std::string);
    void save(std::string);
    
    void calibrate(bool divide); // Do calibrate
    
    
    // "c" means 1st or 2nd derivatives
    double J1(); // Colinearity
    double J1c(int);
    double J1cc(int, int);
    double J2(); // Parallelism
    double J2c(int);
    double J2cc(int, int);
    double J3(); // Orthogonality
    double J3c(int);
    double J3cc(int, int);
    
    // For new optimization
    double F();
    double Fc(int c);
    double Fcc(int c1, int c2);
    void calibrateNew();
};

#endif /* defined(__Calibration__Calibration__) */
