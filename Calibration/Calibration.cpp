//
//  Calibration.cpp
//  Calibration
//
//  Created by Ryohei Suda on 2014/09/11.
//  Copyright (c) 2014年 RyoheiSuda. All rights reserved.
//

#include "Calibration.h"

void Calibration::setParameters(std::vector<Pair>& edges, double& f, double& f0, cv::Point2d& center, cv::Size2i& img_size, int a_size) {
    std::vector<double> a(a_size, 0);
    IncidentVector::setParameters(f, f0, a, img_size, center);
    this->edges = edges;
}

void Calibration::loadData(std::string filename) {
    edges.clear();
    
    tinyxml2::XMLDocument doc;
    doc.LoadFile(filename.c_str());
    tinyxml2::XMLElement *root = doc.FirstChildElement("edges");
    
    double unit = atof(root->FirstChildElement("pixel_size")->GetText());
    double f = atof(root->FirstChildElement("focal_length")->GetText()) / unit;
    IncidentVector::setF(f);
    
    cv::Size2i img_size;
    cv::Point2d center;
    img_size.width = atoi(root->FirstChildElement("width")->GetText());
    center.x = img_size.width / 2.0;
    img_size.height = atoi(root->FirstChildElement("height")->GetText());
    center.y = img_size.height / 2.0;
    IncidentVector::setImgSize(img_size);
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

void Calibration::save(std::string filename)
{
    cv::FileStorage fs_out(filename, cv::FileStorage::WRITE);
    fs_out << "center" << IncidentVector::getCenter();
    fs_out << "img_size" << IncidentVector::getImgSize();
    fs_out << "f" << IncidentVector::getF();
    fs_out << "f0" << IncidentVector::getF0();
    fs_out << "a" << "[";
    std::vector<double> a = IncidentVector::getA();
    for (std::vector<double>::iterator ai = a.begin(); ai != a.end(); ++ai) {
        fs_out << *ai;
    }
    fs_out << "]";
    
}

void Calibration::calibrate()
{
     this->save(std::string("parameters_00"".xml"));
    for (std::vector<Pair>::iterator pair = edges.begin(); pair != edges.end(); ++pair) {
        pair->calcM();
        pair->calcNormal();
        pair->calcLine();
    }
    
    gamma[0] = J1();
    gamma[1] = J2();
    gamma[2] = J3();
//    J0 = gamma[0] / gamma[0] + gamma[1] / gamma[1] + gamma[2] / gamma[2];
    J0 = gamma[0] + gamma[1] + gamma[2];
    std::cout << "J1  \t" << gamma[0] << "\nJ2  \t" << gamma[1] << "\nJ3  \t" << gamma[2] << std::endl;
    std::cout << "======================================" << std::endl;
    
    int iterations = 0;
    while (true) {
        cv::Point2d center = IncidentVector::getCenter();
        double f = IncidentVector::getF();
        std::vector<double> a = IncidentVector::getA();
        
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
//                left.at<double>(i, j) = J1cc(i, j) / gamma[0] + J2cc(i, j) / gamma[1] + J3cc(i, j) / gamma[2];
                left.at<double>(i, j) = J1cc(i, j) + J2cc(i, j) + J3cc(i, j);
            }
//            right.at<double>(i) = J1c(i) / gamma[0] + J2c(i) / gamma[1] + J3c(i) / gamma[2];
            right.at<double>(i) = J1c(i) + J2c(i) + J3c(i);
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
//            cv::solve(left.mul(cmat), right, delta);
            std::cout << "------------------------ Iteration #: "<< iterations << " -------------------------" << std::endl;
            std::cout << "Delta: " << delta << std::endl;
            
            //    ( 5 ) 次のように˜u0, ˜v0, ˜ f, ˜a1, a2, ... を計算し，それに対するJ の値を˜ J とする．
            //    ˜u0 = u0+Δu0, ˜v = v0+Δv0, ˜ f = f+Δf, ˜a1 = a1+Δa1, ˜a2 = a2+Δa2, ... (48)
            cv::Point2d center_(center.x + delta.at<double>(0), center.y + delta.at<double>(1));
            double f_ = f + delta.at<double>(2);
            std::vector<double> a_;
            for (int i = 0; i < a.size(); ++i) {
                a_.push_back(a[i] + delta.at<double>(i+3));
            }
            
            // Recalculate m and relatives based on new parameters
            IncidentVector::setF(f_);
            IncidentVector::setA(a_);
            IncidentVector::setCenter(center_);
            for (std::vector<Pair>::iterator pair = edges.begin(); pair != edges.end(); ++pair) {
                pair->calcM();
                pair->calcNormal();
                pair->calcLine();
            }
            
            double j1 = J1(), j2 = J2(), j3 = J3();
//            J_ =  j1 / gamma[0] + j2 / gamma[1] + j3 / gamma[2];
            J_ = j1 + j2 + j3;
            std::cout << "C: " << C << "\tJ0: " << J0 << "\tJ_: " << J_;
            std::cout << "\tJ1_: " << j1 << "\tJ2_: " << j2 << "\tJ3_: " << j3 << std::endl;
            
            //    ( 6 ) ˜ J < J0 なら次へ進む．そうでなければC Ã 10C としてステップ(4) に戻る．
            if ( J_  <= J0) {
                this->save(std::string("parameters_") + std::to_string(iterations) + std::string(".xml"));
                
                std::cout << "Center:\t" << center_ << std::endl;
                std::cout << "     f:\t" << f_ << std::endl;
                for (int i = 0; i < a_.size(); ++i) {
                    std::cout << "    a" << i << ":\t" << a_[i] << std::endl;
                }

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
            break;
            
        } else {
            J0 = J_;
            C /= 10.0;
        }
    }
}

double Calibration::J1()
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

double Calibration::J1c(int c)
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

double Calibration::J1cc(int c1, int c2)
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

double Calibration::J2()
{
    double j2 = 0;
    
    
    for (std::vector<Pair>::iterator pair = edges.begin(); pair != edges.end(); ++pair) {
        for (int i = 0; i < 2; ++i) {
            j2 += pair->lineValue[i].at<double>(2);
        }
    }
    
    return j2;
}

double Calibration::J2c(int c)
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

double Calibration::J2cc(int c1, int c2)
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


double Calibration::J3()
{
    double j3 = 0;
    
//    cv::Mat test = cv::Mat::zeros(960, 1280, CV_8UC1);
    
    for (std::vector<Pair>::iterator pair = edges.begin(); pair != edges.end(); ++pair) {
        j3 += pow((pair->lineVector[0].row(2)).dot(pair->lineVector[1].row(2)), 2);
        
//        std::cout << pair->lineVector[0].row(2) << "\t" << pair->lineVector[1].row(2) << "\t" << pow((pair->lineVector[0].row(2).t()).dot(pair->lineVector[1].row(2).t()), 2) << std::endl;
//        cv::line(test, center, center+100*cv::Point2d(pair->lineVector[0].row(2).at<double>(0),pair->lineVector[0].row(2).at<double>(1)), 255);
//        cv::line(test, center, center+100*cv::Point2d(pair->lineVector[1].row(2).at<double>(0),pair->lineVector[0].row(2).at<double>(1)), 255);
//        std::cout << pair->lineVector[0].row(2) * 10 << "\t" << pair->lineVector[1].row(2) * 10 << std::endl;
//        cv::imshow("test", test);
//        cv::waitKey();
    }
    
    return j3;
}

double Calibration::J3c(int c)
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

double Calibration::J3cc(int c1, int c2)
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
