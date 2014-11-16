//
//  CornerDetection.cpp
//  CornerDetection
//
//  Created by Ryohei Suda on 2014/07/08.
//  Copyright (c) 2014年 RyoheiSuda. All rights reserved.
//

#include "LineDetection.h"


/*
 * Constructor
 */
LineDetection::LineDetection()
{
    output.NewDeclaration();
    tinyxml2::XMLElement *root = output.NewElement("edges");
    output.InsertFirstChild(root);
}

/*
 * Load filenames of images from XML files
 */
void LineDetection::loadImageXML(std::string filename)
{
    tinyxml2::XMLDocument doc;
    doc.LoadFile(filename.c_str());
    tinyxml2::XMLElement *root = doc.FirstChildElement("images");
    
    const char *fl = root->FirstChildElement("focal_length")->GetText();
    focal_length = atof(fl);
    
    const char *ps = root->FirstChildElement("pixel_size")->GetText();
    pixel_size = atof(ps);
    
    projection = root->FirstChildElement("projection")->GetText();
    
    tinyxml2::XMLElement *node = root->FirstChildElement("pair");
    while (node) {
        pair p;
        
        tinyxml2::XMLElement *filename = node->FirstChildElement("pattern");
        int count;
        for (count = 0; count < 4; ++count) {
            if (!filename) {
                break;
            }
            p.filenames[count] = std::string(filename->GetText());
            filename = filename->NextSiblingElement("pattern");
        }
        if (count == 3|| count < 2 || count > 4) {
            std::cerr << "Unrecognized XML structure!" << std::endl;
        } else if (count == 4) {
            p.type = Four;
        } else if (count == 2) {
            if (node->FirstChildElement("white") && node->FirstChildElement("black")) {
                p.filenames[2] = std::string(node->FirstChildElement("white")->GetText());
                p.filenames[3] = std::string(node->FirstChildElement("black")->GetText());
            } else {
                p.type = Two;
            }
        }
        image_names.push_back(p);
        node = node->NextSiblingElement("pair");
    }
    
}

/*
 * Make mask which represents a (phisical) display area
 */
cv::Mat LineDetection::makeMask(cv::Mat& white, cv::Mat& black)
{
    cv::Mat mask, diff;
    
    // Difference between white and black images
    diff = white - black;
    mask = cv::max(diff, cv::Mat::zeros(diff.rows, diff.cols, diff.type()));
    
//    cv::imshow("mask", mask*255);
//    cv::waitKey();
    
    cv::threshold(mask, mask, 15, 1, cv::THRESH_BINARY);
    
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
void LineDetection::onMouse(int event, int x, int y, int flag, void* data)
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
        case cv::EVENT_LBUTTONDBLCLK: // Remove one point
            selection->status = 3;
            selection->area.x = x;
            selection->area.y = y;
            break;
    }
}

