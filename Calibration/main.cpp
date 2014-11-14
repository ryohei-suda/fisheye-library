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
    
    std::string filename; // "/Users/ryohei/Dropbox/univ/lab/Images/data.dat"
    std::cout << "Type filename > ";
    std::cin >> filename;
    calib.loadData(filename);
    IncidentVector::setF0(150);
    std::cout << "Type correction degree > ";
    int a_size;
    std::cin >> a_size;
    IncidentVector::initA(a_size);
//    std::vector<double> a;
//    a.push_back(0.00629485063263903); a.push_back(0.0000933288514046128); a.push_back(-0.000037316107836422); a.push_back(0.0000025494474188023); a.push_back(-5.12895465870539E-08);
//    a_size = 5;
//    IncidentVector::setA(a);
//    IncidentVector::setF(421.396310797531);
//    IncidentVector::setCenter(cv::Point2d(700,500));
    
    std::cout << "Center:\t" << IncidentVector::getCenter() << std::endl;
    std::cout << "     f:\t" << IncidentVector::getF() << std::endl;
    for (int i = 0; i < a_size; ++i) {
        std::cout << "    a" << i << ":\t" << IncidentVector::getA().at(i) << std::endl;
    }
    
    std::cout << "Orthogonal pairs: " << calib.edges.size() << std::endl;
    double lines = 0;
    for (std::vector<Pair>::iterator it = calib.edges.begin(); it != calib.edges.end(); ++it) {
        lines += it->edge[0].size();
        lines += it->edge[1].size();
    }
    std::cout << "Lines: " << lines << std::endl;

    
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
    
//    calib.calibrate(false);
    calib.calibrate(true);
    calib.save("parameters.xml");

    
    std::cout << "END" << std::endl;
    return 0;
}