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
cv::Size2i IncidentVector::img_size;
int IncidentVector::nparam = 3;
std::string IncidentVector::projection_name[PROJECTION_NUM] = {"Stereographic", "Perspective", "Equidistance", "EquisolidAngle"};
int IncidentVector::projection;

IncidentVector::IncidentVector(cv::Point2d p)
{
    point = p;
}

void IncidentVector::setParameters(double f, double f0, std::vector<double> a, cv::Size2i img_size, cv::Point2d center)
{
    IncidentVector::f = f;
    IncidentVector::f0 = f0;
    IncidentVector::a = a;
    IncidentVector::img_size = img_size;
    IncidentVector::center = center;
    
    nparam = 3 + (int)a.size();
}


void IncidentVector::setF(double f)
{
    IncidentVector::f = f;
}
double IncidentVector::getF()
{
    return IncidentVector::f;
}

void IncidentVector::setF0(double f0)
{
    IncidentVector::f0 = f0;
}
double IncidentVector::getF0()
{
    return IncidentVector::f0;
}

void IncidentVector::setA(std::vector<double> a)
{
    IncidentVector::a = a;
    IncidentVector::nparam = 3 + (int)a.size();
}
void IncidentVector::initA(int a_size)
{
    std::vector<double> a(a_size, 0);
    IncidentVector::a = a;
    IncidentVector::nparam = 3 + a_size;
}
std::vector<double> IncidentVector::getA()
{
    return IncidentVector::a;
}

void IncidentVector::setImgSize(cv::Size2i img_size)
{
    IncidentVector::img_size = img_size;
}
cv::Size2i IncidentVector::getImgSize()
{
    return IncidentVector::img_size;
}

void IncidentVector::setCenter(cv::Point2d c)
{
    IncidentVector::center = c;
}
cv::Point2d IncidentVector::getCenter()
{
    return IncidentVector::center;
}

int IncidentVector::A(int i)
{
    return 3 + i;
}

void IncidentVector::setProjection(std::string projection)
{
    for (int i = 0; i < PROJECTION_NUM; ++i) {
        if (projection_name[i] == projection) {
            IncidentVector::projection = i;
            return;
        }
    }
    exit(4);
}

int IncidentVector::getProjection()
{
    return projection;
}

std::string IncidentVector::getProjectionName()
{
    return projection_name[projection];
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