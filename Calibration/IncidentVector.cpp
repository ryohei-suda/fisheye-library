//
//  incidentVector.cpp
//  Calibration
//
//  Created by Ryohei Suda on 2014/03/31.
//  Copyright (c) 2014 Ryohei Suda. All rights reserved.
//

#include "IncidentVector.h"
#include <iostream>

double IncidentVector::f, IncidentVector::f0;
std::vector<double> IncidentVector::a;
cv::Point2d IncidentVector::center;
int IncidentVector::nparam = 3;

IncidentVector::IncidentVector(cv::Point2d p)
{
    point = p;
}

void IncidentVector::calcM()
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

void IncidentVector::aoi()
{
    theta = r / f0;
    
    for(int i = 0; i < a.size(); ++i) {
        theta += a[i] * pow(r/f0, 3+2*i);
    }
    theta *= f0 / f;
}

void IncidentVector::setParameters(double f_s, double f0_s, std::vector<double> a_s, cv::Point2d c_s)
{
    f = f_s;
    f0 = f0_s;
    a = a_s;
    center = c_s;
    
    nparam = 3 + (int)a.size();
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
    
    nparam = 3 + (int)a.size();
}


void IncidentVector::setCenter(cv::Point2d c_s)
{
    center = c_s;
}


int IncidentVector::A(int i)
{
    return 3 + i;
}

cv::Point3d IncidentVector::calcDu()
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

cv::Point3d IncidentVector::calcDv()
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
cv::Point3d IncidentVector::calcDf()
{
    cv::Point3d mf;
    
    if (r != 0) {
        mf.x = (point.x - center.x) / r * cos(theta);
        mf.y = (point.y - center.y) / r * cos(theta);
        mf.z = -sin(theta);
    } else {
        mf.x = 0;
        mf.y = 0;
        mf.z = -sin(theta);
    }
    double tf = r/f0; // derivative of d(theta)/df
    for (int i = 0; i < a.size(); ++i) {
        tf += a[i] * pow(r/f0, 2*i+3);
    }
    tf *=  - (f0 / pow(f,2));
    
    mf *= tf;
    return mf;
}

std::vector<cv::Point3d> IncidentVector::calcDak()
{
    std::vector<cv::Point3d> ms;
    cv::Point3d m;
    if (r != 0) {
        m.x = (f0/f) * ((point.x-center.x)/r * cos(theta));
        m.y = (f0/f) * ((point.y-center.y)/r * cos(theta));
    } else {
        m.x = 0;
        m.y = 0;
    }
    m.z = (f0/f) * (-sin(theta));
    for(int i=0; i<a.size(); ++i) {
        ms.push_back(pow(r/f0, 2*i+3) * m);
    }
    
    return ms;
}


void IncidentVector::calcDerivatives()
{
    derivatives.clear();
    derivatives.push_back(calcDu());
    derivatives.push_back(calcDv());
    derivatives.push_back(calcDf());
    std::vector<cv::Point3d> dak = calcDak();
    derivatives.insert(derivatives.end(), dak.begin(), dak.end());
    
}