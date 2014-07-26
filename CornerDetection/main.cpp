//
//  main.cpp
//  CornerDetection
//
//  Created by Ryohei Suda on 2014/03/23.
//  Copyright (c) 2014年 Ryohei Suda. All rights reserved.
//

#include <iostream>
#include <vector>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "CornerDetection.h"

int main(int argc, const char * argv[])
{
    
    std::cout << "Type list file name of calibration imgages > ";
    std::string fname;
    std::cin >> fname;
    CornerDetection cd;
    cd.loadImageXML(fname);
    
    cd.processAllImages();
    
    std::cout << "Type output XML file name > ";
    std::string output;
    std::cin >> output;
    cd.writeEdges(output);

    return 0;
}
