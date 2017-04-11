#include "data.h"
#include "aggregation.h"

void /*__kernel*/ dimensionAggregate(/*__global*/ cAgg *c_agg, /*__global*/ cAgg *c_agg_gen, /*__global*/ int cubeDim[3], /*__global*/ int cubeDim_gen[3], /*__global*/ int dimToAggregate[1])
{
    int axisInGenCube = get_global_id(0);

    c_agg[axisInGenCube].cubeNum = axisInGenCube;

    int axisX, axisY, axisZ;

    //Check if it's necessary to do this aggregation at this dimension first
    if(dimToAggregate[0] == X_AGGREGATE && cubeDim[0] == 1 ||
       dimToAggregate[1] == Y_AGGREGATE && cubeDim[1] == 1 ||
       dimToAggregate[2] == Z_AGGREGATE && cubeDim[2] == 1) return;

    //Calculate the corresponding axis in c_agg_gen
    if(dimToAggregate[0] == X_AGGREGATE)
    {
        axisZ = axisInGenCube % cubeDim[2];
        axisY = (axisInGenCube / cubeDim[2]) % cubeDim[1];
        axisX = 0;
    }
    else if(dimToAggregate[0] == Y_AGGREGATE)
    {
        axisZ = axisInGenCube % cubeDim[2];
        axisY = 0;
        axisX = (axisInGenCube / cubeDim[2]) % cubeDim[0];
    }
    else if(dimToAggregate[0] == Z_AGGREGATE)
    {
        axisZ = 0;
        axisY = axisInGenCube % cubeDim[1];
        axisX = (axisInGenCube / cubeDim[1]) % cubeDim[0];
    }

    //Initial some parameters
    if(c_agg_gen[axisInGenCube].histogramType == HIS_TYPE_EWH)
    {
        //Because some following parameters are all the same in any cell in c_agg, so we can do like this even if in fact the cell c_agg_gen[axisInGenCube]'s corresponding cell isn't c_agg[axisInGenCube]
        c_agg_gen[axisInGenCube].ewh_interval_distance = c_agg[axisInGenCube].ewh_interval_distance;
        for(int i = 0; i <= c_agg_gen[axisInGenCube].histogramIntervalNum; i++)
        {
            c_agg_gen[axisInGenCube].histogramIntervalMark[i] == c_agg[axisInGenCube].histogramIntervalMark[i];
            if(i < c_agg_gen[axisInGenCube].histogramIntervalNum) c_agg_gen[axisInGenCube].histogramIntervalCount[i] = 0;
        }
        c_agg_gen[axisInGenCube].max_modified = c_agg[axisInGenCube].max_modified;
        c_agg_gen[axisInGenCube].min_modified = c_agg[axisInGenCube].min_modified;
    }
    c_agg_gen[axisInGenCube].totalCount = 0;
    c_agg_gen[axisInGenCube].sum = 0;
    c_agg_gen[axisInGenCube].max = INIT_MAXIMUM;
    c_agg_gen[axisInGenCube].min = INIT_MINIMUM;
    c_agg_gen[axisInGenCube].type_val[3] = {axisX, axisY, axisZ};

    //if Z_AGGREGATE, we aggregate c_agg into c_agg_gen at Z dimension
    //How can we use axisInCube of a cell in c_agg to determine which cell in c_agg_gen will the cell be aggregated into?
    //Hint: The cells which type_val[0] and type_val[1] are both the same can be aggregated into one cell in c_agg_gen, which means the cells which axisInCube / DIMZ are the same can be aggregated into one cell
    //So, the kernel, which generates c_agg_gen[axisInGenCube], should scans from c_agg[(axisX * DIMY + axisY) * DIMZ + (0)] to c_agg[(axisX * DIMY + axisY) * DIMZ + (DIMZ - 1)] to collect data for c_agg_gen[axisInGenCube]'s generation
    if(dimToAggregate[0] == Z_AGGREGATE)
    {
        for(int i = 0; i < cubeDim[2]; i++)
        {
            int axisInCube = axisX * cubeDim[1] * cubeDim[2] + axisY * cubeDim[2] + i;
            //Aggregate c_agg[axisX * cubeDim[1] * cubeDim[2] + axisY * cubeDim[2] + i] into c_agg_gen[axisInGenCube]
            if(c_agg_gen[axisInGenCube].max < c_agg[axisInCube].max)
                c_agg_gen[axisInGenCube].max = c_agg[axisInCube].max;
            if(c_agg_gen[axisInGenCube].min < c_agg[axisInCube].min)
                c_agg_gen[axisInGenCube].min = c_agg[axisInCube].min;
            c_agg_gen[axisInGenCube].totalCount += c_agg[axisInCube].totalCount;
            c_agg_gen[axisInGenCube].sum += c_agg[axisInCube].sum;
            //Histogram part
            if(c_agg_gen[axisInGenCube].histogramType == HIS_TYPE_EWH)
            {
                for(int j = 0; j < c_agg_gen[axisInGenCube].histogramIntervalNum; j++)
                {
                    c_agg_gen[axisInGenCube].histogramIntervalCount[j] += c_agg[axisInCube].histogramIntervalCount[j];
                }
            }
        }
    }

    //If Y_AGGREGATE, we aggregate c_agg into c_agg_gen at Y dimension
    //The kernel which generates c_agg_gen[axisInGenCube] should scans from c_agg[axisX * DIMY * DIMZ + (0 * DIMZ) + axisZ] to c_agg[axisX * DIMY * DIMZ + ((DIMY - 1) * DIMZ) + axisZ] with step DIMZ
    else if(dimToAggregate[0] == Y_AGGREGATE)
    {
        for(int i = 0; i < cubeDim[1]; i++)
        {
            int axisInCube = axisX * cubeDim[1] * cubeDim[2] + (i * cubeDim[2]) + axisZ;
            //Aggregate c_agg[axisX * cubeDim[1] * cubeDim[2] + (i * cubeDim[2]) + axisZ] into c_agg_gen[axisInGenCube]
            if(c_agg_gen[axisInGenCube].max < c_agg[axisInCube].max)
                c_agg_gen[axisInGenCube].max = c_agg[axisInCube].max;
            if(c_agg_gen[axisInGenCube].min < c_agg[axisInCube].min)
                c_agg_gen[axisInGenCube].min = c_agg[axisInCube].min;
            c_agg_gen[axisInGenCube].totalCount += c_agg[axisInCube].totalCount;
            c_agg_gen[axisInGenCube].sum += c_agg[axisInCube].sum;
            //Histogram part
            if(c_agg_gen[axisInGenCube].histogramType == HIS_TYPE_EWH)
            {
                for(int j = 0; j < c_agg_gen[axisInGenCube].histogramIntervalNum; j++)
                {
                    c_agg_gen[axisInGenCube].histogramIntervalCount[j] += c_agg[axisInCube].histogramIntervalCount[j];
                }
            }
        }
    }

    //If X_AGGREGATE, we aggregate c_agg into c_agg_gen at X dimension
    //The kernel which generates c_agg_gen[axisInGenCube] should scans from c_agg[(0 * DIMY * DIMZ) + axisY * DIMZ + axisZ] to c_agg[(DIMX - 1) * DIMY * DIMZ + axisY * DIMZ + axisZ] with step DIMY * DIMZ
    else if(dimToAggregate[0] == X_AGGREGATE)
    {
        for(int i = 0; i < cubeDim[0]; i++)
        {
            int axisInCube = i * cubeDim[1] * cubeDim[2] + axisY * cubeDim[2] + axisZ;
            //Aggregate c_agg[i * cubeDim[1] * cubeDim[2] + axisY * cubeDim[2] + axisZ] into c_agg_gen[axisInGenCube]
            if(c_agg_gen[axisInGenCube].max < c_agg[axisInCube].max)
                c_agg_gen[axisInGenCube].max = c_agg[axisInCube].max;
            if(c_agg_gen[axisInGenCube].min < c_agg[axisInCube].min)
                c_agg_gen[axisInGenCube].min = c_agg[axisInCube].min;
            c_agg_gen[axisInGenCube].totalCount += c_agg[axisInCube].totalCount;
            c_agg_gen[axisInGenCube].sum += c_agg[axisInCube].sum;
            //Histogram part
            if(c_agg_gen[axisInGenCube].histogramType == HIS_TYPE_EWH)
            {
                for(int j = 0; j < c_agg_gen[axisInGenCube].histogramIntervalNum; j++)
                {
                    c_agg_gen[axisInGenCube].histogramIntervalCount[j] += c_agg[axisInCube].histogramIntervalCount[j];
                }
            }
        }
    }
}
