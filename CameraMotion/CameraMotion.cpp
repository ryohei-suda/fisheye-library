//
//  CameraMotion.cpp
//  CameraMotion
//
//  Created by Ryohei Suda on 2014/10/26.
//  Copyright (c) 2014年 RyoheiSuda. All rights reserved.
//

#include "CameraMotion.h"

void CameraMotion::calcOpticalFlow(cv::Mat img1, cv::Mat img2)
{
    cv::Mat img1_grey, img2_grey;
    cv::cvtColor(img1, img1_grey, CV_BGR2GRAY);
    cv::cvtColor(img2, img2_grey, CV_BGR2GRAY);
    
    // Initialize
    std::vector<cv::Point2f> prev_pts;
    std::vector<cv::Point2f> next_pts;
    cv::Mat corners;
    
    // Find corners in the first image
    cv::goodFeaturesToTrack(img1_grey, corners, 500, 0.001, 5);
    cv:cornerSubPix(img1_grey, corners, cv::Size(3, 3), cv::Size(-1, -1),
                    cv::TermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.03));
    for (int i = 0; i < corners.rows; ++i) {
        prev_pts.push_back(cv::Point2f(corners.at<float>(i,0), corners.at<float>(i,1)));
    }
    
    // Calculate optical flow
    cv::Mat status, error;
    cv::calcOpticalFlowPyrLK(img1_grey, img2_grey, prev_pts, next_pts, status, error);
    
    // Display
    std::vector<cv::Point2f>::const_iterator p = prev_pts.begin();
    std::vector<cv::Point2f>::const_iterator n = next_pts.begin();
    for(; n!=next_pts.end(); ++n,++p) {
        cv::circle(img1, *p, 3, cv::Scalar(0,255,0));
        cv::line(img1, *p, *n, cv::Scalar(255,0,0), 2);
        cv::circle(img2, *n, 3, cv::Scalar(0,255,0));
        cv::line(img2, *p, *n, cv::Scalar(255,0,0), 2);
    }
    
    cv::namedWindow("optical flow", CV_WINDOW_AUTOSIZE|CV_WINDOW_FREERATIO);
    while (true) {
        cv::imshow("optical flow", img1);
        if(cv::waitKey() == 'q') { break; }
        
        cv::imshow("optical flow", img2);
        if(cv::waitKey() == 'q') { break; }
    }
    
}