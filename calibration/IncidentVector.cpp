//
//  incidentVector.cpp
//  Calibration
//
//  Created by Ryohei Suda on 2014/03/31.
//  Copyright (c) 2014 Ryohei Suda. All rights reserved.
//

#include "IncidentVector.h"

double IncidentVector::f, IncidentVector::f0;
std::vector<double> IncidentVector::a;
cv::Point2d IncidentVector::center;

IncidentVector::IncidentVector(cv::Point2d p)
{
    this->point = p;

    std::vector<cv::Point3d> vec;
    r = sqrt(pow(center.x-point.x, 2) + pow(center.y-point.y, 2));
    aoi();
    m.x = ((point.x - center.x) / r) * sin(theta);
    m.y = ((point.y - center.y) / r) * sin(theta);
    m.z = cos(theta);
    
}

double IncidentVector::aoi()
{
    theta = r/f0;
    
    for(int i = 0; i < a.size(); ++i) {
        theta += a[i] * pow(r/f0, 3+2*i);
    }
    theta *= f0 / (2*f);
    theta = 2 * atan(theta);
    
    return theta;
}

void IncidentVector::setParameters(double f_s, double f0_s, std::vector<double> a_s, cv::Point2d c_s)
{
    f = f_s;
    f0 = f0_s;
    a = a_s;
    center = c_s;
}


void IncidentVector::setF(double f_s)
{
    f = f_s;
}

void IncidentVector::setF0(double f0_s)
{
    f0 = f0_s;
}

void IncidentVector::setA(std::vector<double> a_s)
{
    a = a_s;
}


void IncidentVector::setCenter(cv::Point2d c_s)
{
    center = c_s;
}

cv::Point3d IncidentVector::du()
{
    return cv::Point3d(-(sin(theta)/r), 0, 0);
}
cv::Point3d IncidentVector::dv()
{
    return cv::Point3d(0, -(sin(theta)/r), 0);
}
cv::Point3d IncidentVector::df()
{
    cv::Point3d mf;
    mf.x = -(1/f)*sin(theta) * ((point.x-center.x)/r * cos(theta));
    mf.y = -(1/f)*sin(theta) * ((point.y-center.y)/r * cos(theta));
    mf.z = (1/f) * pow(sin(theta), 2);
    return mf;
}
std::vector<cv::Point3d> IncidentVector::dak()
{
    std::vector<cv::Point3d> ms;
    cv::Point3d m;
    m.x = (f0/f) * pow(cos(theta/2), 2) * ((point.x-center.x)/r * cos(theta));
    m.y = (f0/f) * pow(cos(theta/2), 2) * ((point.y-center.y)/r * cos(theta));
    m.z = (f0/f) * pow(cos(theta/2), 2) * (-sin(theta));
    
    for(int i=0; i<5; ++i) {
        ms.push_back(pow(r/f0, 2*i+3) * m);
    }
    return ms;
}