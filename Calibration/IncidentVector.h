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
    static double f; // Focal length (pixel unit)
    static double f0; // Scale constant
    static std::vector<double> a; // Distortion parameters
    static cv::Point2d center; // Optical center
    static cv::Size2i img_size; // Image size
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
    static void setParameters(double f, double f0, std::vector<double> a, cv::Size2i img_size, cv::Point2d center);
    static void setF(double f);
    static double getF();
    static void setF0(double f0);
    static double getF0();
    static void setA(std::vector<double> a);
    static void initA(int a_size);
    static std::vector<double> getA();
    static void setImgSize(cv::Size2i img_size);
    static cv::Size2i getImgSize();
    static void setCenter(cv::Point2d c);
    static cv::Point2d getCenter();
    static int A(int);
    cv::Point3d calcDu();
    cv::Point3d calcDv();
    cv::Point3d calcDf();
    std::vector<cv::Point3d> calcDak();
    void calcDerivatives();
    void calcM();
    
};

#endif
