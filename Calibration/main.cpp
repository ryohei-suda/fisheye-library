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
#include <fstream>
#include <sstream>
#include <Eigen/Core>
#include <opencv2/opencv.hpp>
#include "IncidentVector.h"
#include "Pair.h"
#include "tinyxml2.h"

#define A_SIZE 5

void loadData(std::string, std::vector<Pair>&, double&, cv::Point2d&, cv::Size2i&);
std::string eraseSideWhiteSpace(std::string);

//大文字の後に付いてるcは微分を表している
double J1(std::vector<Pair>&); // 共線性
double J1c(std::vector<Pair>&, int);
double J1cc(std::vector<Pair>&, int, int);
double J2(std::vector<Pair>&); // 平行性
double J2c(std::vector<Pair>&, int);
double J2cc(std::vector<Pair>&, int, int);
double J3(std::vector<Pair>&); // 直交性
double J3c(std::vector<Pair>&, int);
double J3cc(std::vector<Pair>&, int, int);
double rad2deg(double r);

int main(int argc, const char * argv[])
{
    //    ( 1 ) 初期値を与える（例えば光軸点は画像フレームの中心，f を公称焦点距離，a1 = a2
    //    = ... = 0）．そして，それに対するJ の値をJ0 とし，C = 0.0001 と置く．
    cv::Point2d center; //(1200.0/2.0, 900.0/2.0);
    cv::Size2i img_size;
    double f; // = 1.4/0.00318; // f is pixel unit
    double J0, C = 0.0001;
    double const f0 = 150; // Scale constant;
    std::vector<double> a(A_SIZE, 0.0);
    std::vector<Pair> edges;
    double gamma[3];
    
    std::string filename; // "/Users/ryohei/Dropbox/univ/lab/Images/data.dat"
    std::cout << "Type filename> ";
    std::cin >> filename;
    loadData(filename, edges, f, center, img_size); // Loading...
    std::cout << "Orthogonal pairs: " << edges.size() << std::endl;
    double lines = 0;
    for (std::vector<Pair>::iterator it = edges.begin(); it != edges.end(); ++it) {
        lines += it->edge[0].size();
        lines += it->edge[1].size();
    }
    std::cout << "Lines: " << lines << std::endl;
    
    // Show an image of all edges
//    cv::Mat img = cv::Mat::zeros(img_size.height, img_size.width, CV_8UC1);
//    cv::namedWindow("edges", CV_WINDOW_NORMAL);
//    for (int i=0; i < edges.size(); ++i) {
//        for (int j=0; j < edges[i].edge[0].size(); ++j) {
//            for (int k=0; k < edges[i].edge[0][j].size(); ++k) {
//                img.at<uchar>(edges[i].edge[0][j][k].point.y, edges[i].edge[0][j][k].point.x) = 255;
//            }
//            cv::imshow("edges", img);
//            cv::waitKey();
//        }
//        for (int j=0; j < edges[i].edge[1].size(); ++j) {
//            for (int k=0; k < edges[i].edge[1][j].size(); ++k) {
//                img.at<uchar>(edges[i].edge[1][j][k].point.y, edges[i].edge[1][j][k].point.x) = 255;
//            }
//            cv::imshow("edges", img);
//            cv::waitKey();
//        }
//        img = cv::Mat::zeros(img_size.height, img_size.width, CV_8UC1);
//    }
//    cv::imwrite("edges.png", img);
    
    std::cout << "Center:\t" << center << std::endl;
    std::cout << "     f:\t" << f << std::endl;
    std::cout << "    f0:\t" << f0 << std::endl;
    for (int i = 0; i < A_SIZE; ++i) {
        std::cout << "    a" << i << ":\t" << a[i] << std::endl;
    }
    IncidentVector::setParameters(f, f0, a, center);
    
    for (std::vector<Pair>::iterator pair = edges.begin(); pair != edges.end(); ++pair) {
        pair->calcM();
        pair->calcNormal();
        pair->calcLine();
    }
    
    gamma[0] = J1(edges);
    gamma[1] = J2(edges);
    gamma[2] = J3(edges);
    J0 = gamma[0] / gamma[0] + gamma[1] / gamma[1] + gamma[2] / gamma[2];
    std::cout << "J1  \t" << gamma[0] << "\nJ2  \t" << gamma[1] << "\nJ3  \t" << gamma[2] << std::endl;

    int iterations = 0;
    while (true) {
        //    ( 2 ) 式(3) によって入射角θκα を計算し，式(6) によって入射光ベクトルmκα を計算し，
        //    式(7), (10), (13) によって∂mκα/∂c を計算する(c = u0, v0, f, a1, a2, ...)．
        for (std::vector<Pair>::iterator pair = edges.begin(); pair != edges.end(); ++pair) {
            pair->calcM();
            pair->calcDerivatives();
        }
        
        //    ( 3 ) それらを用いてJ のパラメータに関する1 階微分Jc，2 階微分Jcc0 を計算する
        cv::Mat left(IncidentVector::nparam, IncidentVector::nparam, CV_64F);
        cv::Mat right(IncidentVector::nparam, 1, CV_64F);
    
        for (int i = 0; i < IncidentVector::nparam; ++i) {
            for (int j = 0; j < IncidentVector::nparam; ++j) {
                // (1+C) isn't calculated here, look at the next while loop
                left.at<double>(i, j) = J1cc(edges, i, j) / gamma[0] + J2cc(edges, i, j) / gamma[1] + J3cc(edges, i, j) / gamma[2];
            }
            right.at<double>(i) = J1c(edges, i) / gamma[0] + J2c(edges, i) / gamma[1] + J3c(edges, i) / gamma[2];
        }
    
    
        cv::Mat delta;
        double J_;
        while (true) {
            ++iterations;
            cv::Mat cmat = cv::Mat::ones(IncidentVector::nparam, IncidentVector::nparam, CV_64F); // To calculate (1+C)
            for (int i = 0; i < IncidentVector::nparam; ++i) {
                cmat.at<double>(i,i) = 1+C;
            }
            //    ( 4 ) 次の連立1次方程式を解いてΔu0, Δv0, Δf, Δa1, ... を計算する．
            cv::solve(left.mul(cmat), -right, delta);
            std::cout << "------------------------ Iteration #: "<< iterations << " -------------------------" << std::endl;
            std::cout << "Delta: " << delta << std::endl;
    
            //    ( 5 ) 次のように˜u0, ˜v0, ˜ f, ˜a1, a2, ... を計算し，それに対するJ の値を˜ J とする．
            //    ˜u0 = u0+Δu0, ˜v = v0+Δv0, ˜ f = f+Δf, ˜a1 = a1+Δa1, ˜a2 = a2+Δa2, ... (48)
            cv::Point2d center_(center.x + delta.at<double>(0), center.y + delta.at<double>(1));
            double f_ = f + delta.at<double>(2);
            std::vector<double> a_;
            for (int i = 0; i < A_SIZE; ++i) {
                a_.push_back(a[i] + delta.at<double>(i+3));
            }
            
            // Recalculate m and relations based on new parameters
            IncidentVector::setParameters(f_, f0, a_, center_);
            for (std::vector<Pair>::iterator pair = edges.begin(); pair != edges.end(); ++pair) {
                pair->calcM();
                pair->calcNormal();
                pair->calcLine();
            }
            
            double j1 = J1(edges), j2 = J2(edges), j3 = J3(edges);
            J_ =  j1 / gamma[0] + j2 / gamma[1] + j3 / gamma[2];
            std::cout << "C: " << C << "\tJ0: " << J0 << "\tJ_: " << J_;
            std::cout << "\tJ1_: " << j1 << "\tJ2_: " << j2 << "\tJ3_: " << j3 << std::endl;
    
            //    ( 6 ) ˜ J < J0 なら次へ進む．そうでなければC Ã 10C としてステップ(4) に戻る．
            if ( J_  < J0) {
                center = center_;
                f = f_;
                a = a_;
                break;
            } else {
                C *= 10;
            }
        }
    
        //    ( 7 ) u0 Ã ˜u0, v0 Ã ˜v0, f Ã ˜ f, a1 Ã ˜a1, a2 Ã ˜a2, ... とし，jΔu0j < ²0, jΔv0j < ²0,
        //    jΔfj < ²f , jΔa1j < ²1, jΔa2j < ²2, ... ならu0, v0, f, a1, a2, ..., J を返して終了す
        //    る．そうでなければJ0 Ã J, C Ã C/10 としてステップ(2) に戻る
        bool flag = true;
        for (int i = 0; i < IncidentVector::nparam; ++i) {
            double epsilon = 0.001;
            if (i > 2) {
                epsilon = 1.0 / pow(10, i+2);
            }
            if (fabs(delta.at<double>(i)) >= epsilon) {
                flag = false;
                break;
            }
        }
    
        if (flag) {
            std::cout << "Center:\t" << center << std::endl;
            std::cout << "     f:\t" << f << std::endl;
            for (int i = 0; i < A_SIZE; ++i) {
                std::cout << "    a" << i << ":\t" << a[i] << std::endl;
            }
            
            
            break;
        } else {
            J0 = J_;
            C /= 10.0;
            IncidentVector::setParameters(f, f0, a, center);
            
            std::cout << "Center:\t" << center << std::endl;
            std::cout << "     f:\t" << f << std::endl;
            for (int i = 0; i < A_SIZE; ++i) {
                std::cout << "    a" << i << ":\t" << a[i] << std::endl;
            }
            
        }
    }
    
    cv::FileStorage fs_out("parameters.xml", cv::FileStorage::WRITE);
    fs_out << "center" << center;
    fs_out << "img_size" << img_size;
    fs_out << "f" << f;
    fs_out << "f0" << f0;
    fs_out << "a" << "[";
    for (std::vector<double>::iterator ai = a.begin(); ai != a.end(); ++ai) {
        fs_out << *ai;
    }
    fs_out << "]";
    
    std::cout << "END" << std::endl;
    return 0;
}


