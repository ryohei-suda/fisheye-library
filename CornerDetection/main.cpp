//
//  main.cpp
//  CornerDetection
//
//  Created by Ryohei Suda on 2014/03/23.
//  Copyright (c) 2014年 Ryohei Suda. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <tinyxml2.h>

typedef struct {
    cv::Rect area;
    int status; // 0: Not selected 1: Selecting 2: Selected
    int width, height;
} Selection;

void onMouse(int event, int x, int y, int flag, void*);
void display(cv::Mat&, std::string);
std::vector<std::vector<cv::Point2i>> extractEdges(cv::Mat&);
void saveEdges(std::ofstream&, std::vector<std::vector<cv::Point2i>>&, std::vector<std::vector<cv::Point2i>>&);
std::vector<std::vector<std::string>> loadImageFilenames(std::string);
std::string eraseSideWhiteSpace( std::string);

int main(int argc, const char * argv[])
{
    
    std::cout << "Type list file name of calibration imgages > ";
    std::string fname;
    std::cin >> fname;
    std::vector<std::vector<std::string>> filenames = loadImageFilenames(fname);
    std::ofstream ofs("data.dat");
    
    std::cout << "Type focal length > ";
    double f;
    std::cin >> f;
    ofs << "#f\n" << f << std::endl;

    cv::Size2i size(0, 0);
    
    cv::Mat init = cv::imread(filenames[0][0]);
    size.width = init.cols;
    size.height = init.rows;
    init.release();
    
    ofs << "#center\n" << size.width/2.0 << ' ' << size.height/2.0 << std::endl;
    ofs << "#img_size\n" << size.width << ' ' << size.height << std::endl;
    
    bool is_base;
    for (std::vector<std::vector<std::string>>::iterator it = filenames.begin(); it != filenames.end(); ++it) {
        is_base = (it->size() % 2 == 1) ? true : false;
        
        cv::Mat base;
        std::vector<std::string>::iterator it2 = it->begin();
        if (is_base) {
            base = cv::imread(*it2, 0);
            if(base.empty() || size.width != base.cols || size.height != base.rows) { return -1; }
            cv::Canny(base, base, 50, 200);
            cv::Mat kernel = cv::Mat::ones(9, 9, CV_8UC1);
            cv::dilate(base, base, kernel);
            base = ~base;
            ++it2;
        }
        
        while (it2 != it->end()) {
            // Vertical stripe image
            std::cout << "Processing " << *it2 << " ... ";
            cv::Mat pattern = cv::imread(*it2, 0);
            if(pattern.empty() || size.width != pattern.cols || size.height != pattern.rows) {
                std::cout << "cannot open" << std::endl;
                return -1;
            }
            cv::Canny(pattern, pattern, 150, 400);
            if (is_base) {
                pattern = pattern.mul(base);
            }
            display(pattern, *it2);
            std::vector<std::vector<cv::Point2i>> edges1 = extractEdges(pattern);
            ++it2;
            std::cout << "end" << std::endl;
            
            
            // Horizontal stripe image
            std::cout << "Processing " << *it2 << " ... ";
            pattern = cv::imread(*it2, 0);
            if(pattern.empty() || size.width != pattern.cols || size.height != pattern.rows) {
                std::cout << "cannot open" << std::endl;
                return -1;
            }
            cv::Canny(pattern, pattern, 150, 400);
            if (is_base) {
                pattern = pattern.mul(base);
            }
            display(pattern, *it2);
            std::vector<std::vector<cv::Point2i>> edges2 = extractEdges(pattern);
            ++it2;
            std::cout << "end" << std::endl;
            
            saveEdges(ofs, edges1, edges2);
        }
    }

    
    return 0;
}

// Load image filenames from a csv file
std::vector<std::vector<std::string>> loadImageFilenames(std::string filename)
{
    std::ifstream ifs(filename);
    
    std::vector<std::vector<std::string>> imgnames;
    
    if(!ifs) {
        std::cerr << "Cant open " << filename << std::endl;
        return imgnames;
    }
    
    std::string line;
    while (getline(ifs, line)) {
        std::istringstream stream(line);
        std::vector<std::string> imgname;
        int pos;
        while((pos = (int)line.find(",")) != line.npos) {
            std::string name = eraseSideWhiteSpace(line.substr(0, pos));
            if (name.size() == 0) { break; }
            imgname.push_back(name);
            line = line.substr(pos+1, line.npos-1);
        }
        imgname.push_back(eraseSideWhiteSpace(line));
        imgnames.push_back(imgname);
    }
    
    return imgnames;
}

