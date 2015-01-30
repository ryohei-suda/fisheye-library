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
    if (tinyxml2::XML_NO_ERROR != doc.LoadFile(filename.c_str())) {
        std::cerr << "Cannot open " << filename << std::endl;
    }
    tinyxml2::XMLElement *root = doc.FirstChildElement("lines");
    
    double unit = atof(root->FirstChildElement("pixel_size")->GetText());
    double f = atof(root->FirstChildElement("focal_length")->GetText()) / unit;
    IncidentVector::setF(f);
    IncidentVector::setF0((int)f);
    
    cv::Size2i img_size;
    cv::Point2d center;
    img_size.width = atoi(root->FirstChildElement("width")->GetText());
    center.x = img_size.width / 2.0;
    img_size.height = atoi(root->FirstChildElement("height")->GetText());
    center.y = img_size.height / 2.0;
    IncidentVector::setImgSize(img_size);
    IncidentVector::setCenter(center);
    
    std::string projection = root->FirstChildElement("projection")->GetText();
    IncidentVector::setProjection(projection);
    
    std::stringstream ssdata;
    tinyxml2::XMLElement *pair = root->FirstChildElement("pair");
    std::cout << projection << "\t"  << IncidentVector::getProjection() << std::endl;
    while (pair != NULL) {
        Pair tmp;
        
        // edge1
        tinyxml2::XMLElement *edge1 = pair->FirstChildElement("lines1");
        tinyxml2::XMLElement *line = edge1->FirstChildElement("line");
        while (line != NULL) {
            std::vector<IncidentVector *> edge; // One line of points
            tinyxml2::XMLElement *p = line->FirstChildElement("p");
            while (p != NULL) {
                cv::Point2d point;
                ssdata.str(p->GetText());
                ssdata >> point.x;
                ssdata >> point.y;
                switch (IncidentVector::getProjection()) {
                    case 0:
                        edge.push_back(new StereographicProjection(point));
                        break;
                    case 1:
                        edge.push_back(new OrthographicProjection(point));
                        break;
                    case 2:
                        edge.push_back(new EquidistanceProjection(point));
                        break;
                    case 3:
                        edge.push_back(new EquisolidAngleProjection(point));
                        break;
                }
                ssdata.clear();
                p = p->NextSiblingElement("p");
            }
            tmp.edge[0].push_back(edge);
            line = line->NextSiblingElement("line");
        }
        
        // edge2
        tinyxml2::XMLElement *edge2 = pair->FirstChildElement("lines2");
        line = edge2->FirstChildElement("line");
        while (line != NULL) {
            std::vector<IncidentVector *> edge; // One line of points
            tinyxml2::XMLElement *p = line->FirstChildElement("p");
            while (p != NULL) {
                cv::Point2d point;
                ssdata.str(p->GetText());
                ssdata >> point.x;
                ssdata >> point.y;
                switch (IncidentVector::getProjection()) {
                    case 0:
                        edge.push_back(new StereographicProjection(point));
                        break;
                    case 1:
                        edge.push_back(new OrthographicProjection(point));
                        break;
                    case 2:
                        edge.push_back(new EquidistanceProjection(point));
                        break;
                    case 3:
                        edge.push_back(new EquisolidAngleProjection(point));
                        break;
                }
                ssdata.clear();
                p = p->NextSiblingElement("p");
            }
            tmp.edge[1].push_back(edge);
            line = line->NextSiblingElement("line");
        }
        
        edges.push_back(tmp);
        pair = pair->NextSiblingElement("pair");
    }
    
    doc.Clear();
}

