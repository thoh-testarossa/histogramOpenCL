#include "data.h"
#include "aggregation.h"

#include <cstdlib>

void initCell_cAgg(cAgg *c_agg, int intervalNum)
{
    c_agg->histogramIntervalNum = intervalNum;
    c_agg->histogramIntervalMark = new int [intervalNum + 1];
    c_agg->histogramIntervalCount = new int [intervalNum];
}
