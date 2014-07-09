//
//  CornerDetection.h
//  CornerDetection
//
//  Created by Ryohei Suda on 2014/07/08.
//  Copyright (c) 2014年 RyoheiSuda. All rights reserved.
//

#ifndef CornerDetection_CornerDetection_h
#define CornerDetection_CornerDetection_h

#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include "tinyxml2.h"

class CornerDetection
{
private:
    double focal_length;
    double pixel_size;
    tinyxml2::XMLDocument output;
    
    
    typedef struct {
        cv::Rect area;
        int status; // 0: Not selected 1: Selecting 2: Selected
        int width, height;
    } Selection;
    void static onMouse(int event, int x, int y, int flag, void*);
    
    
public:
    typedef struct {
        std::string white, black, pattern1, pattern2;
    } pair;
    std::vector<pair> image_names;
    
    CornerDetection();
    void loadImageXML(std::string);
    cv::Mat makeMask(cv::Mat&, cv::Mat&);
    void display(cv::Size2i, std::vector<std::vector<cv::Point2i>>&, std::string);
    cv::Mat detectEdges(cv::Mat&, cv::Mat&);
    std::vector<std::vector<cv::Point2i>> extractEdges(cv::Mat&);
    void processAllImages();
    void saveTwoEdges(std::vector<std::vector<cv::Point2i>>&, std::vector<std::vector<cv::Point2i>>&);
    void writeEdges(std::string);
};

#endif
