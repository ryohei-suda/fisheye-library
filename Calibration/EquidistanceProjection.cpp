//
//  EquidistanceProjection.cpp
//  Calibration
//
//  Created by Ryohei Suda on 2014/11/14.
//  Copyright (c) 2014年 RyoheiSuda. All rights reserved.
//

#include "EquidistanceProjection.h"

EquidistanceProjection::EquidistanceProjection(cv::Point2d p): IncidentVector(p)
{
}

void EquidistanceProjection::calcM()
{
    r = sqrt(pow(center.x-point.x, 2) + pow(center.y-point.y, 2));
    aoi();
    if (r != 0) {
        m.x = ((point.x - center.x) / r) * sin(theta);
        m.y = ((point.y - center.y) / r) * sin(theta);
        m.z = cos(theta);
    } else {
        m.x = 0;
        m.y = 0;
        m.z = 1;
    }
}

void EquidistanceProjection::aoi()
{
    theta = r / f0;
    
    for(int i = 0; i < a.size(); ++i) {
        theta += a[i] * pow(r/f0, 3+2*i);
    }
    theta *= f0 / f;
}


cv::Point3d EquidistanceProjection::calcDu()
{
    if (r != 0) {
        cv::Point3d mu;
        mu.x = -1/r + pow(point.x-center.x, 2) / pow(r, 3);
        mu.y = (point.x-center.x) * (point.y-center.y) / pow(r, 3);
        mu.z = 0;
        mu *= sin(theta);
        double tu = 1; // derivative of d(theta)/du
        for (int i = 0; i < a.size(); ++i) {
            tu += (2*i+3) * a[i] * pow(r/f0, 2*i+2);
        }
        tu *= -(point.x-center.x) / (r * f);
        mu.x += (point.x-center.x) / r * cos(theta) * tu;
        mu.y += (point.y-center.y) / r * cos(theta) * tu;
        mu.z += -sin(theta) * tu;
        return mu;
        
    } else {
        return cv::Point3d(0, 0, 0);
    }
}

cv::Point3d EquidistanceProjection::calcDv()
{
    if (r != 0) {
        cv::Point3d mv;
        mv.x = (point.x-center.x) * (point.y-center.y) / pow(r, 3);
        mv.y = -1/r + pow(point.y-center.y, 2) / pow(r, 3);
        mv.z = 0;
        mv *= sin(theta);
        double tv = 1; // derivative of d(theta)/dv
        for (int i = 0; i < a.size(); ++i) {
            tv += (2*i+3) * a[i] * pow(r/f0, 2*i+2);
        }
        tv *= -(point.y-center.y) / (r * f);
        mv.x += (point.x-center.x) / r * cos(theta) * tv;
        mv.y += (point.y-center.y) / r * cos(theta) * tv;
        mv.z += -sin(theta) * tv;
        return mv;
        
    } else {
        return cv::Point3d(0, 0, 0);
    }
}
cv::Point3d EquidistanceProjection::calcDf()
{
    cv::Point3d mf;
    
    if (r != 0) {
        mf.x = (point.x - center.x) / r * cos(theta);
        mf.y = (point.y - center.y) / r * cos(theta);
        mf.z = -sin(theta);
        mf *= -1/f * theta;
        return mf;
    } else {
        return cv::Point3d(0, 0, 0);
    }
}

std::vector<cv::Point3d> EquidistanceProjection::calcDak()
{
    std::vector<cv::Point3d> ms;
    if (r != 0) {
        cv::Point3d m;
        m.x = (f0/f) * ((point.x-center.x)/r * cos(theta));
        m.y = (f0/f) * ((point.y-center.y)/r * cos(theta));
        m.z = (f0/f) * (-sin(theta));
        for(int i=0; i<a.size(); ++i) {
            ms.push_back(pow(r/f0, 2*i+3) * m);
        }
    } else {
        for (int i = 0; i < a.size(); ++i) {
            ms.push_back(cv::Point3d(0,0,0));
        }
    }
    
    return ms;
}

