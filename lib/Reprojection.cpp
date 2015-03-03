//
//  Reprojection.cpp
//  Reprojection
//
//  Created by Ryohei Suda on 2014/09/12.
//  Copyright (c) 2014年 RyoheiSuda. All rights reserved.
//

#include "Reprojection.h"

int Reprojection::precision = 20;

void Reprojection::loadPrameters(std::string filename)
{
    cv::FileStorage fs(filename, cv::FileStorage::READ);
    
    double f, f0;
    cv::Point2d center;
    cv::Size2i img_size;
    std::vector<double> a;
    
    if (!fs.isOpened()) {
        std::cerr << filename << " cannnot be opened!" << std::endl;
        exit(-1);
    }
    fs["f"] >> f;
    fs["f0"] >> f0;
    fs["center"] >> center;
    fs["img_size"] >> img_size;
    fs["projection"] >> projection;
    
    a.clear();
    cv::FileNode fn = fs["a"];
    cv::FileNodeIterator it = fn.begin();
    for (; it != fn.end(); ++it) {
        a.push_back((static_cast<double>(*it)));
    }
    
    IncidentVector::setProjection(projection);
    IncidentVector::setParameters(f, f0, a, img_size, center);
}


void Reprojection::theta2radius()
{
    double max_r = 0;
    // Calculate the longest distance between optical center and image corner
    if (IncidentVector::getImgSize().width - IncidentVector::getCenter().x > IncidentVector::getImgSize().width / 2.0) {
        max_r += pow(IncidentVector::getImgSize().width - IncidentVector::getCenter().x, 2);
    } else {
        max_r += pow(IncidentVector::getCenter().x, 2);
    }
    if (IncidentVector::getImgSize().height - IncidentVector::getCenter().y > IncidentVector::getImgSize().height / 2.0) {
        max_r += pow(IncidentVector::getImgSize().height - IncidentVector::getCenter().y, 2);
    } else {
        max_r += pow(IncidentVector::getCenter().y, 2);
    }
    max_r = sqrt(max_r);
//    max_r = 2000;
    int theta_size = max_r * precision + 10; // If PRECISION = 10, r = {0, 0.1, 0.2, 0.3, ...}
    
    r2t.resize(theta_size);
    cv::Point2d p(0,0);
    IncidentVector *iv = nullptr;
    std::cout << IncidentVector::getProjectionName()  << "\t" << IncidentVector::getProjection()<< std::endl;
    switch (IncidentVector::getProjection()) {
        case 0:
            iv = new StereographicProjection(p);
            std::cout << "Stereographic Projection" << std::endl;
            break;
        case 1:
            iv = new OrthographicProjection(p);
            std::cout << "Orthographic Projection" << std::endl;
            break;
        case 2:
            iv = new EquidistanceProjection(p);
            std::cout << "Equidistance Projection" << std::endl;
            break;
        case 3:
            iv = new EquisolidAngleProjection(p);
            std::cout << "Equisolid Angle Projection" << std::endl;
            break;
    }
    for (int i = 0; i < theta_size; ++i) {
        double r = (double)i / precision;
        
        r2t[i] = iv->aoi(r);
    }
    
    int r_size = max_r * precision + 10;
//    r_size = 2000 * precision;
    t2r.resize(r_size);
    int j = 1; // j/PRECISION: radius
    rad_step = r2t[theta_size-1] / r_size; // 0 ~ theta[end] radian
    rad_step = 2.2 / (sqrt(pow(IncidentVector::getImgSize().height,2) + pow(IncidentVector::getImgSize().width,2))/2) / precision; // 130 degree of HFOV
    for (int i = 0; i < r_size; ++i) {
        double rad = rad_step * i;
        for (; j < theta_size; ++j) {
            if (r2t[j] >= rad) {
                t2r[i] = ((i*rad_step - r2t[j-1]) / (r2t[j]-r2t[j-1]) + j-1) / precision; // See my note on 2014/6/6
                break;
            }
        }
    }
}

void Reprojection::saveTheta2Radius(std::string filename)
{
    std::ofstream ofs(filename);
    
    for (int i = 0; i < t2r.size(); ++i) {
        ofs << std::fixed << std::setprecision(8) << t2r[i] << ' ' << rad_step * i << ' ' << rad_step * i * 180 / M_PI << std::endl;
    }
    
    ofs.close();
}

void Reprojection::saveRadius2Theta(std::string filename)
{
    std::ofstream ofs(filename);
    
    for (int i = 0; i < r2t.size(); ++i) {
        ofs << std::fixed << std::setprecision(8) << (double)i/precision << ' ' << r2t[i] << ' ' << r2t[i] * 180 / M_PI << std::endl;
    }
    
    ofs.close();
}


void Reprojection::calcMaps(int step_x, int step_y, double theta_x, double theta_y, double theta_z, double f_, cv::Mat& mapx, cv::Mat& mapy)
{
    mapx.create(IncidentVector::getImgSize().height, IncidentVector::getImgSize().width, CV_32FC1);
    mapy.create(IncidentVector::getImgSize().height, IncidentVector::getImgSize().width, CV_32FC1);
    
    cv::Mat Rx = (cv::Mat_<double>(3,3) <<
                  1,            0,             0,
                  0, cos(theta_x), -sin(theta_x),
                  0, sin(theta_x),  cos(theta_x));
    cv::Mat Ry = (cv::Mat_<double>(3,3) <<
                  cos(theta_y),  0, sin(theta_y),
                  0,             1,            0,
                  -sin(theta_y), 0, cos(theta_y));
    cv::Mat Rz = (cv::Mat_<double>(3,3) <<
                  cos(theta_z), -sin(theta_z), 0,
                  sin(theta_z),  cos(theta_z), 0,
                  0,                        0, 1);
    cv::Mat R = Rz * Ry * Rx;
    
    
    int height = IncidentVector::getImgSize().height;
    int width = IncidentVector::getImgSize().width;
    double xd, yd, xn, yn, zn, theta;
    cv::Mat p3(3,1,CV_64F), real;;
    p3.at<double>(2) = f_;
    cv::Point2d final;
    for (int y_ = 0; y_ < height; ++y_) { // y
        for (int x_ = 0; x_ < width; ++x_) { // x
            
            xd = x_ - width/2.0 + step_x;
            yd = y_ - height/2.0 + step_y;
            p3.at<double>(0) = xd;
            p3.at<double>(1) = yd;
            real = 1.0/sqrt(pow(xd,2) + pow(yd,2) + pow(f_,2)) * R * p3;
            
            xn = real.at<double>(0,0);
            yn = real.at<double>(1,0);
            zn = real.at<double>(2,0);
            theta = atan2(sqrt(1-pow(zn,2)), zn);
            if (t2r.size() <= (int)(theta/rad_step)) {
                mapx.at<float>(y_,x_) = -1;
                mapy.at<float>(y_,x_) = -1;
                continue;
            }
            final = IncidentVector::getCenter() + t2r[(int)(theta/rad_step)] / sqrt(1-pow(zn,2)) * cv::Point2d(xn,yn);
            
            mapx.at<float>(y_,x_) = final.x;
            mapy.at<float>(y_,x_) = final.y;
        }
    }
}