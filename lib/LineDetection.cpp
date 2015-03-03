//
//  CornerDetection.cpp
//  CornerDetection
//
//  Created by Ryohei Suda on 2014/07/08.
//  Copyright (c) 2014年 RyoheiSuda. All rights reserved.
//

#include "LineDetection.h"

int LineDetection::unit = 1;

/*
 * Constructor
 */
LineDetection::LineDetection()
{
    output.NewDeclaration();
    tinyxml2::XMLElement *root = output.NewElement("lines");
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
    
    img_size.width = atoi(root->FirstChildElement("width")->GetText());
    img_size.height = atoi(root->FirstChildElement("height")->GetText());
    
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
        if (count != 4) {
            std::cerr << "Unrecognized XML structure!" << std::endl;
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
void LineDetection::display(cv::Size2i size, std::vector<std::vector<cv::Point2f> >& edges, std::string name)
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
    
    int min = (img_size.width > img_size.height) ? img_size.height/4 : img_size.width/4;
    
    while(1) {
        // Draw lines
        show = cv::Mat::zeros(size.height, size.width, CV_8UC3);
        int i = 0;
        for (auto &line : edges) {
            for (auto &point : line) {
                show.at<cv::Vec3b>(point.y, point.x) = color[i%30];
            }
            ++i;
        }
        //        image.copyTo(show);
        if(selection.status == 0) {
        } else if (selection.status == 1) {
            cv::Mat over = cv::Mat::ones(selection.area.height, selection.area.width, CV_8UC3) * 127;
            show(selection.area) = cv::max(show(selection.area), over);
        } else if (selection.status == 2) {
            for (int i = 0; i < edges.size(); ++i) {
                bool deleted = false;
                std::vector<cv::Point2f> *line = &edges[i];
                for (int j = 0; j < line->size(); ++j) {
                    cv::Point2f *point = &line->at(j);
                    if (point->x >= selection.area.x && point->x <= selection.area.x+selection.area.width && point->y >= selection.area.y && point->y <= selection.area.y+selection.area.height) {
                        line->erase(line->begin()+j);
                        --j;
                        deleted = true;
                    }
                }
                if (deleted) {
                    std::vector<std::vector<cv::Point2f> > clustered = clusteringPoints(*line, 2);
                    for (int k = 0; k < clustered.size(); ++k) {
                        if (clustered[k].size() < min) {
                            clustered.erase(clustered.begin()+k);
                            --k;
                        }
                    }
                    
                    edges.erase(edges.begin()+i);
                    if (clustered.size() != 0) {
                        edges.insert(edges.begin()+i, clustered.begin(), clustered.end());
                        i += clustered.size()-1;
                    } else {
                        --i;
                    }
                }
            }
            
            selection.area.x = 0;
            selection.area.y = 0;
            selection.area.height = 0;
            selection.area.width = 0;
            selection.status = 0;
            
        } else if (selection.status == 3) { // Remove one point
            for (int i = 0; i < edges.size(); ++i) {
                bool deleted = false;
                std::vector<cv::Point2f> *line = &edges[i];
                for (int j = 0; j < line->size(); ++j) {
                    cv::Point2f *point = &line->at(j);
                    if (point->x == selection.area.x && point->y == selection.area.y) {
                        line->erase(line->begin()+j);
                        deleted = true;
                        break;
                    } else {
                        ++point;
                    }
                }
                if (deleted) {
                    std::vector<std::vector<cv::Point2f> > clustered = clusteringPoints(*line, 2);
                    for (int k = 0; k < clustered.size(); ++k) {
                        if (clustered[k].size() < min) {
                            clustered.erase(clustered.begin()+k);
                            --k;
                        }
                    }
                    
                    edges.erase(edges.begin()+i);
                    if (clustered.size() != 0) {
                        edges.insert(edges.begin()+i, clustered.begin(), clustered.end());
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
        if(cv::waitKey(30) == 'n'){
            break;
        }
    }
    cv::destroyWindow(name);
}

/*
 * Extract points of edges from an edge image
 */
std::vector<std::vector<cv::Point2f> > LineDetection::extractPoints(cv::Mat& image)
{
    std::vector<std::vector<cv::Point2f> > lines;
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
                std::vector<cv::Point2f> line;
                std::stack<int> p_x, p_y;
                p_x.push(x); p_y.push(y);
                tmp.data[y * tmp.step + x] = 0;
                while (!p_x.empty()) {
                    cv::Point2i p(p_x.top(), p_y.top());
                    line.push_back(p);
                    p_x.pop(); p_y.pop();
                    if ((p.y!=0 && p.x!=0) && (tmp.data[(p.y - 1) * tmp.step + (p.x - 1)] == 255)) { // Top left
                        tmp.data[(p.y - 1) * tmp.step + (p.x - 1)] = 0;
                        p_x.push(p.x - 1); p_y.push(p.y - 1);
                    }
                    if ((p.y!=0) && (tmp.data[(p.y - 1) * tmp.step + (p.x)] == 255)) { // Top
                        tmp.data[(p.y - 1) * tmp.step + (p.x)] = 0;
                        p_x.push(p.x); p_y.push(p.y - 1);
                    }
                    if ((p.y!=0 && p.x!=tmp.cols-1) && (tmp.data[(p.y - 1) * tmp.step + (p.x + 1)] == 255)) { // Top right
                        tmp.data[(p.y - 1) * tmp.step + (p.x + 1)] = 0;
                        p_x.push(p.x + 1); p_y.push(p.y - 1);
                    }
                    if ((p.x!=0) && (tmp.data[(p.y) * tmp.step + (p.x - 1)] == 255)) { // left
                        tmp.data[(p.y) * tmp.step + (p.x - 1)] = 0;
                        p_x.push(p.x - 1); p_y.push(p.y);
                    }
                    if ((p.x!=tmp.cols-1) && (tmp.data[(p.y) * tmp.step + (p.x + 1)] == 255)) { // Right
                        tmp.data[(p.y) * tmp.step + (p.x + 1)] = 0;
                        p_x.push(p.x + 1); p_y.push(p.y);
                    }
                    if ((p.y!=tmp.rows-1 && p.x!=0) && (tmp.data[(p.y + 1) * tmp.step + (p.x - 1)] == 255)) { // Down left
                        tmp.data[(p.y + 1) * tmp.step + (p.x - 1)] = 0;
                        p_x.push(p.x - 1); p_y.push(p.y + 1);
                    }
                    if ((p.y!=tmp.rows-1) && (tmp.data[(p.y + 1) * tmp.step + (p.x)] == 255)) { // Down
                        tmp.data[(p.y + 1) * tmp.step + (p.x)] = 0;
                        p_x.push(p.x); p_y.push(p.y + 1);
                    }
                    if ((p.y!=tmp.rows-1 && p.x!=tmp.cols-1) && (tmp.data[(p.y + 1) * tmp.step + (p.x + 1)] == 255)) { // Down right
                        tmp.data[(p.y + 1) * tmp.step + (p.x + 1)] = 0;
                        p_x.push(p.x + 1); p_y.push(p.y + 1);
                    }
                }
                lines.push_back(line);
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
    
    return lines;
}

std::vector<std::vector<cv::Point2f> > LineDetection::clusteringPoints(std::vector<cv::Point2f> points, float r)
{
    std::vector<std::vector<cv::Point2f> > edges;
    
    r *= r; // To reduce calcuraion of squre root
    
    while (!points.empty()) {
        std::vector<cv::Point2f> new_edge;
//        std::vector<cv::Point2i>::iterator tmp;
        new_edge.push_back(points[0]); // Push a point
        points.erase(points.begin()); // Delete a point
        for (int i = 0; i < new_edge.size(); ++i) {
            cv::Point2f *n = &new_edge[i];
            for (int j = 0; j < points.size(); ++j) {
                cv::Point2f *p =&points[j];
                float distance = pow(n->x - p->x, 2) + pow(n->y - p->y, 2);
                if (distance <= r) { // If a point is includ
                    new_edge.push_back(*p);
                    n = &new_edge[i];
                    points.erase(points.begin()+j);
                    j--;
                }
            }
        }
//        int counter = 0;
//        for (std::vector<cv::Point2i>::iterator c = new_edge.begin(); c != new_edge.end(); c = tmp){ // Cluster
//            tmp = c + 1;
//            for (std::vector<cv::Point2i>::iterator p = points.begin(); p != points.end();) { // Points
//                if (c->x >= p->x-1 && c->x <= p->x+1 && c->y >= p->y-1 && c->y <= p->y+1) { // If a point is included in a cluster
//                    new_edge.push_back(*p);
//                    tmp = new_edge.begin() + counter;
//                    p = points.erase(p);
//                } else {
//                    ++p;
//                }
//            }
//            ++counter;
//        }
        edges.push_back(new_edge);
    }
    
    return edges;
}

/*
 * Load images, extract edges, and save these
 */
void LineDetection::processAllImages()
{
    //    std::vector<LineDetection::pair>::iterator pair = image_names.begin();
    for (auto &pair : image_names) {
        //    for (; pair != image_names.end(); ++pair) {
        std::vector<std::vector<cv::Point2f> > edges[2];
        cv::Mat img[4];
        cv::Mat tmp;
        for (int i = 0; i < 4; ++i) {
            std::cout << "Loading " << pair.filenames[i] << std::endl;
            img[i] = cv::imread(pair.filenames[i], CV_LOAD_IMAGE_GRAYSCALE);
            
            if (img[i].empty()) {
                std::cerr << "Cannot open!" << pair.filenames[i] << std::endl;
                exit(-1);
            }
            //                    img[i].convertTo(tmp, CV_64F);
            //                    cv::resize(tmp, img[i], cv::Size(), unit, unit, cv::INTER_CUBIC);
        }
        edges[0] = detectLines(img[0], img[1]);
        //                edges[0] = detectValley(img[0], img[1]);
        display(cv::Size2i(img[0].cols, img[0].rows), edges[0], "edges");
        //                edges[1] = detectValley(img[2], img[3]);
        edges[1] = detectLines(img[2], img[3]);
        display(cv::Size2i(img[2].cols, img[2].rows), edges[1], "edges");
        
        
        savePair(edges[0], edges[1]);
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
void LineDetection::savePair(std::vector<std::vector<cv::Point2f> >& first, std::vector<std::vector<cv::Point2f> >& second)
{
    tinyxml2::XMLElement *pair = output.NewElement("pair");
    output.RootElement()->InsertEndChild(pair);
    tinyxml2::XMLElement *lines1 = output.NewElement("lines1");
    pair->InsertFirstChild(lines1);
    tinyxml2::XMLElement *lines2 = output.NewElement("lines2");
    pair->InsertEndChild(lines2);
    
    char str[100];
    
    for (auto &lines : first) { // First lines
        tinyxml2::XMLElement *line = output.NewElement("line");
        lines1->InsertEndChild(line);
        for (auto &point : lines) {
            tinyxml2::XMLElement *p = output.NewElement("p");
            sprintf(str, "%f %f", point.x/(float)unit, point.y/(float)unit);
            p->SetText(str);
            line->InsertEndChild(p);
        }
    }

    for (auto &lines : second) { // Second lines
        tinyxml2::XMLElement *line = output.NewElement("line");
        lines2->InsertEndChild(line);
        for (auto &point : lines) {
            tinyxml2::XMLElement *p = output.NewElement("p");
            sprintf(str, "%f %f", point.x/(float)unit, point.y/(float)unit);
            p->SetText(str);
            line->InsertEndChild(p);
        }
    }
}

void LineDetection::writeXML(std::string filename)
{
    output.SaveFile(filename.c_str());
}

std::vector<std::vector<cv::Point2f> > LineDetection::detectLines(cv::Mat &img1, cv::Mat &img2)
{
    std::vector<std::vector<cv::Point2f> > lines;
    std::vector<cv::Point2f> points;
    
    // Check type of img1 and img2
    if (img1.type() != CV_64FC1) {
        cv::Mat tmp;
        img1.convertTo(tmp, CV_64FC1);
        img1 = tmp;
    }
    if (img2.type() != CV_64FC1) {
        cv::Mat tmp;
        img2.convertTo(tmp, CV_64FC1);
        img2 = tmp;
    }
    cv::Mat diff = img1-img2;
    cv::Mat cross = cv::Mat::zeros(diff.rows, diff.cols, CV_8UC1);
    cv::Mat cross_inv = cv::Mat::zeros(diff.rows, diff.cols, CV_8UC1);
    double thresh = 20;
    bool positive; // Whether previous found cross point was positive
    bool search; // Whether serching
    bool found_first;
    int val_now, val_prev;
    
    // search for x direction
    for (int y = 0; y < diff.rows; y++) {
        val_prev = diff.at<double>(y,0);
        positive = (val_prev > 0);
        search = false;
        found_first = false;
        for (int x = 1; x < diff.cols; ++x) {
            val_now = diff.at<double>(y, x);
            if (search && (
                ((val_now <= 0) && positive) || ((val_now >= 0) && !positive))) {// found crossed point
                if (abs(val_now) < abs(val_prev)) {
                    cross.at<uchar>(y,x) = 255;
                } else {
                    cross.at<uchar>(y,x-1) = 255;
                }
                positive = !positive;
                search = false;
            }
            if (!search && abs(val_now) > thresh) {
                search = true;
                if (!found_first) {
                    found_first = true;
                    positive = (val_now > 0);
                }
            }
            val_prev = val_now;
        }
    }

    // search for y direction
    for (int x = 0; x < diff.cols; x++) {
        val_prev = diff.at<double>(0,x);
        positive = (val_prev > 0);
        search = false;
        found_first = false;
        for (int y = 1; y < diff.rows; ++y) {
            val_now = diff.at<double>(y,x);
            if (search && (
              ((val_now <= 0) && positive) || ((val_now >= 0) && !positive))) {// found crossed point
                if (abs(val_now) < abs(val_prev)) {
                    if (cross.at<uchar>(y,x) != 255) {
                        cross.at<uchar>(y,x) = 255;
                    }
                } else {
                    cross.at<uchar>(y-1,x) = 255;
                }
                positive = !positive;
                search = false;
            }
            if (!search && abs(val_now) > thresh) {
                search = true;
                if (!found_first) {
                    found_first = true;
                    positive = (val_now > 0);
                }
            }
            val_prev = val_now;
        }
    }
    
    lines = extractPoints(cross);
    
    // Remove noise
    int min = (img_size.width > img_size.height) ? img_size.height/4 : img_size.width/4;
    for (int i = 0; i < lines.size(); ++i) {
        if (lines[i].size() < min) {
            lines.erase(lines.begin()+i);
            --i;
        }
    }
    
    return lines;
}


std::vector<std::vector<std::vector<cv::Point2f> > > LineDetection::loadEdgeXML(std::string filename)
{
    std::vector<std::vector<std::vector<cv::Point2f> > > all_edges;
    
    tinyxml2::XMLDocument doc;
    doc.LoadFile(filename.c_str());
    tinyxml2::XMLElement *root = doc.FirstChildElement("lines");
    
    pixel_size = atof(root->FirstChildElement("pixel_size")->GetText());
    focal_length = atof(root->FirstChildElement("focal_length")->GetText());

    
    img_size.width = atoi(root->FirstChildElement("width")->GetText());
    img_size.height = atoi(root->FirstChildElement("height")->GetText());
    
    projection = root->FirstChildElement("projection")->GetText();
    
    std::stringstream ssdata;
    tinyxml2::XMLElement *pair = root->FirstChildElement("pair");
    while (pair != NULL) {
        
        // edge1
        tinyxml2::XMLElement *edge1_elm = pair->FirstChildElement("lines1");
        tinyxml2::XMLElement *line_elm = edge1_elm->FirstChildElement("line");
        std::vector<std::vector<cv::Point2f> > edge1;
        while (line_elm != NULL) {
            std::vector<cv::Point2f> line; // One line of points
            tinyxml2::XMLElement *p = line_elm->FirstChildElement("p");
            while (p != NULL) {
                cv::Point2f point;
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
        tinyxml2::XMLElement *edge2_elm = pair->FirstChildElement("lines2");
        line_elm = edge2_elm->FirstChildElement("line");
        std::vector<std::vector<cv::Point2f> > edge2;
        while (line_elm != NULL) {
            std::vector<cv::Point2f> line; // One line of points
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


void LineDetection::editAllLines(std::vector<std::vector<std::vector<cv::Point2f> > > edges)
{
    std::vector<std::vector<cv::Point2i> > edge;
    for (int i = 0; i < edges.size(); ++i) {
        display(img_size, edges[i], "Edit");
        if (i%2 == 1) {
            savePair(edges[i-1], edges[i]);
        }
    }
}
