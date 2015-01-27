//
//  main.cpp
//  Evaluation
//
//  Created by Ryohei Suda on 2015/01/11.
//  Copyright (c) 2015年 RyoheiSuda. All rights reserved.
//

#include <iostream>
#include "Evaluation.h"

int main(int argc, const char * argv[]) {
    
//    cv::Mat g(2,3, CV_64FC1);
//    cv::Point2d f(1.1964e-3,1.1970e-3);
//    double g1 = 3.4679e-3, g2 = -3.9351e-3, g3 = -3.4121e-3, g4 = 4.3182e-3;
//    g.at<double>(0, 0) = g1+g3;
//    g.at<double>(0, 1) = g4;
//    g.at<double>(0, 2) = g1;
//    g.at<double>(1, 0) = g2;
//    g.at<double>(1, 1) = g3;
//    g.at<double>(1, 2) = g2+g4;
//
//    cv::Point2d p(500, 500);
//    p.x *= f.x;
//    p.y *= f.y;
//    cv::Mat tmp(3,1,CV_64FC1);
//    tmp.at<double>(0, 0) = p.x*p.x;
//    tmp.at<double>(1, 0) = p.x*p.y;
//    tmp.at<double>(2, 0) = p.y*p.y;
//    std::cout << g*tmp << std::endl;
//    
//    double k[] = {-33.028e-3, 6.8692e-3, -1.2050e-3};
//    double r = sqrt(pow(p.x,2) + pow(p.y,2));
//    double R = 1;
//    for ( int i = 0; i < 3; ++i)
//    {
//        R += k[i]*pow(r, i*2+2);;
//    }
//    std::cout << R << std::endl;
//    cv::Mat final =g*tmp + R*cv::Mat(p);
//    std::cout <<  final.at<double>(0)/f.x << " " << final.at<double>(1)/f.y << std::endl;
    
    Evaluation e;
    
    std::string fname;
    std::cout << "Type data file name > ";
    std::cin >> fname;
    e.loadData(fname);
    
    e.projectAllPoints();
    
    return 0;
}
