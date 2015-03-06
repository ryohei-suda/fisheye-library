//
//  Pair.cpp
//  Calibration
//
//  Created by Ryohei Suda on 2014/06/02.
//  Copyright (c) 2014年 Ryohei Suda. All rights reserved.
//

#include "Pair.h"

void Pair::calcM()
{
    for (int i = 0; i < 2; ++i) {
        for(auto &line : edge[i]) {
            for (auto &point : line) {
                point->calcM();
            }
        }
    }
}

void Pair::calcMd()
{
    for (int i = 0; i < 2; ++i) {
        for(auto &line : edge[i]) {
            for (auto &point : line) {
                point->calcDerivatives();
            }
        }
    }
}

void Pair::calcNormal()
{
    for (int i = 0; i < 2; ++i) {
        normalVector[i].clear();
        normalValue[i].clear();
        
        for (auto &line : edge[i]) {
            cv::Mat Mk = cv::Mat::zeros(3, 3, CV_64F);
            for (auto &point : line) {
                cv::Mat m(point->m);
                Mk += m * m.t();
            }
            cv::Mat eigenValues, eigenVectors;
            cv::eigen(Mk, eigenValues, eigenVectors);
            normalVector[i].push_back(eigenVectors);
            normalValue[i].push_back(eigenValues);
        }
    }
}

void Pair::calcLine()
{
    for (int i = 0; i < 2; ++i) {
        
        cv::Mat Ng = cv::Mat::zeros(3, 3, CV_64F);
        for (auto &n : normalVector[i]) {
            cv::Mat nk = n.row(2);
            Ng += nk.t() * nk;
        }
        
        cv::Mat eigenValues, eigenVectors;
        cv::eigen(Ng, eigenValues, eigenVectors);
        lineVector[i] = eigenVectors;
        lineValue[i] = eigenValues;
    }
}

double Pair::calcF()
{
    
    cv::Mat h_ = lineVector[0].row(2).t();
    cv::Mat v_ = lineVector[1].row(2).t();
    std::vector<cv::Mat> ns_h, ns_v;
    for (auto &n : normalVector[0]) {
        ns_h.push_back(n.row(2).t());
    }
    for (auto &n : normalVector[1]) {
        ns_v.push_back(n.row(2).t());
    }
    cv::Mat h = calcVertical(v_, ns_h);
    cv::Mat v = calcVertical(h_, ns_v);
    double error = 0;
    
    
    w[0].clear();
    for (int i = 0; i < normalVector[0].size(); ++i ) { // First line pair
        std::vector<cv::Mat> ps;
        for (auto &n : edge[0][i]) {
            ps.push_back( cv::Mat(n->m) );
        }
        cv::Mat wn = calcVertical(h, ps);
        w[0].push_back(wn);
        
        for (auto &p : ps) { // Calculate error
            cv::Mat e = wn.t() * p;
            error += pow(e.at<double>(0), 2);
        }
    }
    
    w[1].clear();
    for (int i = 0; i < normalVector[1].size(); ++i ) { // Second line pair
        std::vector<cv::Mat> ps;
        for (auto &n : edge[1][i]) {
            ps.push_back( cv::Mat(n->m) );
        }
        cv::Mat wn = calcVertical(v, ps);
        w[1].push_back(wn);
        
        for (auto &p : ps) { // Calculate error
            cv::Mat e = wn.t() * p;
            error += pow(e.at<double>(0), 2);
        }
    }
    
    return error;
}

