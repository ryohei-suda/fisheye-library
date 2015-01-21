//
//  Evaluation.h
//  Evaluation
//
//  Created by Ryohei Suda on 2015/01/11.
//  Copyright (c) 2015年 RyoheiSuda. All rights reserved.
//

#ifndef __Evaluation__Evaluation__
#define __Evaluation__Evaluation__

#include <vector>
#include "../Calibration/Pair.h"
#include "../Calibration/StereographicProjection.h"
#include "../Calibration/EquidistanceProjection.h"
#include "../Calibration/EquisolidAngleProjection.h"
#include "../libs/tinyxml2.h"

class Evaluation {
private:
    std::vector<Pair> pairs;
    
public:
    void loadData(std::string filename);
    void projectAllPoints();
};
    
#endif /* defined(__Evaluation__Evaluation__) */
