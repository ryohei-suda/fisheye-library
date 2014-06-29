//
//  incidentVector.h
//  Calibration
//
//  Created by Ryohei Suda on 2014/03/31.
//  Copyright (c) 2014 Ryohei Suda. All rights reserved.
//

#ifndef Calibration_IncidentVector_h
#define Calibration_IncidentVector_h

#include <vector>
#include <cmath>
#include <opencv2/core/core.hpp>

class IncidentVector
{
private:
    static double f, f0;
    static std::vector<double> a;
    static cv::Point2d center;
    double theta;
    double r;
    
    void aoi();
    
public:
    cv::Point3d m;
    cv::Point2d point;
    std::vector<cv::Point3d> derivatives;
    static const int U = 0, V = 1, F = 2;
    static int nparam; // Number of parameters (u0, v0, f, a1, a2, ...)
    
    IncidentVector(cv::Point2d p);
    static void setParameters(double f_s, double f0_s, std::vector<double> a_s, cv::Point2d c_s);
    static void setF(double f_s);
    static void setF0(double f0_s);
    static void setA(std::vector<double> a_s);
    static void setCenter(cv::Point2d c_s);
    static int A(int);
    cv::Point3d calcDu();
    cv::Point3d calcDv();
    cv::Point3d calcDf();
    std::vector<cv::Point3d> calcDak();
    void calcDerivatives();
    void calcM();
    
};

#endif