cv::Mat Pair::calcVertical(cv::Mat &d, std::vector<cv::Mat> &n)
{
    /*
    cv::Mat h = d.clone();
    cv::Mat b = cv::Mat::zeros(3, 1, CV_64FC1);
    b.at<double>(0) = - h.at<double>(2);
    b.at<double>(2) = h.at<double>(0);
    b *= 1.0 / sqrt(b.dot(b));
    cv::Mat g = b.cross(h);
    g *= 1.0 / sqrt(g.dot(g));
//    std::cout << b << "\t" << g << std::endl;
    
    cv::Mat k = cv::Mat::zeros(3, 3, CV_64FC1);
    for (auto &n : e) {
        k += n * n.t();
    }
    
    cv::Mat c1 = 2 * b.t() * k * g;//b.t() * k * g + g.t() * k * b;
    cv::Mat c2 = b.t() * k * b - g.t() * k * g;
    double c1d = c1.at<double>(0), c2d = c2.at<double>(0);
    cv::Mat v = c1d * b + (c2d-sqrt(c1d*c1d+c2d*c2d)) * g;
    v *= 1.0 / sqrt(v.dot(v));
    
//    std::cout << v.t() << std::endl;
    
//    v = sqrt(1.0/2.0 * (1+c2d/sqrt(c1d*c1d+c2d*c2d))) * b + sqrt(1.0/2.0 * (1-c2d/sqrt(c1d*c1d+c2d*c2d))) * g;
//    v *= 1.0 /sqrt(v.dot(v));
    
    return v;
    */
    
    cv::Mat b = cv::Mat::zeros(3,1, CV_64FC1);
    b.at<double>(0) = -d.at<double>(2);
    b.at<double>(2) = d.at<double>(0);
    b *= 1.0 / sqrt(b.dot(b)); // Make an unit vector
    cv::Mat g = b.cross(d);
    g *= 1.0 / sqrt(g.dot(g));// Make an unit vector
    
    cv::Mat k = cv::Mat::zeros(3, 3, CV_64FC1);
    for (auto &ni : n) {
        k += ni * ni.t();
    }
    
    double c1 = (g.t() * k).dot(b.t()) + (b.t() * k).dot(g.t());
    double c2 = (b.t() * k).dot(b.t()) - (g.t() * k).dot(g.t());
//    c1 = 2 * (b.t() * k).dot(g.t());
//    c2 = (b.t() * k).dot(b.t()) - (g.t() * k).dot(g.t());
    
    double part = c2 / (2 * sqrt(c1*c1+c2*c2));
    double s = sqrt(0.5 - part); // sin
    double c = sqrt(0.5 + part); // cos
    
    cv::Mat v;
    if (c1 > 0) { // 2phi in the 1st or 2nd quadrant -> the first quadrant
        v = -s*b + c*g;
    } else { // 2phi in the second quadrant -> the first quadrant
        v = s*b + c*g;
    }
//    v = c1 * b + (c2-sqrt(c1*c1+c2*c2)) *g;
    
    return v;
}

void Pair::calcMc()
{
    for (int i = 0; i < 2; ++i) {
        Mc[i].clear();
        
        for (auto &line : edge[i]) {
            Pair::C c;
            for (auto &point : line) {
                cv::Mat m(point->m);
                for (int j = 0; j < IncidentVector::nparam; ++j) {
                    cv::Mat mc(point->derivatives[j]);
                    c.ms[j] += mc * m.t();
                }
            }
            for (int j = 0; j < IncidentVector::nparam; ++j) {
                c.ms[j] += c.ms[j].t();
            }
            
            Mc[i].push_back(c);
        }
    }
    
}

void Pair::calcMcc()
{
    for (int i = 0; i < 2; ++i) {
        Mcc[i].clear();
        
        for (auto &line : edge[i]) {
            Pair::Cc cc;
            for (auto &point : line) {
                for (int j = 0; j < IncidentVector::nparam; ++j) {
                    cv::Mat mc1(point->derivatives[j]);
                    for(int l = 0; l < IncidentVector::nparam; ++l) {
                        cv::Mat mc2(point->derivatives[l]);
                        cc.ms[j][l] += mc1 * mc2.t();
                    }
                }
            }
            Mcc[i].push_back(cc);
        }
    }
}

