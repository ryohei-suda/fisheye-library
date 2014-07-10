//
//  CornerDetection.cpp
//  CornerDetection
//
//  Created by Ryohei Suda on 2014/07/08.
//  Copyright (c) 2014年 RyoheiSuda. All rights reserved.
//

#include "CornerDetection.h"


/*
 * Constructor
 */
CornerDetection::CornerDetection()
{
    output.NewDeclaration();
    tinyxml2::XMLElement *root = output.NewElement("edges");
    output.InsertFirstChild(root);
}

/*
 * Load filenames of images from XML files
 */
void CornerDetection::loadImageXML(std::string filename)
{
    tinyxml2::XMLDocument doc;
    doc.LoadFile(filename.c_str());
    tinyxml2::XMLElement *root = doc.FirstChildElement("images");
    
    const char *fl = root->FirstChildElement("focal_length")->GetText();
    focal_length = atof(fl);
    tinyxml2::XMLElement *fl_elm = output.NewElement("focal_length");
    fl_elm->SetText(fl);
    output.RootElement()->InsertEndChild(fl_elm);
    
    const char *ps = root->FirstChildElement("pixel_size")->GetText();
    pixel_size = atof(ps);
    tinyxml2::XMLElement *ps_elm = output.NewElement("pixel_size");
    ps_elm->SetText(ps);
    output.RootElement()->InsertEndChild(ps_elm);
    
    tinyxml2::XMLElement *node = root->FirstChildElement("pair");
    while (true) {
        pair p;
        p.white = std::string(node->FirstChildElement("white")->GetText());
        p.black = std::string(node->FirstChildElement("black")->GetText());
        p.pattern1 = std::string(node->FirstChildElement("pattern1")->GetText());
        p.pattern2 = std::string(node->FirstChildElement("pattern2")->GetText());
        image_names.push_back(p);
        node = node->NextSiblingElement("pair");
        if (node == NULL) {
            break;
        }
    }
    
    cv::Mat img = cv::imread(image_names[0].white);
    tinyxml2::XMLElement *width = output.NewElement("width");
    width->SetText(img.cols);
    output.RootElement()->InsertEndChild(width);
    
    tinyxml2::XMLElement *height = output.NewElement("height");
    height->SetText(img.rows);
    output.RootElement()->InsertEndChild(height);
}

/*
 * Make mask which represents a (phicical) display area
 */
cv::Mat CornerDetection::makeMask(cv::Mat& white, cv::Mat& black)
{
    cv::Mat mask, diff;
    
    // Difference between white and black images
    diff = white - black;
    mask = cv::max(diff, cv::Mat::zeros(diff.rows, diff.cols, diff.type()));
    cv::threshold(mask, mask, 0, 1, cv::THRESH_BINARY|cv::THRESH_OTSU);
    
    // Remove noise
    cv::Mat element = cv::Mat::ones(9, 9, CV_8UC1);
    cv::dilate(mask, mask, element);
    element = cv::Mat::ones(19, 19, CV_8UC1);
    cv:erode(mask, mask, element);
    
//    mask.convertTo(mask, CV_32FC1);
//    white.convertTo(white, CV_32FC1);
//    mask = 255.0 / white.mul(mask);
    
    return mask;
}

// Mouse event handler
// To remove unrelated edges
void CornerDetection::onMouse(int event, int x, int y, int flag, void* data)
{
    static cv::Point2i origin;
    Selection *selection = (Selection *)data;
    
    switch(event) {
        case cv::EVENT_LBUTTONDOWN:
            origin.x = x;
            origin.y = y;
            selection->status = 1;
            break;
        case cv::EVENT_LBUTTONUP:
            selection->status = 2;
            break;
        case cv::EVENT_MOUSEMOVE:
            if (selection->status == 1){
                if (origin.x > x) {
                    x = MAX(0, x);
                    selection->area.x = x;
                    selection->area.width = origin.x - x;
                } else {
                    x = MIN(selection->width, x);
                    selection->area.x = origin.x;
                    selection->area.width = x - origin.x;
                }
                if (origin.y > y) {
                    y = MAX(0, y);
                    selection->area.y =y;
                    selection->area.height = origin.y - y;
                } else {
                    y = MIN(selection->height, y);
                    selection->area.y = origin.y;
                    selection->area.height = y - origin.y;
                }
            }
            break;
    }
}

