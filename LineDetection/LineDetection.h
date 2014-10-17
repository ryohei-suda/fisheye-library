//
//  LineDetection.h
//  LineDetection
//
//  Created by Ryohei Suda on 2014/07/08.
//  Copyright (c) 2014年 RyoheiSuda. All rights reserved.
//

#ifndef LineDetection_LineDetection_h
#define LineDetection_LineDetection_h

#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include "tinyxml2.h"

class LineDetection
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
    
    LineDetection();
    void loadImageXML(std::string filename);
    cv::Mat makeMask(cv::Mat& white, cv::Mat& black);
    void display(cv::Size2i size, std::vector<std::vector<cv::Point2i>>& edges, std::string name);
    cv::Mat detectEdges(cv::Mat& image, cv::Mat& mask);
    std::vector<std::vector<cv::Point2i>> extractEdges(cv::Mat& image);
    std::vector<std::vector<cv::Point2i>> clusteringEdges(std::vector<cv::Point2i> points);
    void processAllImages();
    void saveTwoEdges(std::vector<std::vector<cv::Point2i>>& first, std::vector<std::vector<cv::Point2i>>& second);
    void writeEdges(std::string filename);
    std::vector<std::vector<cv::Point2i> > detectValley(cv::Mat &src);
    
};

#endif
