//
//  main.cpp
//  CalibrationPattern
//
//  Created by Suda on 2014/03/23.
//  Copyright (c) 2014年 Ryohei Suda. All rights reserved.
//

#include <iostream>
#include "Pattern.h"


int main(int argc, const char * argv[])
{
    std::cout << "This shows zonation patterns to calibrate a camera\n" << std::endl;
    
    int height, width;
    
    std::cout << "Type display height in pixel > ";
    std::cin >> height;
    std::cout << "Type display width in pixel > ";
    std::cin >> width;
    
    Pattern pattern(height, width, 75, 5);
    pattern.generate();
    
    std::string dir;
    std::cout << "Type directory name to store images > ";
    std::cin >> dir;
    pattern.save(dir);
    
    return 0;
}