// Display an image with selecting function
void CornerDetection::display(cv::Size2i size, std::vector<std::vector<cv::Point2i>>& edges, std::string name)
{
    cv::namedWindow(name, CV_GUI_NORMAL|CV_WINDOW_NORMAL|CV_WINDOW_KEEPRATIO);
    cv::moveWindow(name, 0, 0);
    
    cv::Vec3b color[30] = {cv::Vec3b(255,255,255), cv::Vec3b(255,0,0), cv::Vec3b(255,255,0), cv::Vec3b(0,255,0), cv::Vec3b(0,0,255),
        cv::Vec3b(255,0,255), cv::Vec3b(204,51,51), cv::Vec3b(204,204,51), cv::Vec3b(51,204,51), cv::Vec3b(51,204,204),
        cv::Vec3b(51,51,204), cv::Vec3b(204,51,204), cv::Vec3b(204,204,204), cv::Vec3b(153,102,102), cv::Vec3b(153,153,102),
        cv::Vec3b(102,153,102), cv::Vec3b(102,153,153), cv::Vec3b(102,102,153), cv::Vec3b(153,102,153), cv::Vec3b(153,153,153),
        cv::Vec3b(51,51,204), cv::Vec3b(204,51,204), cv::Vec3b(204,204,204), cv::Vec3b(153,102,102), cv::Vec3b(153,153,102),
        cv::Vec3b(102,153,102), cv::Vec3b(102,153,153), cv::Vec3b(102,102,153), cv::Vec3b(153,102,153), cv::Vec3b(153,153,153),
    };
    
    Selection selection;
    selection.status = 0;
    selection.width = size.width;
    selection.height = size.height;
    cv::setMouseCallback(name, onMouse, &selection);
    cv::Mat show = cv::Mat::zeros(size.height, size.width, CV_8UC3);
    
    while(1) {
        // Draw lines
        show = cv::Mat::zeros(size.height, size.width, CV_8UC3);
        int i = 0;
        for (std::vector<std::vector<cv::Point2i>>::iterator line = edges.begin(); line != edges.end(); ++line, ++i) {
            for (std::vector<cv::Point2i>::iterator point = line->begin(); point != line->end(); ++point) {
                show.at<cv::Vec3b>(point->y, point->x) = color[i];
            }
        }
        //        image.copyTo(show);
        if(selection.status == 0) {
        } else if (selection.status == 1) {
            cv::Mat over = cv::Mat::ones(selection.area.height, selection.area.width, CV_8UC3) * 127;
            show(selection.area) = cv::max(show(selection.area), over);
        } else if (selection.status == 2) {
//            image(selection.area) = cv::Mat::zeros(selection.area.height, selection.area.width, CV_8UC1);
            for (std::vector<std::vector<cv::Point2i>>::iterator line = edges.begin(); line != edges.end(); ) {
                for (std::vector<cv::Point2i>::iterator point = line->begin(); point != line->end(); ) {
                    
                    if (point->x >= selection.area.x && point->x <= selection.area.x+selection.area.width && point->y >= selection.area.y && point->y <= selection.area.y+selection.area.height) {
                        point = line->erase(point);
                    } else {
                        ++point;
                    }
                }
                if (line->size() == 0) {
                    line = edges.erase(line);
                } else {
                    ++line;
                }
            }
            
            selection.area.x = 0;
            selection.area.y = 0;
            selection.area.height = 0;
            selection.area.width = 0;
            selection.status = 0;
        }
        
        cv::imshow(name, show);
        if(cv::waitKey(10) == 'n'){
            cv::destroyWindow(name);
            break;
        }
    }
}

cv::Mat CornerDetection::detectEdges(cv::Mat &image, cv::Mat &mask)
{
    cv::Mat edge_image;
    
    edge_image = image.mul(mask);
    cv::threshold(edge_image, edge_image, 0, 1, cv::THRESH_BINARY|cv::THRESH_OTSU);
    
    cv::Mat kernel = (cv::Mat_<uchar>(3,3) << 0,1,0, 1,1,1, 0,1,0 );
//    cv::Mat::ones(3, 3, CV_8UC1);
    cv::Mat outline;
    cv::erode(edge_image, outline, kernel);
    edge_image = edge_image - outline;
    
    kernel = cv::Mat::ones(9, 9, CV_8UC1);
    cv::erode(mask, mask, kernel);
    edge_image = edge_image.mul(mask) * 255;
    
    return edge_image;
}

/*
 * Extract points of edges from an edge image
 */
