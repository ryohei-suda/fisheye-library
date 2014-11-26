//
//  SimulatingImage.cpp
//  SimulatingImage
//
//  Created by Ryohei Suda on 2014/11/25.
//  Copyright (c) 2014年 RyoheiSuda. All rights reserved.
//

#include "SimulatingImage.h"


void SimulatingImage::calcCorners() {
    cv::Mat inital_corners[4];
    inital_corners[0] = (cv::Mat_<double>(3,1) << -pattern_size.width/2., -pattern_size.height/2.,0);
    inital_corners[1] = (cv::Mat_<double>(3,1) << -pattern_size.width/2., pattern_size.height/2.,0);
    inital_corners[2] = (cv::Mat_<double>(3,1) << pattern_size.width/2., -pattern_size.height/2.,0);
    inital_corners[3] = (cv::Mat_<double>(3,1) << pattern_size.width/2., pattern_size.height/2.,0);
   
    cv::Mat rotation = (cv::Mat_<double>(3,3) <<
                        cos(pitch[0])*cos(pitch[1]), cos(pitch[0])*sin(pitch[1])*sin(pitch[2])-sin(pitch[1])*cos(pitch[2]), cos(pitch[0])*sin(pitch[1])*cos(pitch[2])+sin(pitch[0])*sin(pitch[2]),
                        sin(pitch[0])*cos(pitch[1]), sin(pitch[0])*sin(pitch[1])*sin(pitch[2])+cos(pitch[0])*cos(pitch[2]), sin(pitch[0])*sin(pitch[1])*cos(pitch[2])-cos(pitch[0])*sin(pitch[2]),
                        -sin(pitch[2]), cos(pitch[1])*sin(pitch[2]), cos(pitch[1])*cos(pitch[2])
                        );
    cv::Mat translation = cv::Mat(pattern_center);
    
    for (int i = 0; i < 4; ++i) {
        cv::Mat p = rotation * inital_corners[i] + translation;
        corners[i].x = p.at<double>(0);
        corners[i].y = p.at<double>(1);
        corners[i].z = p.at<double>(2);
    }
    
    for (int i = 0; i < 2; ++i) {
        units[i] = corners[0] - corners[i+1];
    }
    
    norm = units[1].cross(units[0]) * (1./(pattern_size.width*pattern_size.height));
}

cv::Point3d SimulatingImage::getRay(cv::Point2d point, int model) {
    double theta;
    double r = sqrt(pow(point.x-optical_center.x, 2) + pow(point.y-optical_center.y, 2));
    switch (model) {
        case 0: // Equidistance
            theta = r/f0;
            for (int i = 0; i < a.size(); ++i) {
                theta += a.at(i) * pow(r/f0, 2*i+3);
            }
            theta *= f0 / f;
            break;
        default:
            theta = 0.;
            break;
    }
    
    cv::Point3d m;
    if ( r != 0) {
        m.x = (point.x - optical_center.x) / r * sin(theta);
        m.y = (point.y - optical_center.y) / r * sin(theta);
        m.z = cos(theta);
    } else {
        m.x = 0;
        m.y = 0;
        m.z = 1;
    }
    
    return m;
}

bool SimulatingImage::isCross(cv::Point3d ray) {
    double cosine = ray.ddot(norm);
    if (0. < cosine  && cosine <= 1.) {
        return true;
    } else {
        return false;
    }
}

cv::Point3d SimulatingImage::calcCrossPoint(cv::Point3d ray) {
    return ((norm.ddot(corners[0])) / (norm.ddot(ray))) * ray;
}

double SimulatingImage::calcS(cv::Point3d p) {
    double numerator = p.x * (corners[1].y-corners[0].y) + p.y * (corners[0].x-corners[1].x) + corners[0].y*corners[1].x - corners[0].x*corners[1].y;
    double denominator = corners[2].x * (corners[1].y-corners[0].y) + corners[2].y * (corners[0].x-corners[1].x) + corners[0].y*corners[1].x - corners[0].x*corners[1].y;
    
    return numerator / denominator;
}

double SimulatingImage::calcT(cv::Point3d p) {
    double numerator = p.x * (corners[0].y-corners[2].y) + p.y * (corners[2].x-corners[0].x) + corners[0].x*corners[2].y - corners[0].y*corners[2].x;
    double denominator = corners[1].x * (corners[0].y-corners[2].y) + corners[1].y * (corners[2].x-corners[0].x) + corners[0].x*corners[2].y - corners[0].y*corners[2].x;
    
    return numerator / denominator;
}


cv::Mat SimulatingImage::projectPlane() {
    
    cv::Mat img = cv::Mat::zeros(img_size.height, img_size.width, CV_8UC1);
    
    calcCorners();
    
    for (int y = 0; y < img_size.height; ++y) {
        for (int x = 0; x < img_size.width; ++x) {
            cv::Point3d ray = getRay(cv::Point2d(x,y), 0);
            if (isCross(ray)) {
                cv::Point3d p = calcCrossPoint(ray);
                double s = calcS(p);
                double t = calcT(p);
                if (0 <= s && s <= 1 && 0 <= t && t <= 1) {
                    img.at<uchar>(y,x) = 255;
                }
            }
        }
    }
    
    return img;
}

void SimulatingImage::display() {
    cv::Mat img;
    cv::namedWindow("Simulation Image", CV_WINDOW_NORMAL);
    char key;
    
    while (true) {
        img = projectPlane();
        cv::imshow("Simulation Image", img);
        key = cv::waitKey();
        
        if (key == 27) { // ESC
            break;
        }
        switch (key) {
            case 'r':
                ++ pattern_center.x;
                break;
            case 'f':
                ++ pattern_center.y;
                break;
            case 'v':
                ++ pattern_center.z;
                break;
            case 't':
                pattern_center.x += 10;
                break;
            case 'g':
                pattern_center.y += 10;
                break;
            case 'b':
                pattern_center.z += 10;
                break;
            case 'e':
                -- pattern_center.x;
                break;
            case 'd':
                -- pattern_center.y;
                break;
            case 'c':
                -- pattern_center.z;
                break;
            case 'w':
                pattern_center.x -= 10;
                break;
            case 's':
                pattern_center.y -= 10;
                break;
            case 'x':
                pattern_center.z -= 10;
                break;
            case 'u':
                pitch[0] += d2r(0.5);
                break;
            case 'j':
                pitch[1] += d2r(0.5);
                break;
            case 'm':
                pitch[2] += d2r(0.5);
                break;
            case 'y':
                pitch[0] += d2r(5);
                break;
            case 'h':
                pitch[1] += d2r(5);
                break;
            case 'n':
                pitch[2] += d2r(5);
                break;
            case 'i':
                pitch[0] -= d2r(0.5);
                break;
            case 'k':
                pitch[1] -= d2r(0.5);
                break;
            case ',':
                pitch[2] -= d2r(0.5);
                break;
            case 'o':
                pitch[0] -= d2r(5);
                break;
            case 'l':
                pitch[1] -= d2r(5);
                break;
            case '.':
                pitch[2] -= d2r(5);
                break;
            default:
                break;
        }
    }
}





