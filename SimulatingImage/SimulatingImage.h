//
//  SimulatingImage.h
//  SimulatingImage
//
//  Created by Ryohei Suda on 2014/11/25.
//  Copyright (c) 2014年 RyoheiSuda. All rights reserved.
//

#ifndef __SimulatingImage__SimulatingImage__
#define __SimulatingImage__SimulatingImage__

#include <cmath>
#include <vector>
#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>
#include <opencv2/opencv.hpp>

class SimulatingImage
{
private:
    cv::Size2d pattern_size; // Size of a rectangle pattern
    cv::Point3d corners[4]; // 4 corners after moved
    cv::Point3d units[2]; // Vectors of a pattern, 0: x, 1: y on 2D pattern
    cv::Point3d norm; // normal vector of a pattern
    double interval; // Interval of stripes
    
    // Motion parameters
    cv::Point3d pattern_center; // Center of a pattern
    double pitch[3]; // Radian of rolling, pitching, and yawing
    
    // Camera parameters
    cv::Point2d optical_center;
    double f; // Focal length
    double f0; // Scale constant
    std::vector<double> a; // Distortion parameter
    cv::Size2i img_size;
    double fov; // Field of View in radian
    
public:
    double d2r(double degree) { return degree * M_PI / 180.; } // Convert degree to radian
    double r2d(double radian) { return radian * 180 / M_PI; } // Convert radian to degree
    
    // Setter
    void setPatternSize(double width, double height) { pattern_size.width = width; pattern_size.height = height; };
    void setPatternSize(cv::Size2d size) {pattern_size = size; }
    void setInterval(double val) { interval = val; }
    void setPatternCenter(cv::Point3d center) { pattern_center = center; }
    void setPatternCenter(double x, double y, double z) {
        pattern_center.x = x;
        pattern_center.y = y;
        pattern_center.z = z;
    }
    void setPatternCenterX(double x) { pattern_center.x = x; }
    void setPatternCenterY(double y) { pattern_center.y = y; }
    void setPatternCenterZ(double z) { pattern_center.z = z; }
    void setPitchRadian(double rolling, double pitching, double yawing) {
        pitch[0] = rolling;
        pitch[1] = pitching;
        pitch[2] = yawing;
    }
    void setPitchDegree(double rolling, double pitching, double yawing) {
        pitch[0] = d2r(rolling);
        pitch[1] = d2r(pitching);
        pitch[2] = d2r(yawing);
    }
    void setRollingRadian(double rolling) { pitch[0] = rolling; }
    void setPitchingRadian(double pitching) { pitch[1] = pitching; }
    void setYawingRadian(double yawing) { pitch[2] = yawing; }
    void setRollingDegree(double rolling) { pitch[0] = d2r(rolling); }
    void setPitchingDegree(double pitching) { pitch[1] = d2r(pitching); }
    void setYawingDegree(double yawing) { pitch[2] = d2r(yawing); }
    void setOpticalCenter(cv::Point2d center) { optical_center = center; }
    void setOpticalCenter(double x, double y) { optical_center.x = x; optical_center.y = y; }
    void setFocalLength(double f_) { f = f_; }
    void setF0(double f0_) { f0 = f0_; };
    void setA(std::vector<double> a_) { a = a_; }
    void setImgSize(cv::Size2i size) { img_size = size; }
    void setImgSize(int width, int height) { img_size.width = width; img_size.height = height; }
    void setFoVRadian(double fov) { this->fov = fov/2; }
    void setFoVDegree(double fov) { this->fov = d2r(fov/2);}
    
    void calcCorners();
    cv::Point3d getRay(cv::Point2d point, int model); // model is projection type, 0: Equidistance
    bool isCross(cv::Point3d ray); // Return true if ray crosses on a (infinite) plane
    cv::Point3d calcCrossPoint(cv::Point3d ray); // Return cross point of a plane and ray
    double calcS(cv::Point3d p);
    double calcT(cv::Point3d p);
    
    cv::Mat projectPlane(int pattern); // (pattern) 0: check, 1: vertical strip, 2: inverse of 1, 3: horizontal strip, 4: inverse of 3
    void display();
};

#endif /* defined(__SimulatingImage__SimulatingImage__) */
