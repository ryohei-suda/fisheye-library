//
//  OrthographicProjection.cpp
//  Calibration
//
//  Created by Ryohei Suda on 2015/01/25.
//  Copyright (c) 2015年 RyoheiSuda. All rights reserved.
//

#include "OrthographicProjection.h"


OrthographicProjection::OrthographicProjection(cv::Point2d p): IncidentVector(p)
{
}

double OrthographicProjection::aoi(double r)
{
    double t;
    t = r / f0;
    
    for(int i = 0; i < a.size(); ++i) {
        t += a[i] * pow(r/f0, 3+2*i);
    }
    t *= f0 / f;
    t = asin(t);
    
    return t;
}

cv::Point3d OrthographicProjection::calcDu()
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
        tu *= -(point.x-center.x) / (r * f * cos(theta));
        mu += part * tu;
        return mu;
        
    } else {
        return cv::Point3d(0, 0, 0);
    }
}

cv::Point3d OrthographicProjection::calcDv()
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
        tv *= -(point.y-center.y) / (r * f * cos(theta));
        mv += part * tv;
        return mv;
        
    } else {
        return cv::Point3d(0, 0, 0);
    }
}
cv::Point3d OrthographicProjection::calcDf()
{
    cv::Point3d mf;
    
    if (r != 0) {
        mf = part * (-1/f * tan(theta));
        return mf;
    } else {
        return cv::Point3d(0, 0, 0);
    }
}

std::vector<cv::Point3d> OrthographicProjection::calcDak()
{
    std::vector<cv::Point3d> ms;
    if (r != 0) {
        cv::Point3d m;
        m = part * (f0 / (f * cos(theta)));
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