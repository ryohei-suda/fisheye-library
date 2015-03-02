//
//  main.cpp
//  Reconstruction
//
//  Created by Ryohei Suda on 2015/02/01.
//  Copyright (c) 2015年 RyoheiSuda. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>
//#include <opencv2/nonfree/nonfree.hpp>
//#include <opencv2/legacy/legacy.hpp>
#include "../Reprojection/Reprojection.h"
#include <sys/stat.h>

int main(int argc, const char * argv[])
{
//    cv::initModule_nonfree(); // Needs for surf detection
    
    std::string dir_i = "images";
    std::string dir_m = "matches";
    std::string dir_r = "outReconstruct";
    mkdir(dir_i.c_str(), 0777);
    mkdir(dir_m.c_str(), 0777);
    mkdir(dir_r.c_str(), 0777);
    std::ofstream list(dir_m+"/lists.txt");
    
    Reprojection reproj;
    double f_new;
    double degree;
    std::string param;
    
    for (int i = 0; i < argc; ++i) {
        std::cout << argv[i] << " ";
    }
    std::cout << std::endl;
    
    if (argc > 4) {
        param = std::string(argv[1]);
        f_new = atof(argv[2]);
        degree = atof(argv[3]);
    } else {
        std::cout << "Type parameter file name > ";
        std::cin >> param;
        std::cout << "Type a prefer focal length > ";
        std::cin >> f_new;
        std::cout << "Type a rotation degree > ";
        std::cin >> degree;
    }
    reproj.loadPrameters(param);
    
    // Print parameters
    std::cout << "f: " << IncidentVector::getF() << "\nf0: " << IncidentVector::getF0() << std::endl;
    std::cout << "center: " << IncidentVector::getCenter() << std::endl;
    std::cout << "image size: " << IncidentVector::getImgSize() << std::endl;
    std::cout << "ai: ";
    for (std::vector<double>::iterator it = IncidentVector::getA().begin(); it != IncidentVector::getA().end(); ++it) {
        std::cout << *it << '\t';
    }
    std::cout << std::endl;
    
//    int shift;
//    std::cout << "Type a shift of an image in y axis > ";
//    std::cin >> shift;
    
    reproj.theta2radius();
    //    reproj.saveRadius2Theta("Stereographic.dat");
    
    int const d = 5;
    
    cv::Mat mapx[d];
    cv::Mat mapy[d];
    
    double theta_x[] = {0,   -degree,   -degree,   -degree,   -degree, -degree, -degree, -degree, -degree};
    double theta_y[] = {0,     0,     0,     0,     0,   0,   0,   0,   0};
    double theta_z[] = {0,     0,     90, 180,   -90,  135, 45, 225,  -45};
//    int y[d]          = {0, shift, shift, shift, shift,-950,-950,-950,-960,   0,   0,   0,   0};
    
    for (int i = 0; i < d; ++i) { // Convert from degree to radian
//        reproj.calcMaps(0, y[i], theta_x[i]*M_PI/180.0, theta_y[i]*M_PI/180.0, theta_z[i]*M_PI/180.0, f_new, mapx[i], mapy[i]);
        reproj.calcMaps(0, 0, theta_x[i]*M_PI/180.0, theta_y[i]*M_PI/180.0, theta_z[i]*M_PI/180.0, f_new, mapx[i], mapy[i]);
    }
    
    int count = 4;
    while (true) {
        std::string srcname;
        if (argc > 4) {
            if (count == argc) {
                break;
            }
            srcname = std::string(argv[count]);
        } else {
            std::cout << "Type source image file name > ";
            if (!(std::cin >> srcname)) {
                break;
            }
        }
        cv::Mat src = cv::imread(srcname);
        if(src.empty()) {
            std::cout << "Cannot read " << srcname << "!" << std::endl;
            continue;
        }
        
        cv::Mat dst;
        for (int j = 0; j < d; ++j) {
            int npos = (int)srcname.find_last_of(".");
            std::string outname = srcname.substr(0,npos)+"."+std::to_string(j)+".png";
            std::cout << "Writing " << outname << " ..." << std::flush;
            
            cv::remap(src, dst, mapx[j], mapy[j], cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(0,0,0)); // Rectify
            cv::imwrite(dir_i +"/"+outname, dst);
            
            list << outname << "; " << IncidentVector::getImgSize().width << "; " << IncidentVector::getImgSize().height << "; " << f_new << "; 0; " << IncidentVector::getCenter().x << "; 0; " << f_new << "; " << IncidentVector::getCenter().y << "; 0; 0; 1" << std::endl;
            
            std::cout << " done" << std::endl;
        }
        ++count;
    }
    list.close();
    
    std::cout << "\nFor 3D reconstruction, execute the following command." << std::endl;
    std::cout << "$ <openMVG dir>/software/SfM/openMVG_main_computeMatches -i " << dir_i << " -e *.png " << " -o " << dir_m << std::endl;
    std::cout << "$ <openMVG dir>/software/SfM/openMVG_main_IncrementalSfM -i " << dir_i  << " -m " << dir_m << " -o " << dir_r << " -p 1" << std::endl;
    std::cout << "$ cd " << dir_r << "/PMVS" << std::endl;
    std::cout << "$ <CMVS-PMVS dir>/main/pmvs2 ./ pmvs_options.txt" << std::endl;
    /*
     
     // Reading images
     std::string img1name, img2name;
     std::cout << "Type the first image file > ";
     std::cin >> img1name;
     std::cout << "Type the second image file > ";
     std::cin >> img2name;
     
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
    */
     
    return 0;
}