void Calibration::save(std::string filename)
{
    cv::FileStorage fs_out(filename, cv::FileStorage::WRITE);
    fs_out << "projection" << IncidentVector::getProjectionName();
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

void Calibration::calibrate(bool divide)
{
    const auto start_time = std::chrono::system_clock::now();
    double J0;
    double C = 0.0001;
    
    for (auto &pair : edges) {
        pair.calcM();
        pair.calcNormal();
        pair.calcLine();
        pair.calcVertical(pair.lineVector[0],pair.normalVector[1]);
    }
    std::cin >> J0;
    
    double j1 = J1(), j2 = J2(), j3 = J3();
    double gamma[3];
    C = 0.0001;

    if (divide) {
        gamma[0] = j1; gamma[1] = j2; gamma[2] = j3;
    } else {
//        gamma[0]= 0;
//        for (auto &pair : edges) {
//            gamma[0] += pair.edge[0].size() + pair.edge[1].size();
//        }
//        gamma[1] = edges.size();
//        gamma[2] = gamma[1]/2;
        
        gamma[0] = gamma[1] = gamma[2] = 1;
    }
    J0 = j1 / gamma[0] + j2 / gamma[1] + j3 / gamma[2];
    std::cout << "J1  \t" << j1/gamma[0] << "\nJ2  \t" << j2/gamma[1] << "\nJ3  \t" << j3/gamma[2] << std::endl;
    std::cout << "J1  \t" << j1 << "\nJ2  \t" << j2 << "\nJ3  \t" << j3 << std::endl;
    std::cout << "======================================" << std::endl;
    
    int iterations = 0;
    cv::Mat delta_prev= cv::Mat::ones(IncidentVector::nparam, 1, CV_64F);
    while (true) {
        cv::Point2d center = IncidentVector::getCenter();
        double f = IncidentVector::getF();
        std::vector<double> a = IncidentVector::getA();
        
        // Calculate all derivatives
        for (auto &pair : edges) {
            pair.calcM();
            pair.calcNormal();
            pair.calcLine();
            pair.calcDerivatives();
        }
        
        // Calculate 1st and 2nd derivatives of J
        cv::Mat left(IncidentVector::nparam, IncidentVector::nparam, CV_64F);
        cv::Mat right(IncidentVector::nparam, 1, CV_64F);
        
        for (int i = 0; i < IncidentVector::nparam; ++i) {
            for (int j = 0; j < IncidentVector::nparam; ++j) {
                // (1+C) isn't calculated here, look at the next while loop
                left.at<double>(i, j) = J1cc(i, j) / gamma[0] + J2cc(i, j) / gamma[1] + J3cc(i, j) / gamma[2];
            }
                right.at<double>(i) = J1c(i) / gamma[0] + J2c(i) / gamma[1] + J3c(i) / gamma[2];
        }
        
        cv::Mat delta;
        double J_;
        while (true) {
            ++iterations;
            std::cout << "------------------------ Iteration "<< iterations << " -------------------------" << std::endl;
//            std::cout << left << right << std::endl;
            cv::Mat cmat = cv::Mat::ones(IncidentVector::nparam, IncidentVector::nparam, CV_64F); // To calculate (1+C)
            for (int i = 0; i < IncidentVector::nparam; ++i) {
                cmat.at<double>(i,i) = 1+C;
            }
            // Calculate delta by solving the simultaneous equation
            cv::solve(left.mul(cmat), -right, delta);
            std::cout << "Delta: " << delta << std::endl;
            
            // Update parameters by adding delta and calculate coressponding J
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
            for (auto &pair : edges) {
                pair.calcM();
                pair.calcNormal();
                pair.calcLine();
            }
            
            j1 = J1();
            j2 = J2();
            j3 = J3();
            J_ =  j1 / gamma[0] + j2 / gamma[1] + j3 / gamma[2];
            std::cout << "C: " << C << "\tJ0: " << J0 << "\tJ_: " << J_;
            std::cout.precision(10);
            std::cout.width(10);
            std::cout << "\tJ1_: " << j1/gamma[0] << "\tJ2_: " << j2/gamma[1] << "\tJ3_: " << j3/gamma[2] << std::endl;
            std::cout << "J1_: " << j1 << "\tJ2_: " << j2 << "\tJ3_: " << j3 << std::endl;
            
            if ( J_  <= J0) {
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
        
        // Judge wether converged
        bool converged = true;
        double epsilon = 1.0e-5;
        if (fabs(delta.at<double>(0) / center.x) > epsilon ||
            fabs(delta.at<double>(1) / center.y) > epsilon ||
            fabs(delta.at<double>(2) / f) > epsilon) {
            converged = false;
        }
        for (int i = 3; i < IncidentVector::nparam && converged; ++i) {
            if (fabs(delta.at<double>(i)) /  a.at(i-3) > epsilon) {
                converged = false;
                break;
            }
        }
        
        if (converged) {
            std::cout << "converged" << std::endl;
            break;
            
        } else {
            J0 = J_;
            C /= 10.0;
        }
    }
    
    const auto duration = std::chrono::system_clock::now() - start_time;
    int minutes = (int)std::chrono::duration_cast<std::chrono::minutes>(duration).count();
    int seconds = (int)std::chrono::duration_cast<std::chrono::seconds>(duration).count() - minutes*60;
    std::cout << "Calibration has been finished in " << minutes << " minutes " << seconds << " seconds" << std::endl;
}

void Calibration::calibrate2()
{
    const auto start_time = std::chrono::system_clock::now();
    double J0;
    double C = 0.0001;
    
    double (Calibration::*J[3])() = {&Calibration::J1, &Calibration::J2,&Calibration::J3};
    double (Calibration::*Jc[3])(int) = {&Calibration::J1c, &Calibration::J2c,&Calibration::J3c};
    double (Calibration::*Jcc[3])(int, int) = {&Calibration::J1cc, &Calibration::J2cc,&Calibration::J3cc};
    while(true){
    for (int t = 2; t >= 0; --t) { // For each of Orthongonality, Parallelism, and Colinearity
        
        for (auto &pair : edges) {
            pair.calcM();
            pair.calcNormal();
            pair.calcLine();
        }
        C = 0.0001;
        J0 = (this->*J[t])();
        switch (t) {
            case 0:
                std::cout << "Colinearity ";
                break;
            case 1:
                std::cout << "Parallelism ";
                break;
            case 2:
                std::cout << "Othogonality ";
                break;
        }
        std::cout << "J  \t" << J0 << std::endl;
        
        std::cout << "======================================" << std::endl;
        
        int iterations = 0;
        cv::Mat delta_prev= cv::Mat::ones(IncidentVector::nparam, 1, CV_64F);
        while (true) {
            ++iterations;
            cv::Point2d center = IncidentVector::getCenter();
            double f = IncidentVector::getF();
            std::vector<double> a = IncidentVector::getA();
            
            for (auto &pair : edges) {
                pair.calcM();
                pair.calcNormal();
                pair.calcLine();
                pair.calcDerivatives();
            }
            
            cv::Mat left(IncidentVector::nparam, IncidentVector::nparam, CV_64F);
            cv::Mat right(IncidentVector::nparam, 1, CV_64F);
            
            for (int i = 0; i < IncidentVector::nparam; ++i) {
                for (int j = 0; j < IncidentVector::nparam; ++j) {
                    // (1+C) isn't calculated here, look at the next while loop
                    left.at<double>(i, j) = (this->*Jcc[t])(i, j);
                }
                right.at<double>(i) = (this->*Jc[t])(i);
            }
            
            cv::Mat delta;
            double J_;
            while (true) {
                cv::Mat cmat = cv::Mat::ones(IncidentVector::nparam, IncidentVector::nparam, CV_64F); // To calculate (1+C)
                for (int i = 0; i < IncidentVector::nparam; ++i) {
                    cmat.at<double>(i,i) = 1+C;
                }
                cv::solve(left.mul(cmat), -right, delta);
//                std::cout << "------------------------ Iteration "<< iterations << " -------------------------" << std::endl;
//                std::cout << "Delta: " << delta << std::endl;
                
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
                for (auto &pair : edges) {
                    pair.calcM();
                    pair.calcNormal();
                    pair.calcLine();
                }
                
                J_ = (this->*J[t])();
//                std::cout << "C: " << C << "\tJ0: " << J0 << "\tJ_: " << J_ << std::endl;
                
                if ( J_  <= J0) {
                    std::cout << "------------------------ Iteration "<< iterations << " -------------------------" << std::endl;
                    std::cout << "Delta: " << delta << std::endl;
                    std::cout << "C: " << C << "\tJ0: " << J0 << "\tJ_: " << J_ << std::endl;
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
            
            // Judge wether converged
            bool converged = true;
            double epsilon = 1.0e-5;
            if (fabs(delta.at<double>(0) / center.x) > epsilon ||
                fabs(delta.at<double>(1) / center.y) > epsilon ||
                fabs(delta.at<double>(2) / f) > epsilon) {
                converged = false;
            }
            for (int i = 3; i < IncidentVector::nparam && converged; ++i) {
                if (fabs(delta.at<double>(i)) /  a.at(i-3) > epsilon) {
                    converged = false;
                    break;
                }
            }
            
            if (converged) {
                break;
                
            } else {
                J0 = J_;
                C /= 10.0;
            }
        }
        if (t == 0) {
            t = 3;
        }
    }
    }
    const auto duration = std::chrono::system_clock::now() - start_time;
    int minutes = (int)std::chrono::duration_cast<std::chrono::minutes>(duration).count();
    int seconds = (int)std::chrono::duration_cast<std::chrono::seconds>(duration).count() - minutes*60;
    std::cout << "Calibration has been finished in " << minutes << " minutes " << seconds << " seconds" << std::endl;
}


double Calibration::J1()
{
    double j1 = 0;
    
    for (auto &pair : edges) {
        for (int i = 0; i < 2; ++i) {
            for (auto &nval : pair.normalValue[i]) {
                j1 += nval.at<double>(2);
            }
        }
    }
    
    return j1;
}

double Calibration::J1c(int c)
{
    double j1c = 0;
    
    for (auto &pair : edges) {
        for (int i = 0; i < 2; ++i) {
            std::vector<cv::Mat>::iterator nvec = pair.normalVector[i].begin();
            std::vector<Pair::C>::iterator mc = pair.Mc[i].begin();
            for (; mc != pair.Mc[i].end() && nvec != pair.normalValue[i].end(); ++mc, ++nvec) { // For each line
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
    
    for (auto &pair : edges) {
        for (int i = 0; i < 2; ++i) {
            std::vector<cv::Mat>::iterator nvec = pair.normalVector[i].begin();
            std::vector<cv::Mat>::iterator nval = pair.normalValue[i].begin();
            std::vector<Pair::C>::iterator mc = pair.Mc[i].begin();
            std::vector<Pair::Cc>::iterator mcc = pair.Mcc[i].begin();
            
            for (;mc != pair.Mc[i].end() && mcc != pair.Mcc[i].end() && nvec != pair.normalVector[i].end() && nval != pair.normalValue[i].end(); ++mc, ++mcc, ++nvec, ++nval) { // For each line
                
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
    
    for (auto &pair : edges) {
        for (int i = 0; i < 2; ++i) {
            j2 += pair.lineValue[i].at<double>(2);
        }
    }
    
    return j2;
}

double Calibration::J2c(int c)
{
    double j2c = 0;
    
    for (auto &pair : edges) {
        for (int i = 0; i < 2; ++i) {
            cv::Mat lg = pair.lineVector[i].row(2).t();
            j2c += lg.dot(pair.Nc[i].at(c) * lg);
        }
    }
    
    return j2c;
}

double Calibration::J2cc(int c1, int c2)
{
    double j2cc = 0;
    
    for (auto &pair : edges) {
        for (int i = 0; i < 2; ++i) {
            cv::Mat lg = pair.lineVector[i].row(2).t();
            j2cc += lg.dot(pair.Ncc[i].at(c1, c2) * lg);
            
            cv::Mat lgi[2] = {pair.lineVector[i].row(0).t(), pair.lineVector[i].row(1).t()};
            for (int j = 0; j < 2; ++j) {
                j2cc -= (lgi[j].dot(pair.Nc[i].at(c1) * lg) * lgi[j].dot(pair.Nc[i].at(c2) * lg)) / (pair.lineValue[i].at<double>(j) - pair.lineValue[i].at<double>(2));
            }
        }
    }
    j2cc *= 2;
    
    return j2cc;
}


double Calibration::J3()
{
    double j3 = 0;
    
    for (auto &pair : edges) {
        j3 += pow((pair.lineVector[0].row(2)).dot(pair.lineVector[1].row(2)), 2);
    }
    
    return j3;
}

double Calibration::J3c(int c)
{
    double j3c = 0;
    
    for (auto &pair : edges) {
        cv::Mat lg1 = pair.lineVector[0].row(2).t();
        cv::Mat lg2 = pair.lineVector[1].row(2).t();
        j3c += (lg1.dot(lg2)) * ((pair.lc[0].at(c)).dot(lg2))+(lg1).dot(pair.lc[1].at(c));
    }
    
    j3c *= 2;

    return j3c;
}

double Calibration::J3cc(int c1, int c2)
{
    double j3cc = 0;
    
    for (auto &pair : edges) {
        
        cv::Mat lg1 = pair.lineVector[0].row(2).t();
        cv::Mat lg2 = pair.lineVector[1].row(2).t();
        
        double tmp1 = (pair.lc[0].at(c1)).dot(lg2) + (lg1.dot(pair.lc[1].at(c1)));
        double tmp2 = (pair.lc[0].at(c2)).dot(lg2) + (lg1.dot(pair.lc[1].at(c2)));
        
        j3cc += tmp1 * tmp2;
    }
    
    j3cc *= 2;

    return j3cc;
}
