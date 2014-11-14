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
#include "../libs/tinyxml2.h"

class LineDetection
{
private:
    double focal_length;
    double pixel_size;
    cv::Size2i img_size;
    std::string projection;
    tinyxml2::XMLDocument output;
    
    
    typedef struct { // For the display function
        cv::Rect area;
        int status; // 0: Not selected 1: Selecting 2: Selected
        int width, height;
    } Selection;
    void static onMouse(int event, int x, int y, int flag, void*);

    typedef enum {Four, TwoBW, Two} PType; // 0: 4 patterns, 1: 2 patterns with black and white, 2: 2 patterns

    
public:
    typedef struct {
        PType type;
        std::string filenames[4];
    } pair;
    std::vector<pair> image_names;
    
    LineDetection();
    void loadImageXML(std::string filename);
    cv::Mat makeMask(cv::Mat& white, cv::Mat& black);
    void display(cv::Size2i size, std::vector<std::vector<cv::Point2i>>& edges, std::string name);
    cv::Mat detectEdges(cv::Mat& image, cv::Mat& mask);
    std::vector<std::vector<cv::Point2i> > extractEdges(cv::Mat& image);
    std::vector<std::vector<cv::Point2i> > clusteringEdges(std::vector<cv::Point2i> points);
    void processAllImages();
    void saveParameters(); // Save parameters into XML output
    void saveTwoEdges(std::vector<std::vector<cv::Point2i>>& first, std::vector<std::vector<cv::Point2i>>& second);
    void writeXML(std::string filename);
    std::vector<std::vector<cv::Point2i> > detectValley(cv::Mat &img1, cv::Mat &img2);
    std::vector<std::vector<std::vector<cv::Point2i> > > loadEdgeXML(std::string filename);
    void editAllEdges(std::vector<std::vector<std::vector<cv::Point2i> > > edges);
};

#endif
