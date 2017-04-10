#include "data.h"
#include "aggregation.h"

void __kernel cAggGen(__global data *dataset, __global int *parameterSet, __global int *cubeDim, __global cAgg *c_agg)
{
    //One kernel one cell

    //Assumption: Some parameters and memory space are initialized before this function is called
    //When these parameters are called /**/ mark will appear before corresponding sentence.

    //Calculate the position of the cube this kernel will handle
    int axisInCube = get_global_id(0);

    c_agg[axisInCube].cubeNum = axisInCube;

    int axisZ = axisInCube % cubeDim[2];
    int axisY = (axisInCube / cubeDim[2]) % cubeDim[1];
    int axisX = (axisInCube / cubeDim[2]) / cubeDim[1];

    //Initial some parameters
    c_agg[axisInCube].max = INIT_MAXIMUM;
    c_agg[axisInCube].min = INIT_MINIMUM;

    c_agg[axisInCube].sum = 0;
    //c_agg[axisInCube].avg = 0;
    c_agg[axisInCube].totalCount = 0;
    c_agg[axisInCube].type_val[0] = axisX;
    c_agg[axisInCube].type_val[1] = axisY;
    c_agg[axisInCube].type_val[2] = axisZ;

    //What's in parameter set:
    //parameterSet[0]: datasetSize
    //parameterSet[1]: histogram type
    //parameterSet[2]: histogram interval number
    //parameterSet[3]: maximum value in dataset
    //parameterSet[4]: minimum value in dataset
    //and?
    c_agg[axisInCube].histogramType = parameterSet[1];
    c_agg[axisInCube].histogramIntervalNum = parameterSet[2];


    //Histogram initialization
    int intervalVal_ewh;

    int max_modified = parameterSet[3] + 1, min_modified = parameterSet[4] - 1;
    if(c_agg[axisInCube].histogramType == HIS_TYPE_EWH)
    {
        int error = (max_modified - min_modified) % c_agg[axisInCube].histogramIntervalNum;

        if (error != 0)
        {
            max_modified += (c_agg[axisInCube].histogramIntervalNum - error) / 2;
            min_modified -= (c_agg[axisInCube].histogramIntervalNum - error) - (c_agg[axisInCube].histogramIntervalNum - error) / 2;
        }
        intervalVal_ewh = (max_modified - min_modified) / c_agg[axisInCube].histogramIntervalNum;

        //Test parameter
        c_agg[axisInCube].ewh_interval_distance = intervalVal_ewh;
        c_agg[axisInCube].max_modified = max_modified;
        c_agg[axisInCube].min_modified = min_modified;

        for(int i = 0; i <= c_agg[axisInCube].histogramIntervalNum; i++)
        {
            c_agg[axisInCube].histogramIntervalMark[i] = min_modified + i * intervalVal_ewh;
            if(i < c_agg[axisInCube].histogramIntervalNum) c_agg[axisInCube].histogramIntervalCount[i] = 0;
        }
    }
    else;

    //Scan & Aggregate
    for(int i = 0; i < parameterSet[0]; i++)
    {
        if(dataset[i].type_val[0] == c_agg[axisInCube].type_val[0] &&
           dataset[i].type_val[1] == c_agg[axisInCube].type_val[1] &&
           dataset[i].type_val[2] == c_agg[axisInCube].type_val[2])
        {
            c_agg[axisInCube].sum += dataset[i].value;
            c_agg[axisInCube].totalCount++;
            //c_agg[axisInCube].avg = ((double)c_agg[axisInCube].sum) / ((double)c_agg[axisInCube].totalCount);

            if(dataset[i].value > c_agg[axisInCube].max) c_agg[axisInCube].max = dataset[i].value;
            if(dataset[i].value < c_agg[axisInCube].min) c_agg[axisInCube].min = dataset[i].value;

            //The part of histogram
            if(c_agg[axisInCube].histogramType == HIS_TYPE_EWH)
                c_agg[axisInCube].histogramIntervalCount[(dataset[i].value - min_modified) / intervalVal_ewh]++;
        }
    }

    return;
}
