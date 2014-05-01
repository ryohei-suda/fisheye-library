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
    cv::Point2d point;
    cv::Point3d m;
    double theta;
    double r;
    
    double aoi();
    
public:
    IncidentVector(cv::Point2d p);
    static void setParameters(double f_s, double f0_s, std::vector<double> a_s, cv::Point2d c_s);
    static void setF(double f_s);
    static void setF0(double f0_s);
    static void setA(std::vector<double> a_s);
    static void setCenter(cv::Point2d c_s);
    cv::Point3d du();
    cv::Point3d dv();
    cv::Point3d df();
    cv::vector<cv::Point3d> dak();
    
    
};

#endif