void loadData(std::string filename, std::vector<Pair>& edges, double& f, cv::Point2d& center, cv::Size2i& img_size)
{
    edges.clear();
    
    
    tinyxml2::XMLDocument doc;
    doc.LoadFile(filename.c_str());
    tinyxml2::XMLElement *root = doc.FirstChildElement("edges");
    
    double unit = atof(root->FirstChildElement("pixel_size")->GetText());
    f = atof(root->FirstChildElement("focal_length")->GetText()) / unit;
    IncidentVector::setF(f);

    img_size.width = atoi(root->FirstChildElement("width")->GetText());
    center.x = img_size.width / 2.0;
    img_size.height = atoi(root->FirstChildElement("height")->GetText());
    center.y = img_size.height / 2.0;
    IncidentVector::setCenter(center);
    
    std::stringstream ssdata;
    tinyxml2::XMLElement *pair = root->FirstChildElement("pair");
    while (pair != NULL) {
        Pair tmp;
        
        // edge1
        tinyxml2::XMLElement *edge1 = pair->FirstChildElement("edge1");
        tinyxml2::XMLElement *line = edge1->FirstChildElement("line");
        while (line != NULL) {
            std::vector<IncidentVector> edge; // One line of points
            tinyxml2::XMLElement *p = line->FirstChildElement("p");
            while (p != NULL) {
                cv::Point2d point;
                ssdata.str(p->GetText());
                ssdata >> point.x;
                ssdata >> point.y;
                edge.push_back(*(new IncidentVector(point)));
                ssdata.clear();
                p = p->NextSiblingElement("p");
            }
            tmp.edge[0].push_back(edge);
            line = line->NextSiblingElement("line");
        }
        
        // edge2
        tinyxml2::XMLElement *edge2 = pair->FirstChildElement("edge2");
        line = edge2->FirstChildElement("line");
        while (line != NULL) {
            std::vector<IncidentVector> edge; // One line of points
            tinyxml2::XMLElement *p = line->FirstChildElement("p");
            while (p != NULL) {
                cv::Point2d point;
                ssdata.str(p->GetText());
                ssdata >> point.x;
                ssdata >> point.y;
                edge.push_back(*(new IncidentVector(point)));
                ssdata.clear();
                p = p->NextSiblingElement("p");
            }
            tmp.edge[1].push_back(edge);
            line = line->NextSiblingElement("line");
        }
        
        edges.push_back(tmp);
        pair = pair->NextSiblingElement("pair");
    }
}

