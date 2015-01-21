//
//  main.cpp
//  CameraMotion
//
//  Created by Ryohei Suda on 2014/10/26.
//  Copyright (c) 2014年 RyoheiSuda. All rights reserved.
//

#include <iostream>
#include "CameraMotion.h"

int main(int argc, const char * argv[]) {
    
    
    cv::Mat src = cv::imread("/Users/ryoheisuda/Dropbox/univ/lab/素材/grid04.png");
    cv::Mat corners;
    bool found = cv::findChessboardCorners(src, cv::Size2i(9,6), corners);
    cv::drawChessboardCorners(src, cv::Size2i(9,6), corners, found);
    cv::imshow("pattern", src);
    cv::waitKey();
    
    CameraMotion cm;
    
    cv::Mat img1, img2;
    std::string name1, name2;
    std::cout << "Type a first image name > ";
    std::cin >> name1;
    img1 = cv::imread(name1, CV_LOAD_IMAGE_COLOR);
    
    std::cout << "Type a second image name > ";
    std::cin >> name2;
    img2 = cv::imread(name2, CV_LOAD_IMAGE_COLOR);
    
    cm.calcOpticalFlow(img1, img2);
    
    return 0;
}