std::vector<std::vector<cv::Point2i>> CornerDetection::extractEdges(cv::Mat& image)
{
    std::vector<std::vector<cv::Point2i>> edges;
    std::vector<std::vector<cv::Point2i>>::iterator edge;
    std::vector<cv::Point2i> points;
    
    for (int y = 0; y < image.rows; y++) {
        for (int x = 0; x < image.cols; x++){
            if (image.at<unsigned char>(y, x) == 255) { // If a point is edge
                points.push_back(cv::Point2i(x, y));
            }
        }
    }
    
    while (!points.empty()) {
        std::vector<cv::Point2i> new_edge;
        std::vector<cv::Point2i>::iterator tmp;
        new_edge.push_back(points[0]);
        points.erase(points.begin());
        int counter = 0;
        for (std::vector<cv::Point2i>::iterator c = new_edge.begin(); c != new_edge.end(); c = tmp){ // Cluster
            tmp = c + 1;
            for (std::vector<cv::Point2i>::iterator p = points.begin(); p != points.end();) { // Points
                if (c->x >= p->x-1 && c->x <= p->x+1 && c->y >= p->y-1 && c->y <= p->y+1) { // If a point is included in a cluster
                    new_edge.push_back(*p);
                    tmp = new_edge.begin() + counter;
                    p = points.erase(p);
                } else {
                    ++p;
                }
            }
            ++counter;
        }
        edges.push_back(new_edge);
    }
    
    // Ignore edges which have fewer than 20 points
    for (edge = edges.begin(); edge != edges.end();) {
        if(edge->size() < 20) {
            edge = edges.erase(edge);
        } else {
            ++edge;
        }
    }
    
    return edges;
}

/*
 * Load images, extract edges, and save these
 */
void CornerDetection::processAllImages()
{
    std::vector<CornerDetection::pair>::iterator pair = image_names.begin();
    for (; pair != image_names.end(); ++pair) {
        cv::Mat white = cv::imread(pair->white, CV_LOAD_IMAGE_GRAYSCALE);
        cv::Mat black = cv::imread(pair->black, CV_LOAD_IMAGE_GRAYSCALE);
        
        cv::Mat pattern1 = cv::imread(pair->pattern1, CV_LOAD_IMAGE_GRAYSCALE);
        cv::Mat pattern2 = cv::imread(pair->pattern2, CV_LOAD_IMAGE_GRAYSCALE);
        cv::Mat mask = makeMask(white, black);
        
//        cv::Canny(pattern1, pattern1, 150, 400);
//        pattern1 = pattern1.mul(mask);
        pattern1 = detectEdges(pattern1, mask);
        std::vector<std::vector<cv::Point2i>> edges1 = extractEdges(pattern1);
        display(cv::Size2i(pattern1.cols, pattern1.rows), edges1, "pattern1");
        
//        cv::Canny(pattern2, pattern2, 150, 400);
//        pattern2 = pattern2.mul(mask);
        pattern2 = detectEdges(pattern2, mask);
        std::vector<std::vector<cv::Point2i>>edges2 = extractEdges(pattern2);
        display(cv::Size2i(pattern2.cols, pattern2.rows), edges2, "pattern2");
        
        saveTwoEdges(edges1, edges2);
    }
}

// Save a pair of edges
void CornerDetection::saveTwoEdges(std::vector<std::vector<cv::Point2i>>& first, std::vector<std::vector<cv::Point2i>>& second)
{
    tinyxml2::XMLElement *pair = output.NewElement("pair");
    output.RootElement()->InsertEndChild(pair);
    tinyxml2::XMLElement *edge1 = output.NewElement("edge1");
    pair->InsertFirstChild(edge1);
    tinyxml2::XMLElement *edge2 = output.NewElement("edge2");
    pair->InsertEndChild(edge2);
    
    char str[100];
    
    std::vector<std::vector<cv::Point2i>>::iterator edge;
    for (edge = first.begin(); edge != first.end(); ++edge) { // First edge
        tinyxml2::XMLElement *line = output.NewElement("line");
        edge1->InsertEndChild(line);
        std::vector<cv::Point2i>::iterator point;
        for (point = edge->begin(); point != edge->end(); ++point) {
            tinyxml2::XMLElement *p = output.NewElement("p");
            sprintf(str, "%d %d", point->x, point->y);
            p->SetText(str);
            line->InsertEndChild(p);
        }
    }
    
    for (edge = second.begin(); edge != second.end(); ++edge) { // Second edge
        tinyxml2::XMLElement *line = output.NewElement("line");
        edge2->InsertEndChild(line);
        std::vector<cv::Point2i>::iterator point;
        for (point = edge->begin(); point != edge->end(); ++point) {
            tinyxml2::XMLElement *p = output.NewElement("p");
            sprintf(str, "%d %d", point->x, point->y);
            p->SetText(str);
            line->InsertEndChild(p);
        }
    }
}

void CornerDetection::writeEdges(std::string filename)
{
    output.SaveFile(filename.c_str());
}