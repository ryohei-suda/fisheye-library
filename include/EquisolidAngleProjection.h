//
//  EquisolidAngleProjection.h
//  Calibration
//
//  Created by Ryohei Suda on 2014/11/14.
//  Copyright (c) 2014年 RyoheiSuda. All rights reserved.
//

#ifndef __Calibration__EquisolidAngleProjection__
#define __Calibration__EquisolidAngleProjection__

#include "IncidentVector.h"
#include <opencv2/opencv.hpp>

class EquisolidAngleProjection : public IncidentVector
{
private:
    cv::Point3d calcDu();
    cv::Point3d calcDv();
    cv::Point3d calcDf();
    std::vector<cv::Point3d> calcDak();
    
public:
    EquisolidAngleProjection(cv::Point2d p);
    double aoi(double r); // Calculate theta
};

#endif /* defined(__Calibration__EquisolidAngleProjection__) */
