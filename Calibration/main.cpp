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
//#include <Eigen/Core>
#include <opencv2/opencv.hpp>
#include "IncidentVector.h"
#include "Pair.h"
#include "tinyxml2.h"
#include "Calibration.h"

int main(int argc, const char * argv[])
{
    Calibration calib;
    
    std::string filename; // "/Users/ryohei/Dropbox/univ/lab/Images/data.dat"
    std::cout << "Type filename > ";
    std::cin >> filename;
    calib.loadData(filename);
    IncidentVector::setF0(150);
    std::cout << "Type corection degree > ";
    int a_size;
    std::cin >> a_size;
    IncidentVector::initA(a_size);
    
    std::cout << "Orthogonal pairs: " << calib.edges.size() << std::endl;
    double lines = 0;
    for (std::vector<Pair>::iterator it = calib.edges.begin(); it != calib.edges.end(); ++it) {
        lines += it->edge[0].size();
        lines += it->edge[1].size();
    }
    std::cout << "Lines: " << lines << std::endl;

    /*
    // Show an image of all edges
    cv::Mat img = cv::Mat::zeros(calib.img_size.height, calib.img_size.width, CV_8UC1);
    cv::namedWindow("edges", CV_WINDOW_NORMAL);
    for (int i=0; i < calib.edges.size(); ++i) {
        for (int j=0; j < calib.edges[i].edge[0].size(); ++j) {
            for (int k=0; k < calib.edges[i].edge[0][j].size(); ++k) {
                img.at<uchar>(calib.edges[i].edge[0][j][k].point.y, calib.edges[i].edge[0][j][k].point.x) = 255;
            }
            cv::imshow("edges", img);
            cv::waitKey();
        }
        for (int j=0; j < calib.edges[i].edge[1].size(); ++j) {
            for (int k=0; k < calib.edges[i].edge[1][j].size(); ++k) {
                img.at<uchar>(calib.edges[i].edge[1][j][k].point.y, calib.edges[i].edge[1][j][k].point.x) = 255;
            }
            cv::imshow("edges", img);
            cv::waitKey();
        }
        img = cv::Mat::zeros(calib.img_size.height, calib.img_size.width, CV_8UC1);
    }
    cv::imwrite("edges.png", img);
    */
    
    calib.calibrate();
    calib.save("parameters.xml");

    
    std::cout << "END" << std::endl;
    return 0;
}