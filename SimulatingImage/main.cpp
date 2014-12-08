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
    
    si.setPatternSize(1200, 900);
    si.setPatternCenter(0, 0, 200);
    si.setInterval(50);
    si.setPitchRadian(0, 0, 0);
    si.setImgSize(1900, 1200);
    si.setOpticalCenter(954, 597);
    si.setFocalLength(400);
    si.setF0(400);
    std::vector<double> a;
    a.push_back(0.0001);
    a.push_back(0.00002);
    a.push_back(0.000003);
    a.push_back(0.0000004);
    a.push_back(0.00000005);
    si.setA(a);
    
    si.display();
    
    return 0;
}
