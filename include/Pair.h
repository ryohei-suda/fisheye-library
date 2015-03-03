//
//  Pair.h
//  Calibration
//
//  Created by Ryohei Suda on 2014/06/02.
//  Copyright (c) 2014年 Ryohei Suda. All rights reserved.
//

#ifndef Calibration_Pair_h
#define Calibration_Pair_h

#include <opencv2/opencv.hpp>
#include <cmath>
#include <vector>
#include "IncidentVector.h"
// Pair of orthgonal edges

class Pair
{
public:
    class Cc { // To store all second derivatives of M or N for a pair
    public:
        std::vector<std::vector<cv::Mat> > ms;
        Cc() {
            ms.clear();
            for (int i = 0; i < IncidentVector::nparam; ++i) {
                std::vector<cv::Mat> tmp(IncidentVector::nparam);
                ms.push_back(tmp);
            }
            init();
        };
    
        inline void init() {
            for (int i = 0; i < IncidentVector::nparam; ++i) {
                for (int j = 0; j < IncidentVector::nparam; ++j) {
                    ms[i][j] = cv::Mat::zeros(3, 3, CV_64F);
                }
            }
        }
        inline cv::Mat at(int c1, int c2) {
            return ms[c1][c2];
        }
    };
    
    class C { // To store all first derivatives of M or N for a pair
    public:
        std::vector<cv::Mat> ms;
        C() {
            ms.resize(IncidentVector::nparam);
            init();
        };
        
        inline void init() {
            for (int i = 0; i < IncidentVector::nparam; ++i) {
                ms[i] = cv::Mat::zeros(3, 3, CV_64F);
            }
        }
        
        inline cv::Mat at(int c) {
            return ms[c];
        }
        
    };
    
    std::vector<std::vector<IncidentVector *> > edge[2];
    std::vector<cv::Mat> normalVector[2]; // n1:(0,0), n2:(1,0), n:(2, 0) at<cv::Point3d>(2,0)
    std::vector<cv::Mat> normalValue[2];
    cv::Mat lineVector[2];
    cv::Mat lineValue[2];
    std::vector<C> Mc[2];
    std::vector<Cc> Mcc[2];
    C Nc[2];
    Cc Ncc[2];
    C lc[2];
    
    void calcM();
    
    // n_k and l_g
    void calcNormal();
    void calcLine();
    
    // Derivatives
    void calcMd(); // Calculate all derivatives of points
    void calcMc(); // For M
    void calcMcc();
    void calcNc();
    void calcNcc();
    void calcLc();
    void calcDerivatives();
    
    // For new optimization
    static cv::Mat calcVertical(cv::Mat &d, std::vector<cv::Mat> &e);
    double calcF();
    std::vector<cv::Mat> w[2];
    
    
};


#endif
