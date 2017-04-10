#ifndef AGGREGATION_H
#define AGGREGATION_H

#define HIS_TYPE_EWH 10001
#define HIS_TYPE_EHH 10002

#define INIT_MAXIMUM -2147483648
#define INIT_MINIMUM 2147483647

#define HIS_INTERVAL_NUM 10

typedef struct cubeAggregation
{
    //The data cube
    int type_val[3]; //Only valid dimensions have a value that greater then 0 otherwise it's 0

    //Histogram information
    int cubeNum;
    int histogramType;
    int histogramIntervalNum;
    int histogramIntervalMark[HIS_INTERVAL_NUM + 1];
    int histogramIntervalCount[HIS_INTERVAL_NUM];

    //Other information
    int totalCount;
    int max;
    int min;
    //double avg;
    int sum;

    //Test parameter
    int ewh_interval_distance;
    int max_modified;
    int min_modified;
}cAgg;

typedef struct deviceAggregation
{
    //The device number
    int device;

    //Histogram information
    int typeDimension[3]; //Only valid dimensions have a value that greater then 0 otherwise it's 0
    int histogramIntervalNum;
    int histogramIntervalMark[HIS_INTERVAL_NUM + 1];
    int histogramIntervalCount[HIS_INTERVAL_NUM + 1];
                                 //Uses one-dimension array to simulate multiple dimensional histogram:
                                 //(axisX, axisY, axisZ) -> axisX * dimY * dimZ + axisY * dimZ + axisZ
                                 //histogram bars in (axisX, axisY, axisZ)
                                 //4 dimension(?)
                                 //For example: typeX = a, typeY = b, typeZ = c and value between (d, e)

    //Other information
    int totalCount;
    int max;
    int min;
    double avg;
    int sum;
}dAgg;

void initCell_cAgg(cAgg *c_agg);

#endif // AGGREGATION_H