double J1(std::vector<Pair>& edges)
{
    double j1 = 0;

    for (std::vector<Pair>::iterator pair = edges.begin(); pair != edges.end(); ++pair) {
        for (int i = 0; i < 2; ++i) {
            for (std::vector<cv::Mat>::iterator nval = pair->normalValue[i].begin(); nval != pair->normalValue[i].end(); ++nval) {
                j1 += nval->at<double>(2);
            }
        }
    }
    
    return j1;
}

double J1c(std::vector<Pair>& edges, int c)
{
    double j1c = 0;
    
    for (std::vector<Pair>::iterator pair = edges.begin(); pair != edges.end(); ++pair) {
        for (int i = 0; i < 2; ++i) {
            std::vector<cv::Mat>::iterator nvec = pair->normalVector[i].begin();
            std::vector<Pair::C>::iterator mc = pair->Mc[i].begin();
            for (; mc != pair->Mc[i].end() && nvec != pair->normalValue[i].end(); ++mc, ++nvec) { // For each line
                cv::Mat nk = nvec->row(2).t();
                j1c += nk.dot(mc->at(c) * nk);
            }
        }
    }
    return j1c;
}

double J1cc(std::vector<Pair>& edges, int c1, int c2)
{
    double j1cc = 0;
    
    for (std::vector<Pair>::iterator pair = edges.begin(); pair != edges.end(); ++pair) {
        for (int i = 0; i < 2; ++i) {
            std::vector<cv::Mat>::iterator nvec = pair->normalVector[i].begin();
            std::vector<cv::Mat>::iterator nval = pair->normalValue[i].begin();
            std::vector<Pair::C>::iterator mc = pair->Mc[i].begin();
            std::vector<Pair::Cc>::iterator mcc = pair->Mcc[i].begin();
            
            for (;mc != pair->Mc[i].end() && mcc != pair->Mcc[i].end() && nvec != pair->normalVector[i].end() && nval != pair->normalValue[i].end(); ++mc, ++mcc, ++nvec, ++nval) { // For each line
            
                cv::Mat nk = nvec->row(2).t();
                cv::Mat nki[2] = {nvec->row(0).t(), nvec->row(1).t()};
                j1cc += nk.dot(mcc->at(c1, c2) * nk);
            
                for (int j = 0; j < 2; ++j) {
                    j1cc -= (nki[j].dot(mc->at(c1) * nk)) * (nki[j].dot(mc->at(c2) * nk)) / (nval->at<double>(j) - nval->at<double>(2));
                }
            
            }
        }
    }
    
    j1cc *= 2;
    return j1cc;
}

