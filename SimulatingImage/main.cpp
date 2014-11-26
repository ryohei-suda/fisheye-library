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
    si.setPitchRadian(0, 0, 0);
    si.setOpticalCenter(980, 640);
    si.setFocalLength(400);
    si.setF0(400);
    si.setImgSize(1960, 1280);
    std::vector<double> a(0);
    si.setA(a);
    
    si.display();
    
    return 0;
}
