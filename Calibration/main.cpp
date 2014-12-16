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
#include "IncidentVector.h"
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
//    a.push_back(0.0001); a.push_back(0.00002); a.push_back(0.000003); a.push_back(0.0000004); a.push_back(0.00000005);
//    a_size = 5;
//    IncidentVector::setA(a);
//    int x = atoi(argv[2]), y = atoi(argv[3]), f = atoi(argv[4]);
//    double x = 953., y = 600., f = 401.;
//    IncidentVector::setF(f);
//    IncidentVector::setCenter(cv::Point2d(x,y));
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
//    cv::Mat img = cv::Mat::zeros(IncidentVector::getImgSize().height, IncidentVector::getImgSize().width, CV_8UC1);
//    for (int i=0; i < calib.edges.size(); ++i) {
//        for (int j=0; j < calib.edges[i].edge[0].size(); ++j) {
//            for (int k=0; k < calib.edges[i].edge[0][j].size(); ++k) {
//                img.at<uchar>(calib.edges[i].edge[0][j][k].point.y, calib.edges[i].edge[0][j][k].point.x) = 255;
//            }
//        }
//        cv::imshow("edges", img);
//        cv::waitKey();
//        for (int j=0; j < calib.edges[i].edge[1].size(); ++j) {
//            for (int k=0; k < calib.edges[i].edge[1][j].size(); ++k) {
//                img.at<uchar>(calib.edges[i].edge[1][j][k].point.y, calib.edges[i].edge[1][j][k].point.x) = 255;
//            }
//        }
//        cv::imshow("edges", img);
//        cv::waitKey();
//        img = cv::Mat::zeros(IncidentVector::getImgSize().height, IncidentVector::getImgSize().width, CV_8UC1);
//    }
//    cv::imwrite("edges.png", img);
    
//    if (std::string(argv[7]) == std::string("divide")) {
//        calib.calibrate(true);
//    } else {
//        calib.calibrate(false);
//    }
    calib.calibrate(false);
    
    std::string outname;
    std::cout << "Type output filename > ";
    std::cin >> outname;
//    std::string outname(argv[6]);
    calib.save(outname);

    
    std::cout << "END" << std::endl;
    return 0;
}