#ifndef DATA_H
#define DATA_H

typedef struct data
{
    int type_val[3];
    int value;
}data;

void randomDatasetGeneration(data *dataset, int datasetSize, int dimx, int dimy, int dimz);

void globalMaxMinLinearScan(data *dataset, int datasetSize, int *max, int *min);

#endif // DATA_H
