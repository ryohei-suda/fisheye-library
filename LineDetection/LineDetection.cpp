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
    tinyxml2::XMLElement *fl_elm = output.NewElement("focal_length");
    fl_elm->SetText(fl);
    output.RootElement()->InsertEndChild(fl_elm);
    
    const char *ps = root->FirstChildElement("pixel_size")->GetText();
    pixel_size = atof(ps);
    tinyxml2::XMLElement *ps_elm = output.NewElement("pixel_size");
    ps_elm->SetText(ps);
    output.RootElement()->InsertEndChild(ps_elm);
    
    tinyxml2::XMLElement *node = root->FirstChildElement("pair");
    while (node) {
        pair p;
        if (node->FirstChildElement("white")) {
            p.white = std::string(node->FirstChildElement("white")->GetText());
        }
        if (node->FirstChildElement("black")) {
            p.black = std::string(node->FirstChildElement("black")->GetText());
        }
        p.pattern1 = std::string(node->FirstChildElement("pattern1")->GetText());
        p.pattern2 = std::string(node->FirstChildElement("pattern2")->GetText());
        image_names.push_back(p);
        node = node->NextSiblingElement("pair");
    }
    
    cv::Mat img = cv::imread(image_names[0].pattern1);
    if (img.empty()) {
        std::cerr << "Cannot open " << image_names[0].pattern1 << std::endl;
        exit(-1);
    }
    tinyxml2::XMLElement *width = output.NewElement("width");
    width->SetText(img.cols);
    output.RootElement()->InsertEndChild(width);
    
    tinyxml2::XMLElement *height = output.NewElement("height");
    height->SetText(img.rows);
    output.RootElement()->InsertEndChild(height);
    
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
            cv::destroyWindow(name);
            break;
        }
    }
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
    std::vector<std::vector<cv::Point2i>>::iterator edge;
    std::vector<cv::Point2i> points;
    
    for (int y = 0; y < image.rows; y++) {
        for (int x = 0; x < image.cols; x++){
            if (image.at<unsigned char>(y, x) == 255) { // If a point is edge
                points.push_back(cv::Point2i(x, y));
            }
        }
    }
    
    
    edges = clusteringEdges(points);
    
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
        cv::Mat white = cv::imread(pair->white, CV_LOAD_IMAGE_GRAYSCALE);
        cv::Mat black = cv::imread(pair->black, CV_LOAD_IMAGE_GRAYSCALE);

        cv::Mat pattern1 = cv::imread(pair->pattern1, CV_LOAD_IMAGE_GRAYSCALE);
        if (pattern1.empty()) {
            std::cerr << "Cannot open " << pair->pattern1   << std::endl;
            exit(-1);
        }
        cv::Mat pattern2 = cv::imread(pair->pattern2, CV_LOAD_IMAGE_GRAYSCALE);
        if (pattern2.empty()) {
            std::cerr << "Cannot open " << pair->pattern2   << std::endl;
            exit(-1);
        }
        
        cv::Mat mask;
        if (white.empty() || black.empty()) {
            mask = cv::Mat::ones(pattern1.rows, pattern1.cols, CV_8UC1);
        } else {
             mask = makeMask(white, black);
        }
        
//        cv::Canny(pattern1, pattern1, 150, 400);
//        pattern1 = pattern1.mul(mask);
        pattern1 = detectEdges(pattern1, mask);
        std::vector<std::vector<cv::Point2i>> edges1 = extractEdges(pattern1);
        display(cv::Size2i(pattern1.cols, pattern1.rows), edges1, pair->pattern1);
        
//        cv::Canny(pattern2, pattern2, 150, 400);
//        pattern2 = pattern2.mul(mask);
        pattern2 = detectEdges(pattern2, mask);
        std::vector<std::vector<cv::Point2i>>edges2 = extractEdges(pattern2);
        display(cv::Size2i(pattern2.cols, pattern2.rows), edges2, pair->pattern2);
        
        saveTwoEdges(edges1, edges2);
    }
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

void LineDetection::writeEdges(std::string filename)
{
    output.SaveFile(filename.c_str());
}

