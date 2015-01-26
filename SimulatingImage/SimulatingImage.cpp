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
   
    cv::Mat rotation_x = (cv::Mat_<double>(3,3) <<
                          1, 0, 0,
                          0, cos(pitch[2]), -sin(pitch[2]),
                          0, sin(pitch[2]), cos(pitch[2])
                          );
    cv::Mat rotation_y = (cv::Mat_<double>(3,3) <<
                          cos(pitch[1]), 0, sin(pitch[1]),
                          0, 1, 0,
                          -sin(pitch[1]), 0, cos(pitch[1])
                          );
    cv::Mat rotation_z = (cv::Mat_<double>(3,3) <<
                          cos(pitch[0]), -sin(pitch[0]), 0,
                          sin(pitch[0]), cos(pitch[0]), 0,
                          0, 0, 1
                          );
    cv::Mat rotation = rotation_z * rotation_y * rotation_x;
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
    
    norm = units[1].cross(units[0]);
    norm *= (1./norm.ddot(norm));
}

cv::Point3d SimulatingImage::getRay(cv::Point2d point) {
    double theta;
    double r = sqrt(pow(point.x-optical_center.x, 2) + pow(point.y-optical_center.y, 2));
    switch (model) {
        case 0: //  Stereographic
            theta = r/f0;
            for (int i = 0; i < a.size(); ++i) {
                theta += a.at(i) * pow(r/f0, 2*i+3);
            }
            theta *= f0 / (2*f);
            theta = 2 * atan(theta);
            break;
            
        case 1: // Orthographic
            theta = r/f0;
            for (int i = 0; i < a.size(); ++i) {
                theta += a.at(i) * pow(r/f0, 2*i+3);
            }
            theta *= f0 / f;
            theta = asin(theta);
            break;
            
        case 2: // Equidistance
            theta = r/f0;
            for (int i = 0; i < a.size(); ++i) {
                theta += a.at(i) * pow(r/f0, 2*i+3);
            }
            theta *= f0 / f;
            break;
            
        case 3: // EquisolidAngle
            theta = r/f0;
            for (int i = 0; i < a.size(); ++i) {
                theta += a.at(i) * pow(r/f0, 2*i+3);
            }
            theta *= f0 / (2*f);
            theta = 2 * asin(theta);
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
    
    if (acos(ray.z) > fov) {
        return false;
    }
    
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


cv::Mat SimulatingImage::projectPlane(int pattern) {
    
    cv::Mat img = cv::Mat::zeros(img_size.height, img_size.width, CV_8UC1);
    
    calcCorners();
    
    for (int y = 0; y < img_size.height; ++y) {
        for (int x = 0; x < img_size.width; ++x) {
            cv::Point3d ray = getRay(cv::Point2d(x,y));
            if (isCross(ray)) {
                cv::Point3d p = calcCrossPoint(ray);
                double s = calcS(p);
                double t = calcT(p);
                if (0 <= s && s <= 1 && 0 <= t && t <= 1) {
                    double step;
                    switch (pattern) {
                        case 0: // check
                            step = interval / pattern_size.width;
                            for (int i = 0; 2*i*step <= 1.0; ++i) {
                                if (step*(2*i) <= s && s <= step*(2*i+1)) {
                                    img.at<uchar>(y,x) = 255;
                                    break;
                                }
                            }
                            step = interval / pattern_size.height;
                            for (int i = 0; 2*i*step <= 1.0; ++i) {
                                if (step*(2*i) <= t && t <= step*(2*i+1)) {
                                    img.at<uchar>(y,x) = abs(img.at<uchar>(y,x)-255);
                                    break;
                                }
                            }
                            break;
                        case 1: // vertical strip
                            step = interval / pattern_size.width;
                            for (int i = 0; 2*i*step <= 1.0; ++i) {
                                if (step*(2*i) <= s && s <= step*(2*i+1)) {
                                    img.at<uchar>(y,x) = 255;
                                    break;
                                }
                            }
                            break;
                        case 2: // inverse of 1
                            step = interval / pattern_size.width;
                            for (int i = 0; (2*i+1)*step <= 1.0; ++i) {
                                if (step*(2*i+1) <= s && s <= step*(2*i+2)) {
                                    img.at<uchar>(y,x) = 255;
                                    break;
                                }
                            }
                            break;
                        case 3: // horizontal strip
                            step = interval / pattern_size.height;
                            for (int i = 0; 2*i*step <= 1.0; ++i) {
                                if (step*(2*i) <= t && t <= step*(2*i+1)) {
                                    img.at<uchar>(y,x) = 255;
                                    break;
                                }
                            }
                            break;
                        case 4: // inverse of 3
                            step = interval / pattern_size.height;
                            for (int i = 0; (2*i+1)*step <= 1.0; ++i) {
                                if (step*(2*i+1) <= t && t <= step*(2*i+2)) {
                                    img.at<uchar>(y,x) = 255;
                                    break;
                                }
                            }
                            break;
                    }
                }
            }
        }
    }
    
    return img;
}

void onMouse(int event, int x, int y, int flag, void* data)
{
    SimulatingImage * si = (SimulatingImage *)(data);
    switch (event) {
        case CV_EVENT_LBUTTONDBLCLK:
            cv::Point3d ray = si->getRay(cv::Point2d(x,y));
            cv::Point3d p = si->calcCrossPoint(ray);
            double s = si->calcS(p);
            double t = si->calcT(p);
            std::cout << si->isCross(ray) <<  ' ' << s << ", " << t << std::endl;
            break;
    }
}

void SimulatingImage::display() {
    cv::Mat img;
    cv::namedWindow("Simulation Image", CV_WINDOW_NORMAL);
    int steps = 1;
    int degree = 10; // 0.1 degree
    cv::createTrackbar("Steps [pixel]", "Simulation Image", &steps, 200);
    cv::createTrackbar("Rotation [0.1 degree]", "Simulation Image", &degree, 900);
    char key;
    int counter = 0;
    bool print = true;
    
    std::cout << "[Usage]" << std::endl;
    std::cout << "\t(r, f, v): Add x, y, or z" << std::endl;
    std::cout << "\t(e, d, c): Sub x, y, or z" << std::endl;
    std::cout << "\t(u, j, m): Add Rolling, Piching, or Yawing" << std::endl;
    std::cout << "\t(i, k, ,): Sub Rolling, Piching, or Yawing" << std::endl;
    std::cout << "\t(   0   ): Set steps to initial (1 pixel or 0.5 degree)" << std::endl;
    std::cout << "\t( SPACE ): Save patterns" << std::endl;
    std::cout << "\t(  ESC  ): End" << std::endl;
    std::cout << "\n(x, y, z, Rolling, Piching, Yawing)" << std::endl;
    while (true) {
        if (print) {
            std::cout << "("  << pattern_center.x << ", " << pattern_center.y <<  ", " << pattern_center.z << ", " << pitch[0] << ", " << pitch[1] << ", " << pitch[2] << ")" << std::endl;
        }
        print = true;
        img = projectPlane(0);
        cv::imshow("Simulation Image", img);
        cv::setMouseCallback("Simulation Image", onMouse, this);
        key = cv::waitKey();

        if (key == 27) { // ESC
            break;
        }
        switch (key) {
            case 'r':
                pattern_center.x += steps;
                break;
            case 'f':
                pattern_center.y += steps;
                break;
            case 'v':
                pattern_center.z += steps;
                break;
            case 'e':
                pattern_center.x -= steps;
                break;
            case 'd':
                pattern_center.y -= steps;
                break;
            case 'c':
                pattern_center.z -= steps;
                break;
            case 'u':
                pitch[0] += d2r(0.1*degree);
                break;
            case 'j':
                pitch[1] += d2r(0.1*degree);
                break;
            case 'm':
                pitch[2] += d2r(0.1*degree);
                break;
            case 'i':
                pitch[0] -= d2r(0.1*degree);
                break;
            case 'k':
                pitch[1] -= d2r(0.1*degree);
                break;
            case ',':
                pitch[2] -= d2r(0.1*degree);
                break;
            case '0': // Reset
                pattern_center.x = pattern_center.y = 0;
                pattern_center.z = 200;
                pitch[0] = pitch[1] = pitch[2] = 0;
                
                break;
            case 32: // space
                std::cout << "Saved";
                cv::imwrite("test.png",img);
                for (int i = 1; i <= 4; ++i) {
                    img = projectPlane(i);
                    std::ostringstream sout;
                    sout << std::setfill('0') << std::setw(3) << counter << ".png";
                    cv::GaussianBlur(img, img, cv::Size(5,5), 1);
                    cv::imwrite(sout.str(), img);
                    std::cout << " " << sout.str();
                    ++counter;
                }
                std::cout << std::endl;
                break;
            default:
                print = false;
                break;
        }
    }
}


