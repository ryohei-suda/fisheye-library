//
//  main.cpp
//  Reprojection
//
//  Created by Ryohei Suda on 2014/06/05.
//  Copyright (c) 2014年 Ryohei Suda. All rights reserved.
//

#include <iostream>
#include <opencv2/opencv.hpp>
#include "Reprojection.h"

#define PRECISION 10


int main(int argc, const char * argv[])
{
    Reprojection reproj;
    double f_;
    

    std::string param;
    std::cout << "Type parameter file name > ";
    std::cin >> param;
    reproj.loadPrameters(param);
    
    // Print parameters
    std::cout << "f: " << reproj.f << "\nf0: " << reproj.f0 << std::endl;
    std::cout << "center: " << reproj.center << std::endl;
    std::cout << "image size: " << reproj.img_size << std::endl;
    std::cout << "ai: ";
    for (std::vector<double>::iterator it = reproj.a.begin(); it != reproj.a.end(); ++it) {
        std::cout << *it << '\t';
    }
    std::cout << std::endl;
    
    reproj.theta2radius();
    
    std::string srcname;
    std::cout << "Type source image file name > ";
    std::cin >> srcname;
    cv::Mat src = cv::imread(srcname);
    cv::Mat mapx;
    cv::Mat mapy;

    std::cout << "Type prefer focal length in pixel unit > ";
    std::cin >> f_;
    
    double theta_x, theta_y;
    std::cout << "Type rotation degree around x axis > ";
    std::cin >> theta_x;
    theta_x = theta_x * M_PI /180.0;

    std::cout << "Type rotation degree around y axis > ";
    std::cin >> theta_y;
    theta_y = theta_y * M_PI /180.0;
    
    reproj.calcMaps(theta_x, theta_y, f_, mapx, mapy);
    
    cv::Mat dst;
    cv::remap(src, dst, mapx, mapy, cv::INTER_LINEAR); // Rectify
    
    cv::namedWindow("src", CV_GUI_NORMAL|CV_WINDOW_NORMAL|CV_WINDOW_KEEPRATIO);
    cv::imshow("src", src);
    cv::moveWindow("src", 0, 0);
    cv::namedWindow("dst", CV_GUI_NORMAL|CV_WINDOW_NORMAL|CV_WINDOW_KEEPRATIO| CV_GUI_EXPANDED);
    cv::imshow("dst", dst);
    cv::moveWindow("dst", 0, 0);
    cv::waitKey();
    cv::imwrite("out.png", dst);
    
    return 0;
}
