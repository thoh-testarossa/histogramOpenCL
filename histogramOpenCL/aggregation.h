#ifndef AGGREGATION_H
#define AGGREGATION_H

typedef struct cubeAggregation
{
    //The data cube
    int type_val[3]; //Only valid dimensions have a value that greater then 0 otherwise it's 0

    //Histogram information
    int histogramType;
    int histogramIntervalNum;
    int *histogramIntervalMark;
    int *histogramIntervalCount;

    //Other information
    int totalCount;
    double max;
    double min;
    double avg;
    double sum;
}cAgg;

typedef struct deviceAggregation
{
    //The device number
    int device;

    //Histogram information
    int typeDimension[3]; //Only valid dimensions have a value that greater then 0 otherwise it's 0
    int histogramIntervalNum;
    int *histogramIntervalMark;
    int *histogramIntervalCount; //Uses one-dimension array to simulate multiple dimensional histogram:
                                 //(axisX, axisY, axisZ) -> axisX * dimY * dimZ + axisY * dimZ + axisZ
                                 //histogram bars in (axisX, axisY, axisZ)
                                 //4 dimension(?)
                                 //For example: typeX = a, typeY = b, typeZ = c and value between (d, e)

    //Other information
    int totalCount;
    double max;
    double min;
    double avg;
    double sum;
}dAgg;

#endif // AGGREGATION_H
