#include "data.h"
#include "aggregation.h"

void /*__kernel*/ dAggGen(/*__global*/ data *dataset, /*__global*/ int *parameterSet, /*__global*/ int *cubeDim, /*__global*/ dAgg *d_agg)
{
    //One kernel one subset
    //A precomputation is done to determine which subset of the whole dataset should be used to compute by this kernel

    //Assumption: Some parameters and memory space are initialized before this function is called
    //When these parameters are called /**/ mark will appear before corresponding sentence.

    //Calculate the positions of the subset starts from and ends
    int deviceId = get_global_id(0);

    int sp = (deviceId * parameterSet[0]) / parameterSet[5];
    int ep = ((deviceId + 1) * parameterSet[0]) / parameterSet[5];

    //Initial some parameters in sub cAgg
    for(int i = 0; i < cubeDim[0] * cubeDim[1] * cubeDim[2]; i++)
    {
        d_agg[deviceId].d_c_agg[i].max = INIT_MAXIMUM;
        d_agg[deviceId].d_c_agg[i].min = INIT_MINIMUM;

        d_agg[deviceId].d_c_agg[i].sum = 0;
        //c_agg[axisInCube].avg = 0;
        d_agg[deviceId].d_c_agg[i].totalCount = 0;

        //Calculate each cell's type
        int axisZ = i % cubeDim[2];
        int axisY = (i / cubeDim[2]) % cubeDim[1];
        int axisX = (i / cubeDim[2]) / cubeDim[1];
        d_agg[deviceId].d_c_agg[i].type_val[0] = axisX;
        d_agg[deviceId].d_c_agg[i].type_val[1] = axisY;
        d_agg[deviceId].d_c_agg[i].type_val[2] = axisZ;

        //Histogram part
        d_agg[deviceId].d_c_agg[i].histogramType = parameterSet[1];
        d_agg[deviceId].d_c_agg[i].histogramIntervalNum = parameterSet[2];
    }

    //What's in parameter set:
    //parameterSet[0]: datasetSize
    //parameterSet[1]: histogram type
    //parameterSet[2]: histogram interval number
    //parameterSet[3]: maximum value in dataset
    //parameterSet[4]: minimum value in dataset
    //ParameterSet[5]: number of executing kernel (only used in dAgg mode)
    //and?

    //Histogram initialization
    int intervalVal_ewh;

    int max_modified = parameterSet[3] + 1, min_modified = parameterSet[4] - 1;
    if(parameterSet[1] == HIS_TYPE_EWH)
    {
        int error = (max_modified - min_modified) % parameterSet[2];

        if (error != 0)
        {
            max_modified += (parameterSet[2] - error) / 2;
            min_modified -= (parameterSet[2] - error) - (parameterSet[2] - error) / 2;
        }
        intervalVal_ewh = (max_modified - min_modified) / parameterSet[2];

        for(int i = 0; i < cubeDim[0] * cubeDim[1] * cubeDim[2]; i++)
        {
            //Test parameter
            d_agg[deviceId].d_c_agg[i].ewh_interval_distance = intervalVal_ewh;
            d_agg[deviceId].d_c_agg[i].max_modified = max_modified;
            d_agg[deviceId].d_c_agg[i].min_modified = min_modified;
            for(int j = 0; j <= parameterSet[2]; j++)
            {
                d_agg[deviceId].d_c_agg[i].histogramIntervalMark[j] = min_modified + j * intervalVal_ewh;
                if(j < parameterSet[2]) d_agg[deviceId].d_c_agg[i].histogramIntervalCount[j] = 0;
            }
        }

    }
    else;

    //Scan & Aggregate
    for(int i = sp; i < ep; i++)
    {
        int dataPos = dataset[i].type_val[0] * cubeDim[1] * cubeDim[2] + dataset[i].type_val[1] * cubeDim[2] + dataset[i].type_val[2];
        d_agg[deviceId].d_c_agg[dataPos].sum +=dataset[i].value;
        d_agg[deviceId].d_c_agg[dataPos].totalCount++;

        if(dataset[i].value > d_agg[deviceId].d_c_agg[dataPos].max) d_agg[deviceId].d_c_agg[dataPos].max = dataset[i];
        if(dataset[i].value < d_agg[deviceId].d_c_agg[dataPos].min) d_agg[deviceId].d_c_agg[dataPos].min = dataset[i];

        if(parameterSet[1] == HIS_TYPE_EWH) d_agg[deviceId].d_c_agg[dataPos].histogramIntervalCount[(dataset[i].value - min_modified) / intervalVal_ewh]++;
    }

    return;
}
