//
//  main.cpp
//  LineDetection
//
//  Created by Ryohei Suda on 2014/03/23.
//  Copyright (c) 2014年 Ryohei Suda. All rights reserved.
//

#include <iostream>
#include <vector>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "LineDetection.h"

int main(int argc, const char * argv[])
{
    
    std::cout << "Type list file name of calibration imgages > ";
    std::string fname;
    std::cin >> fname;
    LineDetection ld;
//    ld.editAllEdges(ld.loadEdgeXML(fname));

//    std::vector<std::vector<std::vector<cv::Point2i> > > edges = ld.loadEdgeXML(fname);
//    ld.saveParameters();
//    ld.editAllEdges(edges);
    ld.loadImageXML(fname);
    ld.saveParameters();
    
    ld.processAllImages();
//
    std::cout << "Type output XML file name > ";
    std::string output;
    std::cin >> output;
    ld.writeXML(output);
    
    
    return 0;
}