std::string eraseSideWhiteSpace( std::string String )
{
    while (*String.begin() == ' ' || *String.begin() == '\t' ) { // Beggining
        String.erase(0, 1);
    }
    
    while( *String.rbegin() == ' ' || *String.rbegin() == '\t' ) { // End
        String.erase( String.size() - 1 );
    }
    
    return String;
}

//TODO sort points according to line
std::vector<std::vector<cv::Point2i>> extractEdges(cv::Mat& image)
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
    
    // Ignore edges which have fewer than 10 points
    for (edge = edges.begin(); edge != edges.end();) {
        if(edge->size() < 10) {
            edge = edges.erase(edge);
        } else {
            ++edge;
        }
    }

    return edges;
}

// Save a pair of edges
void saveEdges( std::ofstream& ofs, std::vector<std::vector<cv::Point2i>>& first,
               std::vector<std::vector<cv::Point2i>>& second)
{
    ofs << "#pair\n#edges1\n";
    
    std::vector<std::vector<cv::Point2i>>::iterator edge;
    for (edge = first.begin(); edge != first.end(); ++edge) { // First edge
        std::vector<cv::Point2i>::iterator point;
        for (point = edge->begin(); point != edge->end(); ++point) {
            ofs << point->x << ' ' << point->y << ", ";
        }
        ofs << "\n";
    }
    
    ofs << "#edges2\n";

    for (edge = second.begin(); edge != second.end(); ++edge) { // Second edge
        std::vector<cv::Point2i>::iterator point;
        for (point = edge->begin(); point != edge->end(); ++point) {
            ofs << point->x << ' ' << point->y << ", ";
        }
        ofs << "\n";
    }
}

// Display an image with selecting function
void display(cv::Mat &image, std::string name)
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
    selection.width = image.cols;
    selection.height = image.rows;
    cv::setMouseCallback(name, onMouse, &selection);
    cv::Mat show = cv::Mat::zeros(image.rows, image.cols, CV_8UC3);
    std::vector<std::vector<cv::Point2i>> edge = extractEdges(image);
    int i = 0;
    for (std::vector<std::vector<cv::Point2i>>::iterator it1 = edge.begin(); it1 != edge.end(); ++it1, ++i) {
        for (std::vector<cv::Point2i>::iterator it2 = it1->begin(); it2 != it1->end(); ++it2) {
            show.at<cv::Vec3b>(it2->y, it2->x) = color[i];
        }
    }
    
    while(1) {
        // Draw lines
        show = cv::Mat::zeros(image.rows, image.cols, CV_8UC3);
        int i = 0;
        for (std::vector<std::vector<cv::Point2i>>::iterator it1 = edge.begin(); it1 != edge.end(); ++it1, ++i) {
            for (std::vector<cv::Point2i>::iterator it2 = it1->begin(); it2 != it1->end(); ++it2) {
                show.at<cv::Vec3b>(it2->y, it2->x) = color[i];
            }
        }
//        image.copyTo(show);
        if(selection.status == 0) {
        } else if (selection.status == 1) {
            cv::Mat over = cv::Mat::ones(selection.area.height, selection.area.width, CV_8UC3) * 127;
            show(selection.area) = cv::max(show(selection.area), over);
        } else if (selection.status == 2) {
            image(selection.area) = cv::Mat::zeros(selection.area.height, selection.area.width, CV_8UC1);
            
            edge = extractEdges(image);
            
            selection.status = 0;
        }
        
        cv::imshow(name, show);
        if(cv::waitKey(10) == 'n'){
            cv::destroyWindow(name);
            break;
        }
    }
}

// Mouse event handler
// To remove unrelated edges
void onMouse(int event, int x, int y, int flag, void* data)
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