void Pair::calcNc()
{
    
    for (int i = 0; i < 2; ++i) {
        Pair::C c;
        
        for (int j = 0; j < normalVector[i].size(); ++j) {
            cv::Mat nk1 = normalVector[i][j].row(0).t();
            cv::Mat nk2 = normalVector[i][j].row(1).t();
            cv::Mat nk = normalVector[i][j].row(2).t();
            double muk1 = normalValue[i][j].at<double>(0);
            double muk2 = normalValue[i][j].at<double>(1);
            double muk = normalValue[i][j].at<double>(2);
            
            for (int l = 0; l < IncidentVector::nparam; ++l) {
                cv::Mat mkc = Mc[i][j].ms[l];
                cv::Mat nkc = - ((nk1.dot(mkc*nk))/(muk1-muk) *nk1) - ((nk2.dot(mkc*nk)/(muk2-muk))*nk2);
                c.ms[l] += nkc * nk.t();
            }
        }
        
        for (int l = 0; l < IncidentVector::nparam; ++l) {
            c.ms[l] += c.ms[l].t();
        }
        
        Nc[i] = c;
    }
    
}

void Pair::calcNcc()
{
    for (int i = 0; i < 2; ++i) {
        Pair::Cc cc;
        
        for (int j = 0; j < normalVector[i].size(); ++j) {
            cv::Mat nk1 = normalVector[i][j].row(0).t();
            cv::Mat nk2 = normalVector[i][j].row(1).t();
            cv::Mat nk = normalVector[i][j].row(2).t();
            double muk1 = normalValue[i][j].at<double>(0);
            double muk2 = normalValue[i][j].at<double>(1);
            double muk = normalValue[i][j].at<double>(2);
            
            for (int l = 0; l < IncidentVector::nparam; ++l) {
                cv::Mat mkc1 = Mc[i][j].ms[l];
                cv::Mat nkc1 = - ((nk1.dot(mkc1*nk))/(muk1-muk) *nk1) - ((nk2.dot(mkc1*nk)/(muk2-muk))*nk2);
                for (int m = 0; m < IncidentVector::nparam; ++m) {
                    cv::Mat mkc2 = Mc[i][j].ms[m];
                    cv::Mat nkc2 = - ((nk1.dot(mkc2*nk))/(muk1-muk) *nk1) - ((nk2.dot(mkc2*nk)/(muk2-muk))*nk2);
                    cc.ms[l][m] += nkc1 * nkc2.t();
                    if (isnan(cc.ms[l][m].at<double>(0))) {
                        std::cout << cc.ms[l][m] << std::endl;
                        std::cout << nk1 << std::endl;
                        std::cout << nk2 << std::endl;
                        std::cout << nk << std::endl;
                        std::cout << muk1 << std::endl;
                        std::cout << muk2 << std::endl;
                        std::cout << muk << std::endl;
                        exit(99);
                    }
                }
            }
        }
        Ncc[i] = cc;
    }
}

void Pair::calcLc()
{
    
    for (int i = 0; i < 2; ++i) {
        Pair::C c;
        cv::Mat lg1 = lineVector[i].row(0).t();
        cv::Mat lg2 = lineVector[i].row(1).t();
        cv::Mat lg = lineVector[i].row(2).t();
        double mu1 = lineValue[i].at<double>(0);
        double mu2 = lineValue[i].at<double>(1);
        double mu = lineValue[i].at<double>(2);
            
        for (int k = 0; k < IncidentVector::nparam; ++k) {
            c.ms[k] = - (lg1.dot(Nc[i].ms[k]*lg)*lg1 / (mu1-mu)) - (lg2.dot(Nc[i].ms[k]*lg)*lg2 / (mu2-mu));
        }
        lc[i] = c;
    }
    
}

void Pair::calcFcc()
{
    Fc.clear();
    Fc.resize(IncidentVector::nparam, 0);
    Fcc.clear();
    Fcc.resize(IncidentVector::nparam);
    for (auto &fc : Fcc) {
        fc.resize(IncidentVector::nparam, 0);
    }
    
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < w[i].size(); ++j) {
            cv::Mat w_ = w[i].at(j).t();
            for (int c1 = 0; c1 < IncidentVector::nparam; ++c1) {
                Fc[c1] += (w_ * Mc[i].at(j).at(c1)).dot(w_);
                for (int c2 = 0; c2 < IncidentVector::nparam; ++c2) {
                    Fcc[c1][c2] += (w_ * Mcc[i].at(j).at(c1,c2)).dot(w_);
                }
            }
        }
    }
}

void Pair::calcDerivatives()
{
    calcMd();
    calcMc();
    calcMcc();
    calcNc();
    calcNcc();
    calcLc();
}