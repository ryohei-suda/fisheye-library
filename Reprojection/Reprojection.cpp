//
//  Reprojection.cpp
//  Reprojection
//
//  Created by Ryohei Suda on 2014/09/12.
//  Copyright (c) 2014年 RyoheiSuda. All rights reserved.
//

#include "Reprojection.h"


void Reprojection::loadPrameters(std::string filename)
{
    cv::FileStorage fs(filename, cv::FileStorage::READ);
    
    if (!fs.isOpened()) {
        std::cerr << filename << " cannnot be opened!" << std::endl;
        exit(-1);
    }
    fs["f"] >> f;
    fs["f0"] >> f0;
    fs["center"] >> center;
    fs["img_size"] >> img_size;
    
    a.clear();
    cv::FileNode fn = fs["a"];
    cv::FileNodeIterator it = fn.begin();
    for (; it != fn.end(); ++it) {
        a.push_back((static_cast<double>(*it)));
    }
}


void Reprojection::theta2radius()
{
    double max_r = 0;
    // Calculate the longest distance between optical center and image corner
    if (img_size.width - center.x > img_size.width / 2.0) {
        max_r += pow(img_size.width - center.x, 2);
    } else {
        max_r += pow(center.x, 2);
    }
    if (img_size.height - center.y > img_size.height / 2.0) {
        max_r += pow(img_size.height - center.y, 2);
    } else {
        max_r += pow(center.y, 2);
    }
    max_r = sqrt(max_r);
    int theta_size = round(max_r) * precision + 10; // If PRECISION = 10, r = {0, 0.1, 0.2, 0.3, ...}
    
    std::vector<double> theta(theta_size);
    for (int i = 0; i < theta_size; ++i) {
        double r = (double)i / precision;
        theta[i] = r/f0;
        for (int j = 0; j != a.size(); ++j) {
            theta[i] += a[j] * pow(r/f0, j*2+3);
        }
        theta[i] *= f0/f;
    }
    
    std::ofstream ofs("graph.dat");
    int r_size = max_r * precision + 10;
    r.resize(r_size);
    int j = 1; // j/PRECISION: radius
    rad_step = theta[theta_size-1] / r_size; // 0 ~ theta[end] radian
    for (int i = 0; i < r_size; ++i) {
        double rad = rad_step * i;
        for (; j < theta_size; ++j) {
            if (theta[j] > rad) {
                r[i] = ((i*rad_step - theta[j-1]) / (theta[j]-theta[j-1]) + j-1) / precision; // See my note on 2014/6/6
                //                std::cout << "(i,j): ("<< i << "," << j << ")\t" << theta[j] << "\t" << rad << "\t" << r[i] << std::endl;
                ofs << rad * 180 / M_PI << ' ' << rad << ' ' << r[i] << ' ' << f * rad << std::endl;
                break;
            }
        }
    }
    ofs.close();
}


void Reprojection::calcMaps(double theta_x, double theta_y, double f_, cv::Mat& mapx, cv::Mat& mapy)
{
    mapx.create(img_size.height, img_size.width, CV_32FC1);
    mapy.create(img_size.height, img_size.width, CV_32FC1);
    
    cv::Mat Rx = (cv::Mat_<double>(3,3) <<
                  1,            0,             0,
                  0, cos(theta_x), -sin(theta_x),
                  0, sin(theta_x),  cos(theta_x));
    cv::Mat Ry = (cv::Mat_<double>(3,3) <<
                  cos(theta_y),  0, sin(theta_y),
                  0,             1,            0,
                  -sin(theta_y), 0, cos(theta_y));
    cv::Mat R = Ry * Rx;
    
    for (int y_ = 0; y_ < img_size.height; ++y_) { // y
        for (int x_ = 0; x_ < img_size.width; ++x_) { // x
            
            cv::Point2d p2(x_ - img_size.width/2.0, y_ - img_size.height/2.0);
            cv::Mat p3 = (cv::Mat_<double>(3,1) << p2.x, p2.y, f_);
            cv::Mat real = 1.0/sqrt(pow(p2.x,2) + pow(p2.y,2) + pow(f_,2)) * R * p3;
            
            double x = real.at<double>(0,0);
            double y = real.at<double>(1,0);
            double z = real.at<double>(2,0);
            double theta = atan2(sqrt(1-pow(z,2)), z);
            cv::Point2d final = center + r[(int)(theta/rad_step)] / sqrt(1-pow(z,2)) * cv::Point2d(x,y);
            //            cv::Point2d final = center + f * theta / sqrt(1-pow(z,2))  * cv::Point2d(x,y); // Perspective projection
            //            cv::Point2d final = center + 2*f*tan(theta/2) / sqrt(1-pow(z,2))  * cv::Point2d(x,y); // Stereo graphic projection
            
            mapx.at<float>(y_,x_) = final.x;
            mapy.at<float>(y_,x_) = final.y;
        }
    }
}