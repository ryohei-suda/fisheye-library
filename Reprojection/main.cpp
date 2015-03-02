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

int main(int argc, const char * argv[])
{
    for (int i = 1; i < argc; ++i) {
        Reprojection reproj;
        std::string fname(argv[i]);
        reproj.loadPrameters(fname);
        reproj.theta2radius();
        fname.replace(fname.size()-3, fname.size(), "dat");
        reproj.saveTheta2Radius(fname);
    }
    if (argc > 1) {
        return 0;
    }
    
    Reprojection reproj;
    double f_;

    std::string param;
    std::cout << "Type parameter file name > ";
    std::cin >> param;
    reproj.loadPrameters(param);
    
    // Print parameters
    std::cout << "f: " << IncidentVector::getF() << "\nf0: " << IncidentVector::getF0() << std::endl;
    std::cout << "center: " << IncidentVector::getCenter() << std::endl;
    std::cout << "image size: " << IncidentVector::getImgSize() << std::endl;
    std::cout << "ai: ";
    for (auto &ai : IncidentVector::getA()) {
        std::cout << ai << '\t';
    }
    std::cout << std::endl;
    
    reproj.theta2radius();
    reproj.saveRadius2Theta("graph.dat");
    
    std::string srcname;
    std::cout << "Type source image file name > ";
    std::cin >> srcname;
    cv::Mat src = cv::imread(srcname);
    cv::Mat mapx;
    cv::Mat mapy;

    std::cout << "Type prefer focal length in pixel unit > ";
    std::cin >> f_;
    
    double theta_x, theta_y, theta_z;
    std::cout << "Type rotation degree around x axis > ";
    std::cin >> theta_x;
    theta_x = theta_x * M_PI /180.0;

    std::cout << "Type rotation degree around y axis > ";
    std::cin >> theta_y;
    theta_y = theta_y * M_PI /180.0;
    
    std::cout << "Type rotation degree around z axis > ";
    std::cin >> theta_z;
    theta_z = theta_z * M_PI /180.0;
    
    reproj.calcMaps(0, 0, theta_x, theta_y, theta_z, f_, mapx, mapy);
    
    cv::Mat dst;
    cv::remap(src, dst, mapx, mapy, cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(0,0,0)); // Rectify
    cv::namedWindow("src", CV_GUI_NORMAL);
    cv::imshow("src", src);
    cv::moveWindow("src", 0, 0);
    cv::namedWindow("dst", CV_GUI_NORMAL);
    cv::imshow("dst", dst);
    cv::moveWindow("dst", 0, 0);
    cv::waitKey();
    cv::imwrite("out.png", dst);
    
    return 0;
}