// Display an image with selecting function
void LineDetection::display(cv::Size2i size, std::vector<std::vector<cv::Point2i>>& edges, std::string name)
{
    cv::namedWindow(name, CV_WINDOW_NORMAL);
    
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
                show.at<cv::Vec3b>(point->y, point->x) = color[i%30];
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
                bool deleted = false;
                for (std::vector<cv::Point2i>::iterator point = line->begin(); point != line->end(); ) {
                    if (point->x >= selection.area.x && point->x <= selection.area.x+selection.area.width && point->y >= selection.area.y && point->y <= selection.area.y+selection.area.height) {
                        point = line->erase(point);
                        deleted = true;
                    } else {
                        ++point;
                    }
                }
                if (deleted) {
                    std::vector<std::vector<cv::Point2i>> clustered = clusteringEdges(*line);
                    for (std::vector<std::vector<cv::Point2i>>::iterator it = clustered.begin(); it != clustered.end();) {
                        if (it->size() < 20) { // Delete edges which have under 20 points
                            it = clustered.erase(it);
                        } else {
                            ++it;
                        }
                    }
                    if (clustered.size() == 0) {
                        line = edges.erase(line);
                    } else {
                        *line = clustered[0];
                        line = edges.insert(line, clustered.begin()+1, clustered.end());
                    }
                } else {
                    ++line;
                    
                }
            }
            
            selection.area.x = 0;
            selection.area.y = 0;
            selection.area.height = 0;
            selection.area.width = 0;
            selection.status = 0;
            
        } else if (selection.status == 3) { // Remove one point
            for (std::vector<std::vector<cv::Point2i>>::iterator line = edges.begin(); line != edges.end(); ) {
                bool deleted = false;
                for (std::vector<cv::Point2i>::iterator point = line->begin(); point != line->end(); ) {
                    if (point->x == selection.area.x && point->y == selection.area.y) {
                        point = line->erase(point);
                        deleted = true;
                        break;
                    } else {
                        ++point;
                    }
                }
                if (deleted) {
                    std::vector<std::vector<cv::Point2i>> clustered = clusteringEdges(*line);
                    for (std::vector<std::vector<cv::Point2i>>::iterator it = clustered.begin(); it != clustered.end();) {
                        if (it->size() < 20) { // Delete edges which have under 20 points
                            it = clustered.erase(it);
                        } else {
                            ++it;
                        }
                    }
                    if (clustered.size() == 0) {
                        line = edges.erase(line);
                    } else {
                        *line = clustered[0];
                        line = edges.insert(line, clustered.begin()+1, clustered.end());
                    }
                    break;
                }
            }
            
            std::cout << "end test" << std::endl;
            selection.area.x = 0;
            selection.area.y = 0;
            selection.area.height = 0;
            selection.area.width = 0;
            selection.status = 0;
        }
        
        cv::imshow(name, show);
        if(cv::waitKey(10) == 'n'){
            break;
        }
    }
    cv::destroyWindow(name);
}

