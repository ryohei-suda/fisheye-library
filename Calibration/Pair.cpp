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
        for (std::vector<std::vector<IncidentVector *> >::iterator k = edge[i].begin();  k != edge[i].end(); ++k) { // For each line
            for (std::vector<IncidentVector *>::iterator alpha = k->begin(); alpha != k->end(); ++alpha) { // For each point
                (*alpha)->calcM();
            }
        }
    }
}

void Pair::calcMd()
{
    for (int i = 0; i < 2; ++i) {
        for (std::vector<std::vector<IncidentVector *> >::iterator line = edge[i].begin(); line != edge[i].end(); line++) {
            for (std::vector<IncidentVector *>::iterator point = line->begin(); point != line->end(); ++point) {
                (*point)->calcDerivatives();
            }
        }
    }
}

void Pair::calcNormal()
{
    for (int i = 0; i < 2; ++i) {
        normalVector[i].clear();
        normalValue[i].clear();
            
        for (std::vector<std::vector<IncidentVector *> >::iterator k = edge[i].begin();  k != edge[i].end(); ++k) { // For each line
            cv::Mat Mk = cv::Mat::zeros(3, 3, CV_64F);
            for (std::vector<IncidentVector *>::iterator alpha = k->begin(); alpha != k->end(); ++alpha) { // For each point
                cv::Mat m((*alpha)->m);
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
        for (std::vector<cv::Mat>::iterator n = normalVector[i].begin(); n != normalVector[i].end(); ++n) { // For each line
            cv::Mat nk = n->row(2);
            Ng += nk.t() * nk;
        }
        
        cv::Mat eigenValues, eigenVectors;
        cv::eigen(Ng, eigenValues, eigenVectors);
        lineVector[i] = eigenVectors;
        lineValue[i] = eigenValues;
    }
}

void Pair::calcMc()
{
    for (int i = 0; i < 2; ++i) {
        Mc[i].clear();
        
        for (std::vector<std::vector<IncidentVector *> >::iterator k = edge[i].begin(); k != edge[i].end() ; ++k) { // For each line
            Pair::C c;
            for (std::vector<IncidentVector *>::iterator alpha = k->begin(); alpha != k->end(); ++alpha) { // For each point
                
                cv::Mat m((*alpha)->m);
                for (int j = 0; j < IncidentVector::nparam; ++j) {
                    cv::Mat mc((*alpha)->derivatives[j]);
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
        
        for (std::vector<std::vector<IncidentVector *> >::iterator k = edge[i].begin(); k != edge[i].end() ; ++k) { // For each line
            Pair::Cc cc;
            for (std::vector<IncidentVector *>::iterator alpha = k->begin(); alpha != k->end(); ++alpha) { // For each point
                
                for (int j = 0; j < IncidentVector::nparam; ++j) {
                    cv::Mat mc1((*alpha)->derivatives[j]);
                    for(int l = 0; l < IncidentVector::nparam; ++l) {
                        cv::Mat mc2((*alpha)->derivatives[l]);
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
                }
            }
        }
        Ncc[i] = cc;
    }
}

void Pair::calcLc()
{
    
    for (int i = 0; i < 2; ++i) {
        lc[i].empty();
        cv::Mat lg1 = lineVector[i].row(0).t();
        cv::Mat lg2 = lineVector[i].row(1).t();
        cv::Mat lg = lineVector[i].row(2).t();
        double mu1 = lineValue[i].at<double>(0);
        double mu2 = lineValue[i].at<double>(1);
        double mu = lineValue[i].at<double>(2);
            
        for (int k = 0; k < IncidentVector::nparam; ++k) {
            lc[i].push_back(- (lg1.dot(Nc[i].ms[k]*lg)*lg1 / (mu1-mu)) - (lg2.dot(Nc[i].ms[k]*lg)*lg2 / (mu2-mu)));
        }
    }
    
}

void Pair::calcDerivatives()
{
    calcNormal();
    calcLine();
    calcMd();
    calcMc();
    calcMcc();
    calcNc();
    calcNcc();
    calcLc();
    
}