double J2(std::vector<Pair>& edges)
{
    double j2 = 0;

    
    for (std::vector<Pair>::iterator pair = edges.begin(); pair != edges.end(); ++pair) {
        for (int i = 0; i < 2; ++i) {
            j2 += pair->lineValue[i].at<double>(2);
        }
    }

    return j2;
}

double J2c(std::vector<Pair>& edges, int c)
{
    double j2c = 0;
    
    for (std::vector<Pair>::iterator pair = edges.begin(); pair != edges.end(); ++pair) {
        for (int i = 0; i < 2; ++i) {
            cv::Mat lg = pair->lineVector[i].row(2).t();
            j2c += lg.dot(pair->Nc[i].at(c) * lg);
        }
    }
    
    return j2c;
}

double J2cc(std::vector<Pair>& edges, int c1, int c2)
{
    double j2cc = 0;
    
    for (std::vector<Pair>::iterator pair = edges.begin(); pair != edges.end(); ++pair) {
        for (int i = 0; i < 2; ++i) {
            cv::Mat lg = pair->lineVector[i].row(2).t();
            j2cc += lg.dot(pair->Ncc[i].at(c1, c2) * lg);
            
            cv::Mat lgi[2] = {pair->lineVector[i].row(0).t(), pair->lineVector[i].row(1).t()};
            for (int j = 0; j < 2; ++j) {
                j2cc -= (lgi[j].dot(pair->Nc[i].at(c1) * lg) * lgi[j].dot(pair->Nc[i].at(c2) * lg)) / (pair->lineValue[i].at<double>(j) - pair->lineValue[i].at<double>(2));
            }
        }
    }
    
    j2cc *= 2;
    return j2cc;
}


double J3(std::vector<Pair>& edges)
{
    double j3 = 0;
    
    for (std::vector<Pair>::iterator pair = edges.begin(); pair != edges.end(); ++pair) {
        j3 += pow((pair->lineVector[0].row(2).t()).dot(pair->lineVector[1].row(2).t()), 2);
    }
    
    return j3;
}

double J3c(std::vector<Pair>& edges, int c)
{
    double j3c = 0;
    
    for (std::vector<Pair>::iterator pair = edges.begin(); pair != edges.end(); ++pair) {
        cv::Mat lg1 = pair->lineVector[0].row(2).t();
        cv::Mat lg2 = pair->lineVector[1].row(2).t();
        j3c += (lg1.dot(lg2)) * ((pair->lc[0].at(c)).dot(lg2))+(lg1).dot(pair->lc[1].at(c));
    }
    
    j3c *= 2;
    return j3c;
}

double J3cc(std::vector<Pair>& edges, int c1, int c2)
{
    double j3cc = 0;
    
    for (std::vector<Pair>::iterator pair = edges.begin(); pair != edges.end(); ++pair) {
        
        cv::Mat lg1 = pair->lineVector[0].row(2).t();
        cv::Mat lg2 = pair->lineVector[1].row(2).t();
        
        double tmp1 = (pair->lc[0].at(c1)).dot(lg2) + (lg1.dot(pair->lc[1].at(c1)));
        double tmp2 = (pair->lc[0].at(c2)).dot(lg2) + (lg1.dot(pair->lc[1].at(c2)));
        
        j3cc += tmp1 * tmp2;
    }
    
    j3cc *= 2;
    return j3cc;
}

double rad2deg(double r) {
    return (r * 180.0) / (std::atan(1.0) * 4.0);
}