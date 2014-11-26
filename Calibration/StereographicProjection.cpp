//
//  StereographicProjection.cpp
//  Calibration
//
//  Created by Ryohei Suda on 2014/11/15.
//  Copyright (c) 2014年 RyoheiSuda. All rights reserved.
//

#include "StereographicProjection.h"

StereographicProjection::StereographicProjection(cv::Point2d p): IncidentVector(p)
{
}

void StereographicProjection::aoi()
{
    theta = r / f0;
    
    for(int i = 0; i < a.size(); ++i) {
        theta += a[i] * pow(r/f0, 3+2*i);
    }
    theta *= f0 / (2 * f);
    theta = 2 * atan(theta);
}


cv::Point3d StereographicProjection::calcDu()
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
        tu *= - pow(cos(theta/2), 2) * (point.x-center.x) / (r * f);
        mu.x += (point.x-center.x) / r * cos(theta) * tu;
        mu.y += (point.y-center.y) / r * cos(theta) * tu;
        mu.z += -sin(theta) * tu;
        return mu;
        
    } else {
        return cv::Point3d(0, 0, 0);
    }
}

cv::Point3d StereographicProjection::calcDv()
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
        tv *= - pow(cos(theta/2), 2) * (point.y-center.y) / (r * f);
        mv.x += (point.x-center.x) / r * cos(theta) * tv;
        mv.y += (point.y-center.y) / r * cos(theta) * tv;
        mv.z += -sin(theta) * tv;
        return mv;
        
    } else {
        return cv::Point3d(0, 0, 0);
    }
}
cv::Point3d StereographicProjection::calcDf()
{
    cv::Point3d mf;
    
    if (r != 0) {
        mf.x = (point.x - center.x) / r * cos(theta);
        mf.y = (point.y - center.y) / r * cos(theta);
        mf.z = -sin(theta);
        mf *= -sin(theta) / f;
        return mf;
    } else {
        return cv::Point3d(0, 0, 0);
    }
}

std::vector<cv::Point3d> StereographicProjection::calcDak()
{
    std::vector<cv::Point3d> ms;
    if (r != 0) {
        cv::Point3d m;
        m.x = (point.x-center.x)/r * cos(theta);
        m.y = (point.y-center.y)/r * cos(theta);
        m.z = -sin(theta);
        m *= cos(theta/2) * f0 / f;
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