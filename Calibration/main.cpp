//
//  main.cpp
//  Calibration
//
//  Created by Ryohei Suda on 2014/03/30.
//  Copyright (c) 2014 Ryohei Suda. All rights reserved.
//

#include <iostream>
#include <cmath>
#include <vector>
#include <fstream>
#include <sstream>
#include <opencv2/opencv.hpp>
#include "Pair.h"
#include "../libs/tinyxml2.h"
#include "Calibration.h"

int main(int argc, const char * argv[])
{
    Calibration calib;
    
//    std::string filename(argv[1]);
    
    std::string filename;
    std::cout << "Type filename > ";
    std::cin >> filename;
    calib.loadData(filename);
//    int a_size = atoi(argv[5]);
    int a_size;
    std::cout << "Type correction degree > ";
    std::cin >> a_size;
    IncidentVector::initA(a_size);
//    std::vector<double> a;
//    a.push_back(5e-3); a.push_back( 6e-4); a.push_back( 7e-5); a.push_back( 8e-6); a.push_back( 9e-7);
//    a_size = 5;
//    IncidentVector::setA(a);
//    int x = atoi(argv[2]), y = atoi(argv[3]), f = atoi(argv[4]);
//    double x = 953., y = 600., f = 401.;
//    IncidentVector::setF(f);
//    IncidentVector::setCenter(cv::Point2d(793,606));
//    IncidentVector::setF0((int)f);
    
    std::cout << "Projection Model:\t" << IncidentVector::getProjectionName() << std::endl;
    std::cout << "Center:\t" << IncidentVector::getCenter() << std::endl;
    std::cout << "     f:\t" << IncidentVector::getF() << std::endl;
    for (int i = 0; i < IncidentVector::nparam-3; ++i) {
        std::cout << "    a" << i << ":\t" << IncidentVector::getA().at(i) << std::endl;
    }
    
    std::cout << "Orthogonal pairs: " << calib.edges.size() << std::endl;
    long lines = 0;
    long points = 0;
    for (auto &pair : calib.edges) {
        lines += pair.edge[0].size() + pair.edge[1].size();
        for (auto &line : pair.edge[0]) {
            points += line.size();
        }
        for (auto &line : pair.edge[1]) {
            points += line.size();
        }
    }
    std::cout << "Lines: " << lines << std::endl;
    std::cout << "Points: " << points << std::endl;

    
    // Show an image of all edges
//    cv::Mat img = cv::Mat::zeros(IncidentVector::getImgSize().height, IncidentVector::getImgSize().width, CV_8UC3);
//    cv::Vec3b color[30] = {cv::Vec3b(255,255,255), cv::Vec3b(255,0,0), cv::Vec3b(255,255,0), cv::Vec3b(0,255,0), cv::Vec3b(0,0,255),
//        cv::Vec3b(255,0,255), cv::Vec3b(204,51,51), cv::Vec3b(204,204,51), cv::Vec3b(51,204,51), cv::Vec3b(51,204,204),
//        cv::Vec3b(51,51,204), cv::Vec3b(204,51,204), cv::Vec3b(204,204,204), cv::Vec3b(153,102,102), cv::Vec3b(153,153,102),
//        cv::Vec3b(102,153,102), cv::Vec3b(102,153,153), cv::Vec3b(102,102,153), cv::Vec3b(153,102,153), cv::Vec3b(153,153,153),
//        cv::Vec3b(51,51,204), cv::Vec3b(204,51,204), cv::Vec3b(204,204,204), cv::Vec3b(153,102,102), cv::Vec3b(153,153,102),
//        cv::Vec3b(102,153,102), cv::Vec3b(102,153,153), cv::Vec3b(102,102,153), cv::Vec3b(153,102,153), cv::Vec3b(153,153,153),
//    };
//    cv::namedWindow("lines", CV_WINDOW_NORMAL);
//    int j = 0;
//    for (auto &pair : calib.edges) {
//        for (int i = 0; i < 2; ++i) {
//            for (auto &line : pair.edge[i]) {
//                for (auto &point : line) {
//                    img.at<cv::Vec3b>(int(point->point.y), int(point->point.x)) = color[j%30];
//                }
//            }
//        }
//        cv::imshow("lines", img);
//        cv::waitKey();
//        ++j;
//    }
//        cv::imshow("edges", img);
//        cv::waitKey();
//        img = cv::Mat::zeros(IncidentVector::getImgSize().height, IncidentVector::getImgSize().width, CV_8UC1);
//    cv::imwrite("lines.png", img);
    
//    if (std::string(argv[7]) == std::string("divide")) {
//        calib.calibrate(true);
//    } else {
//        calib.calibrate(false);
//    }

    IncidentVector::initA(0);
    calib.calibrate(false);
    IncidentVector::initA(a_size);
    calib.calibrate(false);
//    calib.calibrate(true);
//    calib.calibrate2();
    
    std::string outname;
    std::cout << "Type output filename > ";
    std::cin >> outname;
//    std::string outname(argv[6]);
    calib.save(outname);
    
//    calib.calibrate(true);
//    calib.save(std::string("d_")+outname);

    
    std::cout << "END" << std::endl;
    return 0;
}