//
//  main.cpp
//  Evaluation
//
//  Created by Ryohei Suda on 2015/01/11.
//  Copyright (c) 2015年 RyoheiSuda. All rights reserved.
//

#include <iostream>
#include "Evaluation.h"

int main(int argc, const char * argv[]) {
    Evaluation e;
    
    std::string fname;
    std::cout << "Type data file name > ";
    std::cin >> fname;
    e.loadData(fname);
    
    e.projectAllPoints();
    
    return 0;
}