cv::Mat LineDetection::detectEdges(cv::Mat &image, cv::Mat &mask)
{
    cv::Mat edge_image;
    edge_image = image.mul(mask);
    
    cv::Canny(edge_image, edge_image, 50, 200);
    
//    cv::threshold(edge_image, edge_image, 0, 255, cv::THRESH_BINARY|cv::THRESH_OTSU);
    cv::Mat kernel = (cv::Mat_<uchar>(3,3) << 0,1,0, 1,1,1, 0,1,0 );
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
std::vector<std::vector<cv::Point2i>> LineDetection::extractEdges(cv::Mat& image)
{
    std::vector<std::vector<cv::Point2i>> edges;
//    std::vector<cv::Point2i> points;
//    
//    for (int y = 0; y < image.rows; y++) {
//        for (int x = 0; x < image.cols; x++){
//            if (image.at<unsigned char>(y, x) == 255) { // If a point is edge
//                points.push_back(cv::Point2i(x, y));
//            }
//        }
//    }
//    
//    edges = clusteringEdges(points);
  
    cv::Mat tmp = image.clone();
    for (int y = 0; y < tmp.rows; ++y) {
        for (int x = 0; x < tmp.cols; ++x) {
            if (tmp.data[y * tmp.step + x] == 255) {
                std::vector<cv::Point2i> line;
                std::stack<int> p_x, p_y;
                p_x.push(x); p_y.push(y);
                tmp.data[y * tmp.step + x] = 0;
                while (!p_x.empty()) {
                    cv::Point2i p(p_x.top(), p_y.top());
                    line.push_back(p);
                    p_x.pop(); p_y.pop();
                    if (tmp.data[(p.y - 1) * tmp.step + (p.x - 1)] == 255) {
                        tmp.data[(p.y - 1) * tmp.step + (p.x - 1)] = 0;
                        p_x.push(p.x - 1); p_y.push(p.y - 1);
                    }
                    if (tmp.data[(p.y - 1) * tmp.step + (p.x)] == 255) {
                        tmp.data[(p.y - 1) * tmp.step + (p.x)] = 0;
                        p_x.push(p.x); p_y.push(p.y - 1);
                    }
                    if (tmp.data[(p.y - 1) * tmp.step + (p.x + 1)] == 255) {
                        tmp.data[(p.y - 1) * tmp.step + (p.x + 1)] = 0;
                        p_x.push(p.x + 1); p_y.push(p.y - 1);
                    }
                    if (tmp.data[(p.y) * tmp.step + (p.x - 1)] == 255) {
                        tmp.data[(p.y) * tmp.step + (p.x - 1)] = 0;
                        p_x.push(p.x - 1); p_y.push(p.y);
                    }
                    if (tmp.data[(p.y) * tmp.step + (p.x + 1)] == 255) {
                        tmp.data[(p.y) * tmp.step + (p.x + 1)] = 0;
                        p_x.push(p.x + 1); p_y.push(p.y);
                    }
                    if (tmp.data[(p.y + 1) * tmp.step + (p.x - 1)] == 255) {
                        tmp.data[(p.y + 1) * tmp.step + (p.x - 1)] = 0;
                        p_x.push(p.x - 1); p_y.push(p.y + 1);
                    }
                    if (tmp.data[(p.y + 1) * tmp.step + (p.x)] == 255) {
                        tmp.data[(p.y + 1) * tmp.step + (p.x)] = 0;
                        p_x.push(p.x); p_y.push(p.y + 1);
                    }
                    if (tmp.data[(p.y + 1) * tmp.step + (p.x + 1)] == 255) {
                        tmp.data[(p.y + 1) * tmp.step + (p.x + 1)] = 0;
                        p_x.push(p.x + 1); p_y.push(p.y + 1);
                    }
                }
                if (line.size() > 20) {
                    edges.push_back(line);
                }
            }
        }
    }
    
    // Ignore edges which have fewer than 20 points
//    std::vector<std::vector<cv::Point2i>>::iterator edge;
//    for (edge = edges.begin(); edge != edges.end();) {
//        if(edge->size() < 20) {
//            edge = edges.erase(edge);
//        } else {
//            ++edge;
//        }
//    }
    
    return edges;
}

std::vector<std::vector<cv::Point2i>> LineDetection::clusteringEdges(std::vector<cv::Point2i> points)
{
    std::vector<std::vector<cv::Point2i>> edges;
    
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
    
    return edges;
}

/*
 * Load images, extract edges, and save these
 */
void LineDetection::processAllImages()
{
    std::vector<LineDetection::pair>::iterator pair = image_names.begin();
    for (; pair != image_names.end(); ++pair) {
        std::vector<std::vector<cv::Point2i> > edges[2];
        cv::Mat img[4];
        switch (pair->type) {
            case Four:
                for (int i = 0; i < 4; ++i) {
                    std::cout << "Loading " << pair->filenames[i] << std::endl;
                    img[i] = cv::imread(pair->filenames[i], CV_LOAD_IMAGE_GRAYSCALE);
                    if (img[i].empty()) {
                        std::cerr << "Cannot open!" << pair->filenames[i] << std::endl;
                        exit(-1);
                    }
                }
                edges[0] = detectValley(img[0], img[1]);
                display(cv::Size2i(img[0].cols, img[0].rows), edges[0], "edges");
                edges[1] = detectValley(img[2], img[3]);
                display(cv::Size2i(img[2].cols, img[2].rows), edges[1], "edges");
                break;
                
            case Two:
                for (int i = 0; i < 2; ++i) {
                    std::cout << "Loading " << pair->filenames[i] << std::endl;
                    img[i] = cv::imread(pair->filenames[i], CV_LOAD_IMAGE_GRAYSCALE);
                    if (img[i].empty()) {
                        std::cerr << "Cannot open!" << pair->filenames[i] << std::endl;
                        exit(-1);
                    }
                    cv::Canny(img[i], img[i], 50, 200);
                    edges[i] = extractEdges(img[i]);
                    display(cv::Size2i(img[i].cols, img[i].rows), edges[i], pair->filenames[i]);
                }
                break;
                
            case TwoBW:
                for (int i = 0; i < 4; ++i) {
                    std::cout << "Loading " << pair->filenames[i] << std::endl;
                    img[i] = cv::imread(pair->filenames[i], CV_LOAD_IMAGE_GRAYSCALE);
                    if (img[i].empty()) {
                        std::cerr << "Cannot open!" << pair->filenames[i] << std::endl;
                        exit(-1);
                    }
                }
                cv::Mat mask = makeMask(img[2], img[3]);
                for (int i = 0; i < 2; ++i) {
                    cv::Mat edge = detectEdges(img[0], mask);
                    edges[i] = extractEdges(edge);
                    display(cv::Size2i(edge.cols, edge.rows), edges[i], pair->filenames[i]);
                }
                break;
        }
        
        saveTwoEdges(edges[0], edges[1]);
    }
}

void LineDetection::saveParameters()
{
    tinyxml2::XMLElement *fl_elm = output.NewElement("focal_length");
    fl_elm->SetText(focal_length);
    output.RootElement()->InsertEndChild(fl_elm);
    
    tinyxml2::XMLElement *ps_elm = output.NewElement("pixel_size");
    ps_elm->SetText(pixel_size);
    output.RootElement()->InsertEndChild(ps_elm);
    
    tinyxml2::XMLElement *width = output.NewElement("width");
    width->SetText(img_size.width);
    output.RootElement()->InsertEndChild(width);
    
    tinyxml2::XMLElement *height = output.NewElement("height");
    height->SetText(img_size.height);
    output.RootElement()->InsertEndChild(height);
    
    tinyxml2::XMLElement *proj = output.NewElement("projection");
    proj->SetText(projection.c_str());
    output.RootElement()->InsertEndChild(proj);
}

// Save a pair of edges
void LineDetection::saveTwoEdges(std::vector<std::vector<cv::Point2i>>& first, std::vector<std::vector<cv::Point2i>>& second)
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

void LineDetection::writeXML(std::string filename)
{
    output.SaveFile(filename.c_str());
}

std::vector<std::vector<cv::Point2i> > LineDetection::detectValley(cv::Mat &img1, cv::Mat &img2)
{
    typedef enum {UpLeft, Up, UpRight, Left, Center, Right, DownLeft, Down, DownRight} Direction;
    int d2x[9] = {-1, 0, 1, -1, 0, 1, -1, 0, 1}; // Direction to which x coordination
    int d2y[9] = {-1, -1, -1, 0, 0, 0, 1, 1, 1}; // Direction to which y coordination
    Direction front[9][5] = {
        {UpRight, DownLeft, Up, Left, UpLeft}, // UpLeft
        {Left, Right, UpLeft, UpRight, Up}, // Up
        {UpLeft, DownRight, Up, Right, UpRight}, // UpRight
        {Up, Down, UpLeft, DownLeft, Left}, // Left
        {Center},
        {Up, Down, UpRight, DownRight, Right}, // Right
        {UpLeft, DownRight, Left, Down, DownLeft}, // DownLeft
        {Left, Right, DownLeft, DownRight, Down}, // Down
        {UpRight, DownLeft, Right, Down, DownRight} // DownRight
    };
    Direction back[9][3] = {
        {Right, Down, DownRight}, // Upleft
        {DownLeft, Down, DownRight}, // Up
        {Left, DownLeft, Down}, // UpRight
        {UpRight, Right, DownRight}, // Left
        {Center}, // Center
        {UpLeft, Left, DownLeft}, // Right
        {Up, UpRight, Right}, // DownLeft
        {UpLeft, Up, UpRight}, // Down
        {UpLeft, Up, Left}  // DownRight
    };
    std::vector<std::vector<cv::Point2i> > edges;
    
    uchar threshold = 10;
    cv::Mat mask, blur, diff = abs(img1 - img2);
//    cv::namedWindow("diff", CV_WINDOW_NORMAL);
//    cv::imshow("diff", diff);
//    cv::waitKey();
    cv::GaussianBlur(diff, blur, cv::Size(5,5), 1);
    
    cv::Mat valley = cv::Mat::zeros(blur.rows, blur.cols, CV_8UC1);
    for (int y = 3; y < blur.rows-3; ++y) {
        for (int x = 3; x < blur.cols-3; ++x) {
            if (diff.at<uchar>(y,x) > 128) {
                continue;
            }
            if (threshold < blur.at<uchar>(y-3,x) &&
                blur.at<uchar>(y-3,x) > blur.at<uchar>(y-2,x) &&
                blur.at<uchar>(y-2,x) > blur.at<uchar>(y-1,x) &&
                blur.at<uchar>(y-1,x) >= blur.at<uchar>(y,x) &&
                blur.at<uchar>(y,x) <= blur.at<uchar>(y+1,x) &&
                blur.at<uchar>(y+1,x) < blur.at<uchar>(y+2,x) &&
                blur.at<uchar>(y+2,x) < blur.at<uchar>(y+3,x) &&
                threshold < blur.at<uchar>(y+3,x)) {
                valley.at<uchar>(y,x) = 255;
            } else if (threshold < blur.at<uchar>(y,x-3) &&
                blur.at<uchar>(y,x-3) > blur.at<uchar>(y,x-2) &&
                blur.at<uchar>(y,x-2) > blur.at<uchar>(y,x-1) &&
                blur.at<uchar>(y,x-1) >= blur.at<uchar>(y,x) &&
                blur.at<uchar>(y,x) <= blur.at<uchar>(y,x+1) &&
                blur.at<uchar>(y,x+1) < blur.at<uchar>(y,x+2) &&
                blur.at<uchar>(y,x+2) < blur.at<uchar>(y,x+3) &&
                threshold < blur.at<uchar>(y,x+3)) {
                valley.at<uchar>(y,x) = 255;
            } else if (threshold < blur.at<uchar>(y-3,x-3) &&
                blur.at<uchar>(y-3,x-3) > blur.at<uchar>(y-2,x-2) &&
                blur.at<uchar>(y-2,x-2) > blur.at<uchar>(y-1,x-1) &&
                blur.at<uchar>(y-1,x-1) >= blur.at<uchar>(y,x) &&
                blur.at<uchar>(y,x) <= blur.at<uchar>(y+1,x+1) &&
                blur.at<uchar>(y+1,x+1) < blur.at<uchar>(y+2,x+2) &&
                blur.at<uchar>(y+2,x+2) < blur.at<uchar>(y+3,x+3) &&
                threshold < blur.at<uchar>(y+3,x+3)) {
                valley.at<uchar>(y,x) = 255;
            } else if (threshold < blur.at<uchar>(y+3,x-3) &&
                blur.at<uchar>(y+3,x-3) > blur.at<uchar>(y+2,x-2) &&
                blur.at<uchar>(y+2,x-2) > blur.at<uchar>(y+1,x-1) &&
                blur.at<uchar>(y+1,x-1) >= blur.at<uchar>(y,x) &&
                blur.at<uchar>(y,x) <= blur.at<uchar>(y-1,x+1) &&
                blur.at<uchar>(y-1,x+1) < blur.at<uchar>(y-2,x+2) &&
                blur.at<uchar>(y-2,x+2) < blur.at<uchar>(y-3,x+3) &&
                threshold < blur.at<uchar>(y-3,x+3)) {
                valley.at<uchar>(y,x) = 255;
            }
        }
    }
    
//    cv::namedWindow("valley", CV_WINDOW_NORMAL);
//    cv::imshow("valley", valley);
//    cv::waitKey();
    std::vector<std::vector<cv::Point2i> > points = extractEdges(valley);
    cv::Mat clustered = cv::Mat::zeros(blur.rows, blur.cols, CV_8UC1);
    unsigned long max_point_num = 0; // Number of max points in clustered valleys
    std::vector<cv::Point2i> min_points;
    for (int i = 0; i < points.size(); ++i) {
        if (points[i].size() > max_point_num) {
            max_point_num = points[i].size();
        }
    }
    for (int i = 0; i < points.size(); ++i) {
        if (max_point_num/4 > points[i].size()) {
            points.erase(points.begin()+i);
            --i;
            continue;
        }
        uchar min_val = 255;
        cv::Point2i min_point;
        for (int j = 0; j < points[i].size(); ++j) {
            clustered.at<uchar>(points[i][j].y, points[i][j].x) = 255;
            uchar val = diff.at<uchar>(points[i][j].y, points[i][j].x);
            if (val < min_val && abs((int)points[i].size()/2-j) < 20) {
                min_val = val;
                min_point = points[i][j];
            }
        }
        min_points.push_back(min_point);
    }
    cv::dilate(clustered, clustered, cv::Mat::ones(5, 5, CV_8UC1));
    diff += ~clustered;
//cv::imshow("diff", diff);
//cv::waitKey();
    
    bool opposite_lines = false; // If find lines of an oppsite direction
    
    for (int k = 0; k < min_points.size(); ++k) {// Until detecting all lines
        cv::Point2i base = min_points[k];
        int x = base.x, y = base.y;
        Direction c_direction = Center; // direcion of current pixel
        bool opposite = false; // If searched opposite side of a current line
        
        
        uchar min = 255;
        for (int i = 0; i < 9; ++i) {
            uchar val = diff.at<uchar>(base.y+d2y[i], base.x+d2x[i]);
            if (val < min && i != 4) {
                x = base.x+d2x[i];
                y = base.y+d2y[i];
                base.x = x;
                base.y = y;
                min = val;
            }
        }
        min = 255;
        for (int i = 0; i < 9; ++i) {
            uchar val = diff.at<uchar>(base.y+d2y[i], base.x+d2x[i]);
            if (val < min && i != 4) {
                c_direction = (Direction)i;
                min = val;
            }
        }
        Direction o_direction = (Direction)abs(c_direction - 8); // Opposite direcion of the first point
        std::deque<cv::Point2i> line;
        line.push_back(cv::Point2i(x, y));
        
        while(true) { // Until detectiong all pixels of both sides of a line
            uchar min = 255;
            Direction *f = front[c_direction];
            uchar center = diff.at<uchar>(y, x);
            for (int i = 0; i < 5; ++i) { // Front side of direction
                uchar val = diff.at<uchar>(y+d2y[f[i]], x+d2x[f[i]]);
                if (val <= min) {
                    min = val;
                    c_direction = f[i];
                }
                if (val == center) {
                    if(opposite) {
                        line.push_front(cv::Point2i(x,y));
                    } else {
                        line.push_back(cv::Point2i(x,y));
                    }
                }
            }
            if(opposite) {
                line.push_front(cv::Point2i(x,y));
            } else {
                line.push_back(cv::Point2i(x,y));
            }            // Update to a next pixel
            x += d2x[c_direction];
            y += d2y[c_direction];
            Direction *b = back[c_direction];
            for (int i = 0; i < 3; ++i) { // Back side of direction
                diff.at<uchar>(y+d2y[b[i]], x+d2x[b[i]]) = 255;
            }

            if (x <= 0 || x >= diff.cols-1 || y <= 0 || y >= diff.rows-1) { // Check range of current pixel
                if (opposite) {
                    break;
                } else {
                    opposite = true;
                    x = base.x;
                    y = base.y;
                    c_direction = o_direction;
                    continue;
                }
            }
            // Check if the pixel is out of line
            if (diff.at<uchar>(y+d2y[c_direction],x+d2x[c_direction]) == 255) {
                if (opposite) {
                    break;
                } else {
                    opposite = true;
                    x = base.x;
                    y = base.y;
                    c_direction = o_direction;
                    continue;
                }
            }
        }
        
        // Delete 10 points from both sides of a line
        for (int i = 0; i < 10 && line.size() > 2; ++i) {
            line.pop_front();
            line.pop_back();
        }
        std::vector<cv::Point2i> t;
        t.insert(t.begin(), line.begin(), line.end());
        if (opposite_lines) {
            edges.insert(edges.begin(), t);
        } else {
            edges.insert(edges.end(), t);
        }
    }

    return edges;
}


std::vector<std::vector<std::vector<cv::Point2i> > > LineDetection::loadEdgeXML(std::string filename)
{
    std::vector<std::vector<std::vector<cv::Point2i> > > all_edges;
    
    tinyxml2::XMLDocument doc;
    doc.LoadFile(filename.c_str());
    tinyxml2::XMLElement *root = doc.FirstChildElement("edges");
    
    pixel_size = atof(root->FirstChildElement("pixel_size")->GetText());
    focal_length = atof(root->FirstChildElement("focal_length")->GetText());

    
    img_size.width = atoi(root->FirstChildElement("width")->GetText());
    img_size.height = atoi(root->FirstChildElement("height")->GetText());
    
    std::stringstream ssdata;
    tinyxml2::XMLElement *pair = root->FirstChildElement("pair");
    while (pair != NULL) {
        
        // edge1
        tinyxml2::XMLElement *edge1_elm = pair->FirstChildElement("edge1");
        tinyxml2::XMLElement *line_elm = edge1_elm->FirstChildElement("line");
        std::vector<std::vector<cv::Point2i> > edge1;
        while (line_elm != NULL) {
            std::vector<cv::Point2i> line; // One line of points
            tinyxml2::XMLElement *p = line_elm->FirstChildElement("p");
            while (p != NULL) {
                cv::Point2i point;
                ssdata.str(p->GetText());
                ssdata >> point.x;
                ssdata >> point.y;
                line.push_back(point);
                ssdata.clear();
                p = p->NextSiblingElement("p");
            }
            edge1.push_back(line);
            line_elm = line_elm->NextSiblingElement("line");
        }
        all_edges.push_back(edge1);
        
        // edge2
        tinyxml2::XMLElement *edge2_elm = pair->FirstChildElement("edge2");
        line_elm = edge2_elm->FirstChildElement("line");
        std::vector<std::vector<cv::Point2i> > edge2;
        while (line_elm != NULL) {
            std::vector<cv::Point2i> line; // One line of points
            tinyxml2::XMLElement *p = line_elm->FirstChildElement("p");
            while (p != NULL) {
                cv::Point2d point;
                ssdata.str(p->GetText());
                ssdata >> point.x;
                ssdata >> point.y;
                line.push_back(point);
                ssdata.clear();
                p = p->NextSiblingElement("p");
            }
            edge2.push_back(line);
            line_elm = line_elm->NextSiblingElement("line");
        }
        
        all_edges.push_back(edge2);
        pair = pair->NextSiblingElement("pair");
    }

    return all_edges;
}


void LineDetection::editAllEdges(std::vector<std::vector<std::vector<cv::Point2i> > > edges)
{
    std::vector<std::vector<cv::Point2i> > edge;
    for (int i = 0; i < edges.size(); ++i) {
        display(img_size, edges[i], "Edit");
        if (i%2 == 1) {
            saveTwoEdges(edges[i-1], edges[i]);
        }
    }
}