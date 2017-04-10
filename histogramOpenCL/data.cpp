#include "data.h"

#include <cstdlib>
#include <ctime>

void randomDatasetGeneration(data *dataset, int datasetSize, int dimx, int dimy, int dimz)
{
    //srand(time(0));

    for(int i = 0; i < datasetSize; i++)
    {
        dataset[i].type_val[0] = rand() % dimx;
        dataset[i].type_val[1] = rand() % dimy;
        dataset[i].type_val[2] = rand() % dimz;
        dataset[i].value = /*(32768 * (rand() % 32768))*/ + (rand() % 32768);
    }
}

void globalMaxMinLinearScan(data *dataset, int datasetSize, int *max, int *min)
{
    for(int i = 0; i < datasetSize; i++)
    {
        if(*max < dataset[i].value) *max = dataset[i].value;
        if(*min > dataset[i].value) *min = dataset[i].value;
    }
}
