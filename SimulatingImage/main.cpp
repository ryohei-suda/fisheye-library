//
//  main.cpp
//  SimulatingImage
//
//  Created by Ryohei Suda on 2014/11/25.
//  Copyright (c) 2014年 RyoheiSuda. All rights reserved.
//

#include <iostream>
#include "SimulatingImage.h"

int main(int argc, const char * argv[]) {
    
    SimulatingImage si;
    
    si.setPatternSize(50000, 40000);
    si.setPatternCenter(0, 0, 11000);
    si.setInterval(1000);
    si.setPitchRadian(0, 0, 0);
    si.setImgSize(1600, 1200);
    si.setOpticalCenter(793, 606);
    si.setFocalLength(450);
    si.setF0(450);
    si.setFoVDegree(200);
    si.setModel(1);
    std::vector<double> a(0);
//    a.push_back(5e-3);
//    a.push_back(6e-4);
//    a.push_back(7e-5);
//    a.push_back(8e-6);
//    a.push_back(9e-7);
    si.setA(a);
    
    si.display();
    
    return 0;
}
