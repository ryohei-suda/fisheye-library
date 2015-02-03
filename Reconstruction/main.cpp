//
//  main.cpp
//  Reconstruction
//
//  Created by Ryohei Suda on 2015/02/01.
//  Copyright (c) 2015年 RyoheiSuda. All rights reserved.
//

#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/nonfree/nonfree.hpp>
#include <opencv2/legacy/legacy.hpp>

int main(int argc, const char * argv[])
{
    cv::initModule_nonfree(); // Needs for surf detection
    
    // Reading images
    std::string img1name, img2name;
    std::cout << "Type the first image file > ";
    std::cin >> img1name;
    std::cout << "Type the second image file > ";
    std::cin >> img2name;
    
    cv::Mat img1, img2, prev, next;
    img1 = cv::imread(img1name, CV_LOAD_IMAGE_COLOR);
    img2 = cv::imread(img2name, CV_LOAD_IMAGE_COLOR);
    
    cv::cvtColor(img1, prev, CV_RGB2GRAY);
    cv::cvtColor(img2, next, CV_RGB2GRAY);
    
//    // Detect points to track
//    std::vector<cv::Point2f> corners1, corners2;
//    cv::goodFeaturesToTrack(prev, corners1, 200, 0.01, 10);
//    cv::cornerSubPix(prev, corners1, cv::Size(3,3), cv::Size(-1,-1), cv::TermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03));
//    
//    // Calculate optical flow
//    std::vector<uchar> status;
//    std::vector<float> err;
//    cv::calcOpticalFlowPyrLK(prev, next, corners1, corners2, status, err);
//    for (int i = 0; i < corners1.size(); ++i) { // Remove un matched points
//        if (status[i] == 0) {
//            status.erase(status.begin()+i);
//            corners1.erase(corners1.begin()+i);
//            corners2.erase(corners2.begin()+i);
//            --i;
//        }
//    }
//    
    // Detect feature points
    cv::Ptr<cv::FeatureDetector> detector = cv::FeatureDetector::create("SIFT");
    cv::Ptr<cv::DescriptorExtractor> extractor = cv::DescriptorExtractor::create("SIFT");
    cv::Ptr<cv::DescriptorMatcher> matcher = cv::DescriptorMatcher::create("FlannBased");
    
    std::vector<cv::KeyPoint> keypoints1, keypoints2;
    cv::Mat descriptor1, descriptor2;
    detector->detect(img1, keypoints1);
    extractor->compute(img1, keypoints1, descriptor1);
    detector->detect(img2, keypoints2);
    extractor->compute(img2, keypoints2, descriptor2);
    
    // Find corresponding points
    std::vector<cv::DMatch> dmatch;
    std::vector<cv::DMatch> dmatch12, dmatch21;
    matcher->match(descriptor1, descriptor2, dmatch12); // img1 -> img2
    matcher->match(descriptor2, descriptor1, dmatch21); // img2 -> img1
    
    for (int i = 0; i < dmatch12.size(); ++i) { // Check if the previous results are matcthing
        cv::DMatch m12 = dmatch12[i];
        cv::DMatch m21 = dmatch21[m12.trainIdx];
        
        if (m21.trainIdx == m12.queryIdx) {
            dmatch.push_back(m12);
        }
    }
    
    std::vector<cv::Point2f> points1, points2; // Store feature points
    points1.resize(dmatch.size()); points2.resize(dmatch.size());
    for (int i = 0; i < dmatch.size(); ++i) {
        cv::KeyPoint p = keypoints1[dmatch[i].queryIdx];
        points1[i] = p.pt;
        p = keypoints2[dmatch[i].trainIdx];
        points2[i] = p.pt;
    }
    
    cv::Mat pt1 = cv::Mat(points1), pt2 = cv::Mat(points2);
    
    for (auto & p : points1) {
        cv::circle(img1, p, 5, cv::Scalar(255,0,0));
    }
    for (int i = 0; i < points1.size(); ++i) {
        cv::line(img1, points1[i], points2[i], cv::Scalar(0,255,0));
    }
    cv::namedWindow(img1name, CV_WINDOW_NORMAL);
    cv::imshow(img1name, img1);
    cv::waitKey();
    
//    cv::Mat match_img;
//    cv::drawMatches(img1, keypoints1, img2, keypoints2, dmatch, match_img);
//    cv::namedWindow("Corresponding points", CV_WINDOW_NORMAL);
//    cv::imshow("Corresponding points", match_img);
//    cv::waitKey();
    
//    std::vector<cv::Point2f> points1(corners1.size());
//    std::vector<cv::Point2f> points2(corners2.size());
//    for (int i = 0; i < corners1.size(); ++i) {
//        points1[i] = corners1[i];
//        points2[i] = corners2[i];
//    }
    cv::Mat fund = cv::findFundamentalMat(pt1, pt2, CV_FM_LMEDS);
    
    cv::Mat H1, H2, img1r, img2r;
    img1r = cv::Mat(prev.size(), prev.type());
    img2r = cv::Mat(next.size(), next.type());
    cv::stereoRectifyUncalibrated(pt1, pt2, fund, img1.size(), H1, H2);
    cv::warpPerspective(prev, img1r, H1, prev.size());
    cv::warpPerspective(next, img2r, H2, next.size());
    
    // Show 2 warped images
    cv::Mat two_img(img1r.rows, img1r.cols*2, CV_8UC1);
    img1r.copyTo(two_img(cv::Rect(0,0,img1.cols,img1r.rows)));
    img2r.copyTo(two_img(cv::Rect(img1.cols,0,img1.cols,img1r.rows)));
    cv::namedWindow("Epiline", CV_WINDOW_NORMAL);
    cv::imshow("Epiline", two_img);
    cv::waitKey();
    
    int window_size = 3;
    cv::StereoSGBM sgbm;
    sgbm.minDisparity = 0;
    sgbm.numberOfDisparities = 32;
    sgbm.SADWindowSize = window_size;
    sgbm.P1 = 8*window_size*window_size;
    sgbm.P2 = 32*window_size*window_size;
    sgbm.uniquenessRatio = 10;
    sgbm.speckleWindowSize = 100;
    sgbm.speckleRange = 32;
    sgbm.disp12MaxDiff = -1;
    sgbm.fullDP = true;
    
    cv::Mat disparity;
    sgbm(img1r, img2r, disparity);
    
    cv::Mat show;
    disparity.convertTo(show, CV_8UC1, 255/(32.*16.));
    cv::namedWindow("disparity", CV_WINDOW_NORMAL);
    cv::imshow("disparity", show);
    cv::waitKey();
    
    return 0;
}
