//
//  CameraMotion.h
//  CameraMotion
//
//  Created by Ryohei Suda on 2014/10/26.
//  Copyright (c) 2014年 RyoheiSuda. All rights reserved.
//

#ifndef __CameraMotion__CameraMotion__
#define __CameraMotion__CameraMotion__

#include <opencv2/opencv.hpp>

class CameraMotion
{
public:
    void calcOpticalFlow(cv::Mat img1, cv::Mat img2);
};

#endif /* defined(__CameraMotion__CameraMotion__) */
