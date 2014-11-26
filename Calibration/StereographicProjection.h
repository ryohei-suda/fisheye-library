//
//  StereographicProjection.h
//  Calibration
//
//  Created by Ryohei Suda on 2014/11/15.
//  Copyright (c) 2014年 RyoheiSuda. All rights reserved.
//

#ifndef __Calibration__StereographicProjection__
#define __Calibration__StereographicProjection__

#include "IncidentVector.h"
#include <opencv2/opencv.hpp>

class StereographicProjection : public IncidentVector
{
public:
    StereographicProjection(cv::Point2d p);
    
    void calcM();
    void aoi(); // Calculate theta
    cv::Point3d calcDu();
    cv::Point3d calcDv();
    cv::Point3d calcDf();
    std::vector<cv::Point3d> calcDak();
};

#endif /* defined(__Calibration__StereographicProjection__) */
