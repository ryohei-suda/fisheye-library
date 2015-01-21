//
//  Evaluation.cpp
//  Evaluation
//
//  Created by Ryohei Suda on 2015/01/11.
//  Copyright (c) 2015年 RyoheiSuda. All rights reserved.
//

#include "Evaluation.h"

void Evaluation::loadData(std::string filename) {
    pairs.clear();
    
    tinyxml2::XMLDocument doc;
    if (tinyxml2::XML_NO_ERROR != doc.LoadFile(filename.c_str())) {
        std::cerr << "Cannot open " << filename << std::endl;
    }
    
    // Load parameters
    tinyxml2::XMLElement *root = doc.FirstChildElement("data");
    tinyxml2::XMLElement *params = root->FirstChildElement("parameters");
    
    double f = atof(params->FirstChildElement("f")->GetText());
    IncidentVector::setF(f);
    double f0 = atof(params->FirstChildElement("f0")->GetText());
    IncidentVector::setF0(f0);
    
    cv::Size2i img_size;
    std::stringstream ss(params->FirstChildElement("img_size")->GetText());
    std::string elem;
    ss >> elem;
    img_size.width = atoi(elem.c_str());
    ss >> elem;
    img_size.height = atoi(elem.c_str());
    IncidentVector::setImgSize(img_size);
    
    cv::Point2d center;
    std::stringstream ss1(params->FirstChildElement("center")->GetText());
    ss1 >> elem;
    center.x = atof(elem.c_str());
    ss1 >> elem;
    center.y = atof(elem.c_str());
    IncidentVector::setCenter(center);
    
    std::vector<double> a;
    std::stringstream ss2(params->FirstChildElement("a")->GetText());
    while (ss2 >> elem) {
        a.push_back(atof(elem.c_str()));
    }
    IncidentVector::setA(a);
    
    std::string projection = params->FirstChildElement("projection")->GetText();
    IncidentVector::setProjection(projection);
    
    // Load lines
    std::stringstream ssdata;
    tinyxml2::XMLElement *pair = root->FirstChildElement("pairs")->FirstChildElement("pair");
    
    while (pair != NULL) {
        Pair tmp;
        
        // edge1
        tinyxml2::XMLElement *lines1 = pair->FirstChildElement("lines1");
        tinyxml2::XMLElement *line = lines1->FirstChildElement("line");
        while (line != NULL) {
            std::vector<IncidentVector *> points; // One line
            tinyxml2::XMLElement *p = line->FirstChildElement("p");
            while (p != NULL) {
                cv::Point2d point;
                ssdata.str(p->GetText());
                ssdata >> point.x;
                ssdata >> point.y;
                switch (IncidentVector::getProjection()) {
                    case 0:
                        points.push_back(new StereographicProjection(point));
                        break;
                    case 1:
                        //TODO add Perspective projection
                        break;
                    case 2:
                        points.push_back(new EquidistanceProjection(point));
                        break;
                    case 3:
                        points.push_back(new EquisolidAngleProjection(point));
                        break;
                }
                ssdata.clear();
                p = p->NextSiblingElement("p");
            }
            tmp.edge[0].push_back(points);
            line = line->NextSiblingElement("line");
        }
        
        // edge2
        tinyxml2::XMLElement *lines2 = pair->FirstChildElement("lines2");
        line = lines2->FirstChildElement("line");
        while (line != NULL) {
            std::vector<IncidentVector *> points; // One line
            tinyxml2::XMLElement *p = line->FirstChildElement("p");
            while (p != NULL) {
                cv::Point2d point;
                ssdata.str(p->GetText());
                ssdata >> point.x;
                ssdata >> point.y;
                switch (IncidentVector::getProjection()) {
                    case 0:
                        points.push_back(new StereographicProjection(point));
                        break;
                    case 1:
                        //TODO add Perspective projection
                        break;
                    case 2:
                        points.push_back(new EquidistanceProjection(point));
                        break;
                    case 3:
                        points.push_back(new EquisolidAngleProjection(point));
                        break;
                }
                ssdata.clear();
                p = p->NextSiblingElement("p");
            }
            tmp.edge[1].push_back(points);
            line = line->NextSiblingElement("line");
        }
        
        pairs.push_back(tmp);
        pair = pair->NextSiblingElement("pair");
    }
    
    doc.Clear();
}

void Evaluation::projectAllPoints()
{
    double errors[1000] = {0};
    int count[1000] = {0};
    
    for (auto &pair : pairs) {
        pair.calcM();
        pair.calcNormal();
        for (int i = 0; i < 2; ++i) {
            for (int j = 0; j < pair.normalVector[i].size(); ++j) {
                cv::Point3d norm(pair.normalVector[i].at(j).row(2));
                for (auto &p : pair.edge[i][j]) {
                    double error = fabs(M_PI_2 - acos(norm.ddot(p->m)));
                    double degree = p->getTheta() * 180 / M_PI;
//                    std::cout << error << " " << error*180/M_PI << " " << p->theta << " " << degree <<  std::endl;
                    errors[(int)(degree*10)] += error*180/M_PI;
                    ++count[(int)(degree*10)];
                }
            }
        }
    }
    for (int i = 0; i < 1000; ++i) {
        if (count[i] != 0) {
            std::cout << (double)i/10 << " " << errors[i]/count[i] << std::endl;
        }
    }
}