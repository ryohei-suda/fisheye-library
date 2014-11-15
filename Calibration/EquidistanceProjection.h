//
//  EquidistanceProjection.h
//  Calibration
//
//  Created by Ryohei Suda on 2014/11/14.
//  Copyright (c) 2014年 RyoheiSuda. All rights reserved.
//

#ifndef __Calibration__EquidistanceProjection__
#define __Calibration__EquidistanceProjection__

#include "IncidentVector.h"
#include <opencv2/opencv.hpp>

class EquidistanceProjection : public IncidentVector
{
public:
    EquidistanceProjection(cv::Point2d p);
    
    void calcM();
    void aoi(); // Calculate theta
    cv::Point3d calcDu();
    cv::Point3d calcDv();
    cv::Point3d calcDf();
    std::vector<cv::Point3d> calcDak();
};

#endif /* defined(__Calibration__EquidistanceProjection__) */