std::vector<std::vector<cv::Point2i> > LineDetection::detectValley(cv::Mat &src)
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
    Direction first_direction = Center;
    cv::Point2i first_center;
    
    uchar threshold = 10;
    cv::Mat mask, blur;
    cv::GaussianBlur(src, blur, cv::Size(5,5), 2);
    
    cv::threshold(blur, mask, 10, 1, CV_THRESH_OTSU|CV_THRESH_BINARY);
    cv::erode(mask, mask, cv::Mat::ones(5, 5, CV_8UC1));
    //    cv::imshow("diff", mask*255);
    //    cv::waitKey();
    
    double minVal, maxVal;
    cv::Point2i minLoc, maxLoc;
    cv::minMaxLoc(src, &minVal, &maxVal, &minLoc, &maxLoc, mask);
    
    cv::Mat buff = cv::Mat::zeros(src.rows, src.cols, CV_8UC1);
    cv::Mat dst = blur.clone();
    int x = minLoc.x, y = minLoc.y;
    int base_x = x, base_y = y;
    bool opposite_lines = false; // If find lines of an oppsite direction
    
    while (true) { // Until detecting all lines
        
        Direction c_direction = Center; // direcion of current pixel
        bool opposite = false; // If searched opposite side of a current line
        int l_directions[9] = {0}; // To vote which directions is a appropriate line
        
        uchar min = 255;
        for (int i = 0; i < 9; ++i) {
            uchar val = src.at<uchar>(base_y+d2y[i], base_x+d2x[i]);
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
            uchar center = src.at<uchar>(y, x);
            for (int i = 0; i < 5; ++i) { // Front side of direction
                uchar val = src.at<uchar>(y+d2y[f[i]], x+d2x[f[i]]);
                if (val <= min) {
                    min = val;
                    c_direction = f[i];
                }
                if (val == center) { //TODO May need to change
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
            }
            // Check if the pixel is out of line
            if (blur.at<uchar>(y,x) < threshold) {
                if (opposite) {
                    break;
                } else {
                    opposite = true;
                    x = base_x;
                    y = base_y;
                    c_direction = o_direction;
                    continue;
                }
            }
            
            // Update to a next pixel
            x += d2x[c_direction];
            y += d2y[c_direction];
            Direction *b = back[c_direction];
            for (int i = 0; i < 3; ++i) { // Back side of direction
                src.at<uchar>(y+d2y[b[i]], x+d2x[b[i]]) = 255;
            }
            ++l_directions[c_direction];
            //            cv::imshow("diff", src);
            //            cv::waitKey();
            
            if (x <= 0 || x >= src.cols-1 || y <= 0 || y >= src.rows-1) { // Check range of current pixel
                if (opposite) {
                    break;
                } else {
                    opposite = true;
                    x = base_x;
                    y = base_y;
                    c_direction = o_direction;
                    continue;
                }
            }
        }
        
        // Delete 5 points from both sides of a line
        for (int i = 0; i < 5; ++i) {
            line.pop_front();
            line.pop_back();
        }
        std::vector<cv::Point2i> t(line.size());
        t.insert(t.begin(), line.begin(), line.end());
        if (opposite_lines) {
            edges.insert(edges.begin(), t);
        } else {
            edges.push_back(t);
        }
        
        // Find a direction of a next line
        int l_direction = 0, max_vote = -1;
        for (int i = 0; i < 4; ++i) {
            int vote = l_directions[i] + l_directions[8-i];
            if (max_vote < vote) {
                l_direction = i;
                max_vote = vote;
            }
        }
        Direction next_direction = (Direction)((l_direction+2)%4);
        if (opposite_lines) {
            next_direction = (Direction)(8-next_direction);
        }
        if (edges.size() == 1) { // When first line is found
            first_direction = (Direction)(8-next_direction);
            first_center = line.at((int)(line.size()/2));
        }
        
        cv::Point2i center = line.at((int)(line.size()/2));
        bool found = false;
        while (true) {// Find next valley
            int next_x = center.x, next_y = center.y;
            int step_x = (next_direction%3 - 1), step_y = (next_direction/3 - 1);
            std::deque<uchar> dq;
            for (int i = 0; i < 5; ++i) {
                dq.push_back(blur.at<uchar>(next_y, next_x));
                next_x += step_x;
                next_y += step_y;
            }
            while (dq.back() >= threshold && next_y < src.rows && next_y >= 0 && next_x < src.cols && next_x >= 0) {
                // Whether the pixel is on the bottom of valley
                if (dq[0]-2 > dq[1] && dq[1] >= dq[2] && dq[2] <= dq[3] && dq[3] < dq[4]-2) {
                    found = true;
                    break;
                }
                next_x += step_x;
                next_y += step_y;
                dq.pop_front();
                dq.push_back(blur.at<uchar>(next_y, next_x));
                dst.at<uchar>(next_y, next_x) = 127;
            }
            
            if (found) {
                base_x = next_x - 2*step_x;
                base_y = next_y - 2*step_y;
                uchar min_pix = 255, pix;
                for (int i = -1; i <= 1; ++i) {
                    for (int j = -1; j <= 1; ++j) {
                        pix = src.at<uchar>(base_y+i, base_x+j);
                        if (pix <= min_pix) {
                            step_x = j;
                            step_y = i;
                            min_pix = pix;
                        }
                    }
                }
                base_x += step_x;
                base_y += step_y;
                x = base_x;
                y = base_y;
                break;
            } else {
                if (!opposite_lines) {
                    center = first_center;
                    next_direction = first_direction;
                    opposite_lines = true;
                } else {
                    break;
                }
            }
        }
        
        for (int i = 0; i < line.size(); ++i) {
            dst.at<uchar>(line[i].y, line[i].x) = 255;
        }
        
        if (opposite_lines && !found) {
            break;
        }
    }
    
    return edges;
}