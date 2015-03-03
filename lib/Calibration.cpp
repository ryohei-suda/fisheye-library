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
    
    fs_out << "J1" <<  J1();
    fs_out << "J2" << J2();
    fs_out << "J3" << J3();
    
    
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
    }
    
//    int count = 0;
//    for (auto &pair : edges) { // Write to point cloud data file
//        std::vector<cv::Point3d> points;
//        for (int j = 0; j < 2; ++j) {
//            for (auto n : pair.normalVector[j]) {
//                cv::Point3d n_(n.row(2));
//                for (double i = 0.0; i <= 1; i+=0.01) {
//                    points.push_back(n_*i);
//                }
//            }
//            
//            cv::Mat l = pair.lineVector[j].row(2);
//            for (double i = 0.0; i <= 1; i+=0.01){
//                points.push_back(cv::Point3d(l)*i);
//            }
//            
//            for (auto &line : pair.edge[j]) {
//                for (auto &point : line) {
//                    cv::Point3d m  = point->m;
//                    points.push_back(m);
//                }
//            }
//        }
//        
//        
//        std::ofstream ofs(std::to_string(count) + ".pcd");
//        ofs << "VERSION .7" << std::endl;
//        ofs << "FIELDS x y z" << std::endl;
//        ofs << "SIZE 4 4 4" << std::endl;
//        ofs << "TYPE F F F" << std::endl;
//        ofs << "COUNT 1 1 1" << std::endl;
//        ofs << "WIDTH " << points.size() << std::endl;
//        ofs << "HEIGHT 1" << std::endl;
//        ofs << "VIEWPOINT 0 0 0 1 0 0 0" << std::endl;
//        ofs << "POINTS " << points.size() << std::endl;
//        ofs << "DATA ascii" << std::endl;
//        
//        for (auto &p : points) {
//            ofs << p.x << " " << p.y << " " << p.z << std::endl;
//        }
//        
//        ofs.close();
//        
//        ++count;
//    }
//    exit(1);

    
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
        int local_iterations = 0, max_local_iterations = 100;
        while (true) {
            ++local_iterations;
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
                if (C == INFINITY) {
                    C = 0.0001;
                }
                if (local_iterations > max_local_iterations) {
                    break;
                }
            }
        }
        
        // Judge wether converged
        bool converged = true;
        double epsilon = 1.0e-5;
        if (fabs(delta.at<double>(0)) > epsilon ||
            fabs(delta.at<double>(1)) > epsilon ||
            fabs(delta.at<double>(2)) > epsilon) {
            converged = false;
        }
        for (int i = 3; i < IncidentVector::nparam && converged; ++i) {
            if (fabs(delta.at<double>(i)) /  a.at(i-3) > epsilon) {
                converged = false;
                break;
            }
        }
        
        if (converged && (local_iterations <= max_local_iterations)) {
            std::cout << "converged" << std::endl;
            break;
            
        } else {
            J0 = J_;
            C /= 10.0;
            if (C == 0.) {
                C = 0.0001;
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

double Calibration::F()
{
    double f = 0;
    
    for (auto &pair : edges) {
            f += pair.calcF();
    }
    
    return f;
}

double Calibration::Fc(int c)
{
    double fc = 0;
    
    for (auto &pair : edges) {
        for (int i = 0; i < 2; ++i) {
            std::vector<cv::Mat> w = pair.w[i];
            std::vector<Pair::C> mc = pair.Mc[i];
            for (int j = 0; j < w.size(); ++j) {
                fc += (w[j].t() * mc[j].at(c)).dot(w[j].t());
            }
        }
    }
    
    return fc;
}

double Calibration::Fcc(int c1, int c2)
{
    double fcc = 0;
    
    for (auto &pair : edges) {
        for (int i = 0; i < 2; ++i) {
            std::vector<cv::Mat> w = pair.w[i];
            std::vector<Pair::Cc> mcc = pair.Mcc[i];
            for (int j = 0; j < w.size(); ++j) {
                fcc += (w[j].t() * mcc[j].at(c1,c2)).dot(w[j].t());
            }
        }
    }
    
    return fcc;
}

void Calibration::calibrateNew()
{
    const auto start_time = std::chrono::system_clock::now();
    double F0;
    double C = 0.0001;
    
    for (auto &pair : edges) {
        pair.calcM();
        pair.calcNormal();
        pair.calcLine();
    }
    
//    int count = 0;
//    for (auto &pair : edges) { // Write to point cloud data file
//        std::vector<cv::Point3d> points;
//        
//        for (int j = 0; j < 1; ++j) {
//            std::vector<cv::Mat> ns;
//            for (auto &n : pair.normalVector[1]) {
//                ns.push_back(n.row(2).t());
//            }
//            cv::Mat h = pair.lineVector[0].row(2).t();
//            cv::Mat v_ = pair.lineVector[1].row(2).t();
//            cv::Mat v = pair.calcVertical(h, ns);
//            std::cout << acos(v.dot(v_))*180/M_PI << std::endl;
//            for (double i = 0.0; i <= 1; i+=0.01){
//                points.push_back(cv::Point3d(v)*i);
//            }
//            for (double i = 0.0; i <= 1; i+=0.01){
//                points.push_back(cv::Point3d(h)*i);
//            }
//            
//            for (auto &line : pair.edge[j]) {
//                for (auto &point : line) {
//                    cv::Point3d m  = point->m;
//                    points.push_back(m);
//                }
//            }
//        }
//        
//        std::ofstream ofs(std::to_string(count) + ".pcd");
//        ofs << "VERSION .7" << std::endl;
//        ofs << "FIELDS x y z" << std::endl;
//        ofs << "SIZE 4 4 4" << std::endl;
//        ofs << "TYPE F F F" << std::endl;
//        ofs << "COUNT 1 1 1" << std::endl;
//        ofs << "WIDTH " << points.size() << std::endl;
//        ofs << "HEIGHT 1" << std::endl;
//        ofs << "VIEWPOINT 0 0 0 1 0 0 0" << std::endl;
//        ofs << "POINTS " << points.size() << std::endl;
//        ofs << "DATA ascii" << std::endl;
//        
//        for (auto &p : points) {
//            ofs << p.x << " " << p.y << " " << p.z << std::endl;
//        }
//        
//        ofs.close();
//        
//        ++count;
//        
//    }
    
     F0 = F();
    
    std::cout << "F\t" << F0 << std::endl;
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
            pair.calcMd();
            pair.calcMc();
            pair.calcMcc();
        }
        F();
        
        // Calculate 1st and 2nd derivatives of J
        cv::Mat left(IncidentVector::nparam, IncidentVector::nparam, CV_64F);
        cv::Mat right(IncidentVector::nparam, 1, CV_64F);
        
        for (int i = 0; i < IncidentVector::nparam; ++i) {
            for (int j = 0; j < IncidentVector::nparam; ++j) {
                // (1+C) isn't calculated here, look at the next while loop
                left.at<double>(i, j) = Fcc(i, j);
            }
            right.at<double>(i) = Fc(i);
        }
        
        cv::Mat delta;
        double F_;
        int local_iterations = 0, max_local_iterations = 100;
        while (true) {
            ++local_iterations;
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
//                pair.calcMd();
//                pair.calcMc();
//                pair.calcMcc();
            }
            
            F_ =  F();
            std::cout << "C: " << C << "\tF0: " << F0 << "\tF_: " << F_ << std::endl;
            std::cout << "J1: " << J1() << "\tJ2: " << J2() << "\tJ3: " << J3() << std::endl;
            
            if ( F_  <= F0) {
                std::cout << "Center:\t" << center_ << std::endl;
                std::cout << "     f:\t" << f_ << std::endl;
                for (int i = 0; i < a_.size(); ++i) {
                    std::cout << "    a" << i << ":\t" << a_[i] << std::endl;
                }
                
                break;
            } else {
                C *= 10;
                if (C == INFINITY) {
                    C = 0.0001;
                }
                if (local_iterations > max_local_iterations) {
                    break;
                }
            }
        }
        
        // Judge wether converged
        bool converged = true;
        double epsilon = 1.0e-5;
        if (fabs(delta.at<double>(0)) > epsilon ||
            fabs(delta.at<double>(1)) > epsilon ||
            fabs(delta.at<double>(2)) > epsilon) {
            converged = false;
        }
        for (int i = 3; i < IncidentVector::nparam && converged; ++i) {
            if (fabs(delta.at<double>(i)) /  a.at(i-3) > epsilon) {
                converged = false;
                break;
            }
        }
        
        if (converged && (local_iterations <= max_local_iterations)) {
            std::cout << "converged" << std::endl;
            break;
            
        } else {
            F0 = F_;
            C /= 10.0;
            if (C == 0.) {
                C = 0.0001;
            }
        }
    }
    
    const auto duration = std::chrono::system_clock::now() - start_time;
    int minutes = (int)std::chrono::duration_cast<std::chrono::minutes>(duration).count();
    int seconds = (int)std::chrono::duration_cast<std::chrono::seconds>(duration).count() - minutes*60;
    std::cout << "Calibration has been finished in " << minutes << " minutes " << seconds << " seconds" << std::endl;
}