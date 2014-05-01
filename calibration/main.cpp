//
//  main.cpp
//  Calibration
//
//  Created by Ryohei Suda on 2014/03/30.
//  Copyright (c) 2014 Ryohei Suda. All rights reserved.
//

#include <iostream>
#include <cmath>
#include <vector>
#include <Eigen/Core>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/core/eigen.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "IncidentVector.h"

#define A_SIZE 5

double J1(std::vector<std::vector<cv::Point3d>>); // 共線性
double J2(std::vector<std::vector<cv::Point3d>>); // 平行性
double J3(std::vector<std::vector<cv::Point3d>>, std::vector<std::vector<cv::Point3d>>); // 直交性

double rad2deg(double r);


int main(int argc, const char * argv[])
{
    
    cv::Mat pattern = cv::imread("/Users/ryohei/Dropbox/univ/lab/Images/20_crrop.png");

    
//    ( 1 ) 初期値を与える（例えば光軸点は画像フレームの中心，f を公称焦点距離，a1 = a2
//    = ... = 0）．そして，それに対するJ の値をJ0 とし，C = 0.0001 と置く．
//      aは何個？とりあえずa1~a5
    cv::Point2d center(pattern.cols/2.0, pattern.rows/2.0);
    double f = 1.4, a[A_SIZE] = {0}, J, C = 0.0001;
    
    cv::Point3d p(1, 2, 3);
    cv::Mat m(p);
    std::cout << m * m.t() << std::endl;
    
    
//    ( 2 ) 式(3) によって入射角θκα を計算し，式(6) によって入射光ベクトルmκα を計算し，
//    式(7), (10), (13) によって∂mκα/∂c を計算する(c = u0, v0, f, a1, a2, ...)．
    
//    ( 3 ) それらを用いてJ のパラメータに関する1 階微分Jc，2 階微分Jcc0 を計算する．
    
//    ( 4 ) 次の連立1 次方程式を解いてΔu0, Δv0, Δf, Δa1, ... を計算する．
    
//    ( 5 ) 次のように˜u0, ˜v0, ˜ f, ˜a1, a2, ... を計算し，それに対するJ の値を˜ J とする．
//    ˜u0 = u0+Δu0, ˜v = v0+Δv0, ˜ f = f+Δf, ˜a1 = a1+Δa1, ˜a2 = a2+Δa2, ... (48)
    
//    ( 6 ) ˜ J < J0 なら次へ進む．そうでなければC Ã 10C としてステップ(4) に戻る．
    
//    ( 7 ) u0 Ã ˜u0, v0 Ã ˜v0, f Ã ˜ f, a1 Ã ˜a1, a2 Ã ˜a2, ... とし，jΔu0j < ²0, jΔv0j < ²0,
//    jΔfj < ²f , jΔa1j < ²1, jΔa2j < ²2, ... ならu0, v0, f, a1, a2, ..., J を返して終了す
//    る．そうでなければJ0 Ã J, C Ã C/10 としてステップ(2) に戻る

    return 0;
}

double J1(std::vector<std::vector<cv::Point3d>> lines)
{
    double j1 = 0;
    for(std::vector<std::vector<cv::Point3d>>::iterator k = lines.begin(); k != lines.end(); ++k ) { // For each line
        cv::Mat Mk = cv::Mat::zeros(3, 3, CV_64F);
        for(std::vector<cv::Point3d>::iterator p = k->begin(); p != k->end(); ++p) { // For each point
            cv::Mat m(*p);
            Mk += m * m.t();
        }
        
        Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> Mk_eigen;
        cv::Mat eigenValues;
        cv::eigen(Mk, eigenValues);
        j1 += eigenValues.at<double>(2,2);
	}
    return j1;
}

double J2(std::vector<std::vector<cv::Point3d>> planes)
{
    double j2 = 0;
    for(std::vector<std::vector<cv::Point3d>>::iterator g = planes.begin(); g != planes.end(); ++g) { // For each pattern plane
        cv::Mat Ng = cv::Mat::zeros(3, 3, CV_64F);
        for(std::vector<cv::Point3d>::iterator n = g->begin(); n != g->end(); ++n) { // For each line on a plane
            cv::Mat norm(*n);
            Ng += norm * norm.t();
        }
        Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> Ng_eigen;
        cv::Mat eigenValues;
        cv::eigen(Ng, eigenValues);
        j2 += eigenValues.at<double>(2,2);
    }
    
    return j2;
}

double J3(std::vector<std::vector<cv::Point3d>> planes1, std::vector<std::vector<cv::Point3d>> planes2)
{
    double j3 = 0;
    
    if (planes1.size() != planes2.size()) {
        std::cerr << "The number of lines mismatch!" << std::endl;
        exit(2);
    }
    
    for(std::vector<std::vector<cv::Point3d>>::iterator lines1 = planes1.begin(), lines2 = planes2.begin(); lines1 != planes1.end(); ++lines1, ++lines2) { // For each of lines
        for(std::vector<cv::Point3d>::iterator l1 = lines1->begin(); l1 != lines1->end(); ++l1) {
            for(std::vector<cv::Point3d>::iterator l2 = lines2->begin(); l2 != lines2->end(); ++l2) {
                j3 += std::pow(l1->dot(*l2), 2);
            }
        }
    }
    return j3;
}


double rad2deg(double r) {
    return (r * 180.0) / (std::atan(1.0) * 4.0);
}