//
//  main.cpp
//  CornerDetection
//
//  Created by Ryohei Suda on 2014/03/23.
//  Copyright (c) 2014年 Ryohei Suda. All rights reserved.
//

#include <iostream>
#include <vector>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "CornerDetection.h"

int main(int argc, const char * argv[])
{
    
    std::cout << "Type list file name of calibration imgages > ";
    std::string fname;
    std::cin >> fname;
    CornerDetection cd;
    cd.loadImageXML(fname);
    
    std::vector<CornerDetection::pair>::iterator pair = cd.image_names.begin();
    for (; pair != cd.image_names.end(); ++pair) {
        cv::Mat white = cv::imread(pair->white, CV_LOAD_IMAGE_GRAYSCALE);
        if (white.empty()) {
            std::cerr << "Cannot open " << pair->white << std::endl;
        }
        cv::Mat black = cv::imread(pair->black, CV_LOAD_IMAGE_GRAYSCALE);
        if (black.empty()) {
            std::cerr << "Cannot open " << pair->black << std::endl;
        }
    
        cv::Mat pattern1 = cv::imread(pair->pattern1, CV_LOAD_IMAGE_GRAYSCALE);
        if (pattern1.empty()) {
            std::cerr << "Cannot open " << pair->pattern1 << std::endl;
        }
        cv::Mat pattern2 = cv::imread(pair->pattern2, CV_LOAD_IMAGE_GRAYSCALE);
        if (pattern2.empty()) {
            std::cerr << "Cannot open " << pair->pattern2 << std::endl;
        }
        cv::Mat mask = cd.makeMask(white, black);
      
        cv::Canny(pattern1, pattern1, 150, 400);
        pattern1 = pattern1.mul(mask);
        std::vector<std::vector<cv::Point2i>> edges1 = cd.extractEdges(pattern1);
        cd.display(cv::Size2i(pattern1.cols, pattern1.rows), edges1, "pattern1");
    
        cv::Canny(pattern2, pattern2, 150, 400);
        pattern2 = pattern2.mul(mask);
        std::vector<std::vector<cv::Point2i>>edges2 = cd.extractEdges(pattern2);
        cd.display(cv::Size2i(pattern2.cols, pattern2.rows), edges2, "pattern2");
    
        cd.saveTwoEdges(edges1, edges2);
    }
    
    cd.writeEdges("test.xml");

    return 0;
}