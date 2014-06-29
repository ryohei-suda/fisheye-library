//
//  main.cpp
//  CalibrationPattern
//
//  Created by Suda on 2014/03/23.
//  Copyright (c) 2014年 Ryohei Suda. All rights reserved.
//

#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <pthread.h>

void mycallback(int state, void* data)
{
    int a;
    a = 1+1;
}

int main(int argc, const char * argv[])
{
    std::cout << "This shows zonation patterns to calibrate a camera\n" << std::endl;
    
    int height = 1080, width = 1920, line_width = 100;
    
    
    // Make white image
    cv::Mat pattern0 = cv::Mat::ones(height, width, CV_8UC1) * 255;
    cv::imwrite("/Users/ryohei/Dropbox/univ/lab/Images/pattern0.png", pattern0);
    
    // Make vertical pattern
    cv::Mat pattern1 = cv::Mat::ones(height, width, CV_8UC1) * 255;
    for(int i = line_width/2; i < width; i += 2*line_width){
        cv::line(pattern1, cv::Point2d(i, 0), cv::Point2d(i, height), cv::Scalar(0), line_width);
    }
    cv::imwrite("/Users/ryohei/Dropbox/univ/lab/Images/pattern1.png", pattern1);
    
    // Make horizontal pattern
    cv::Mat pattern2 = cv::Mat::ones(height, width, CV_8UC1) * 255;
    for(int i = line_width/2; i < height; i += 2*line_width){
        cv::line(pattern2, cv::Point2d(0, i), cv::Point2d(width, i), cv::Scalar(0), line_width);
    }
    cv::imwrite("/Users/ryohei/Dropbox/univ/lab/Images/pattern2.png", pattern2);
    
    line_width *= sqrt(2);
    
    // Make right-down aslant pattern
    cv::Mat pattern3 = cv::Mat::ones(height, width, CV_8UC1) * 255;
    for(int i = 0; i < height; i++){
        int step = i % (2 * line_width) - line_width;
        for(int left = step, right = line_width+step; left < width; left += 2*line_width, right += 2*line_width){
            
            cv::line(pattern3, cv::Point2d(left, i), cv::Point2d(right, i), cv::Scalar(0), 1);
        }
    }
    cv::imwrite("/Users/ryohei/Dropbox/univ/lab/Images/pattern3.png", pattern3);
    
    // Make left-down aslant pattern
    cv::Mat pattern4 = cv::Mat::ones(height, width, CV_8UC1) * 255;
    for(int i = 0; i < height; i++){
        int step = i % (2 * line_width);
        for(int left = -step, right = line_width-step; left < width; left += 2*line_width, right += 2*line_width){
            
            cv::line(pattern4, cv::Point2d(left, i), cv::Point2d(right, i), cv::Scalar(0), 1);
        }
    }
    cv::imwrite("/Users/ryohei/Dropbox/univ/lab/Images/pattern4.png", pattern4);
 
    // Make an image to check if patterns 3 and 4 are vertical
    cv::imwrite("/Users/ryohei/Dropbox/univ/lab/Images/pattern5.png", cv::min(pattern3, pattern4));
    
    
    
//    int* pointer = &height;
//    
//    cv::namedWindow("Pattern", CV_WINDOW_NORMAL);
////    cv::setWindowProperty("Pattern", CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
//    cv::imshow("Pattern", pattern1);
//    
//    cv::createButton("button", mycallback, pointer, CV_PUSH_BUTTON, 0);
    cv::waitKey();
    
    return 0;
}

