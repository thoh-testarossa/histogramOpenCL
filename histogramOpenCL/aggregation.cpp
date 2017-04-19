#include "data.h"
#include "aggregation.h"

#include <cstdlib>

void initCell_cAgg(cAgg *c_agg)
{
    c_agg->histogramType = HIS_TYPE_EWH;
    c_agg->histogramIntervalNum = HIS_INTERVAL_NUM;
    //c_agg->histogramIntervalNum = intervalNum;
    //c_agg->histogramIntervalMark = new int [intervalNum + 1];
    //c_agg->histogramIntervalCount = new int [intervalNum];
}

int decideDimensionToAggregate(int cubeDim[3])
{
    int dimensionToAggregate = X_AGGREGATE;
    int maxDimNum = cubeDim[0];
    if(cubeDim[1] > maxDimNum) { maxDimNum = cubeDim[1]; dimensionToAggregate = Y_AGGREGATE; }
    if(cubeDim[2] > maxDimNum) { maxDimNum = cubeDim[2]; dimensionToAggregate = Z_AGGREGATE; }
    return dimensionToAggregate;
}
