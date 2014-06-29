//
//  main.cpp
//  Reprojection
//
//  Created by Ryohei Suda on 2014/06/05.
//  Copyright (c) 2014年 Ryohei Suda. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>

#define PRECISION 10

void loadPrameters(std::string, double&, double&, cv::Point2d&, cv::Size2i&, std::vector<double>&);
void theta2radius(std::vector<double>&, double&, double&, double&, cv::Point2d&, cv::Size2i&, std::vector<double>&);

int main(int argc, const char * argv[])
{
    double f, f0, f_;
    cv::Point2d center;
    cv::Size2i img_size, out_img_size;
    std::vector<double> a;
    std::vector<double> r; // theta to radius
    double rad_step;

    std::string param;
    std::cout << "Type parameter file name > ";
    std::cin >> param;
    loadPrameters(param, f, f0, center, img_size, a);
    
    // Print parameters
    std::cout << "f: " << f << "\nf0: " << f0 << std::endl;
    std::cout << "center: " << center << std::endl;
    std::cout << "image size: " << img_size << std::endl;
    std::cout << "ai: ";
    for (std::vector<double>::iterator it = a.begin(); it != a.end(); ++it) {
        std::cout << *it << '\t';
    }
    std::cout << std::endl;
    
    theta2radius(r, rad_step, f, f0, center, img_size, a);
    
    std::string srcname;
    std::cout << "Type source image file name > ";
    std::cin >> srcname;
    cv::Mat src = cv::imread(srcname);
    cv::Mat mapx(img_size.height, img_size.width, CV_32FC1);
    cv::Mat mapy(img_size.height, img_size.width, CV_32FC1);
    
//    std::cout << "Type output image size x and y > ";
//    std::cin >> out_img_size.width;
//    std::cin >> out_img_size.height
    
    std::cout << "Type prefer focal length in pixel unit > ";
    std::cin >> f_;
    
    double theta_x, theta_y;
    std::cout << "Type rotation degree around x axis > ";
    std::cin >> theta_x;
    theta_x = theta_x * M_PI /180.0;
    cv::Mat Rx = (cv::Mat_<double>(3,3) <<
                  1,            0,             0,
                  0, cos(theta_x), -sin(theta_x),
                  0, sin(theta_x),  cos(theta_x));
    std::cout << "Type rotation degree around y axis > ";
    std::cin >> theta_y;
    theta_y = theta_y * M_PI /180.0;
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
    
    cv::Mat dst;
    cv::remap(src, dst, mapx, mapy, cv::INTER_LINEAR); // Rectify
    
    cv::namedWindow("src", CV_GUI_NORMAL|CV_WINDOW_NORMAL|CV_WINDOW_KEEPRATIO);
    cv::imshow("src", src);
    cv::moveWindow("src", 0, 0);
    cv::namedWindow("dst", CV_GUI_NORMAL|CV_WINDOW_NORMAL|CV_WINDOW_KEEPRATIO| CV_GUI_EXPANDED);
    cv::imshow("dst", dst);
    cv::moveWindow("dst", 0, 0);
    cv::waitKey();
    cv::imwrite("out.png", dst);
    
    return 0;
}


void loadPrameters(std::string filename, double& f, double& f0, cv::Point2d& center, cv::Size& img_size, std::vector<double>& a)
{
    cv::FileStorage fs(filename, cv::FileStorage::READ);
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


void theta2radius(std::vector<double>& r, double& rad_step, double& f, double& f0, cv::Point2d& center, cv::Size2i& img_size, std::vector<double>& a)
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
    int theta_size = round(max_r) * PRECISION + 10; // If PRECISION = 10, r = {0, 0.1, 0.2, 0.3, ...}
    
    std::vector<double> theta(theta_size);
    for (int i = 0; i < theta_size; ++i) {
        double r = (double)i / PRECISION;
        theta[i] = r/f0;
        for (int j = 0; j != a.size(); ++j) {
            theta[i] += a[j] * pow(r/f0, j*2+3);
        }
        theta[i] *= f0/f;
    }
    
    std::ofstream ofs("graph.dat");
    int r_size = 1000;
    r.resize(r_size);
    int j = 1; // j/PRECISION: radius
    rad_step = *(theta.end()-1) / r_size; // 0 ~ theta[end] radian
    for (int i = 0; i < r_size; ++i) {
        double rad = rad_step * i;
        for (; j < theta_size; ++j) {
            if (theta[j] > rad) {
                r[i] = ((i*rad_step - theta[j-1]) / (theta[j]-theta[j-1]) + j-1) / PRECISION; // See my note on 2014/6/6
//                std::cout << "(i,j): ("<< i << "," << j << ")\t" << theta[j] << "\t" << rad << "\t" << r[i] << std::endl;
                ofs << rad * 180 / M_PI << ' ' << rad << ' ' << r[i] << ' ' << f * rad << std::endl;
                break;
            }
        }
    }
    ofs.close();
}