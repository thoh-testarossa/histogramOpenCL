#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include <ctime>

#include "aggregation.h"
#include "data.h"

#ifdef __linux__
#include <CL/cl.h>
#elif __APPLE__
#include <OpenCL/cl.h>
#else
#endif

#define DATASETSIZE 16777216

#define PARAMETERSETSIZE 6

#define BUFFERSIZE 128
#define BUILDLOGSIZE 1024

#define KERNELNUMBER 16

using namespace std;

//Todo
//1.How to scan dataset to distribute/aggregate all datas into corresponding OLAP clubs


//2.no idea

cl_context CreateContext()
{
    cl_int errNum;
    cl_uint numPlatforms;
    cl_platform_id *platformId = NULL;
    cl_context context = NULL;

    //First, select an OpenCL platform to run on.
    //Normally, you would query for all available platforms and select the most appropriate one.
    //This is an example to query for all available platforms
    //1. To get the number of available platforms
    errNum = clGetPlatformIDs(0, NULL, &numPlatforms);
    if(errNum != CL_SUCCESS || numPlatforms <= 0)
    {
        cerr << "Failed to find any OpenCL platforms." << endl;
        return NULL;
    }
    //2. Allocate memory to store information of all the available platforms
    platformId = new cl_platform_id [numPlatforms];
    if(platformId == NULL)
    {
        cerr << "Failed to allocate memory for information of available platforms." << endl;
        return NULL;
    }
    errNum = clGetPlatformIDs(numPlatforms, platformId, NULL);
    if(errNum != CL_SUCCESS)
    {
        cerr << "Failed to write platformId information into memory." << endl;
        return NULL;
    }
    //3. Get platform detailed information from platformId and select the best one here
    //As an example, we get the vendor information here, and you can change the parameter for the information you want
    char buffer[BUFFERSIZE] = {0};
    cl_uint platformIdToUse = 0;
    for(cl_uint i = 0; i < numPlatforms; i++)
    {
        errNum = clGetPlatformInfo(platformId[i], /*Change this parameter to change the information you want*/CL_PLATFORM_VENDOR, sizeof(char) * BUFFERSIZE, buffer, NULL);
        if(errNum != CL_SUCCESS)
        {
            cerr << "Failed to get information from platform " << (int)i << "." << endl;
            continue;
        }
        //And this part is used to determine platformIdToUse
        /*
         *
         *
         */
    }
    //The result when testing with mac pro:
    //Only 1 platform available: Apple
    //So we just simply choose id = 0
    platformIdToUse = 0;

    //Next, create an OpenCL context on the platform.
    //Attempt to create a GPU-based context, and if that fails, try to create a CPU-based context
    cl_context_properties contextProperties[] =
    {
        CL_CONTEXT_PLATFORM, (cl_context_properties)platformId[platformIdToUse], 0
    };
    //context = clCreateContextFromType(contextProperties, CL_DEVICE_TYPE_GPU, NULL, NULL, &errNum);
    //if(errNum != CL_SUCCESS)
    //{
        //cout << "Could not create GPU context, trying CPU..." << endl;
        context = clCreateContextFromType(contextProperties, CL_DEVICE_TYPE_CPU, NULL, NULL, &errNum);
        if(errNum != CL_SUCCESS)
        {
            cerr << "Could not create an OpenCL GPU or CPU context." << endl;
            return NULL;
        }
    //}
    return context;
}

cl_command_queue CreateCommandQueue(cl_context context, cl_device_id *device)
{
    cl_int errNum;
    cl_device_id *devices;
    cl_uint numDevices;
    cl_command_queue commandQueue = NULL;
    size_t deviceBufferSize = -1;

    //First get the size of the device buffer
    errNum = clGetContextInfo(context, CL_CONTEXT_DEVICES, 0, NULL, &deviceBufferSize);
    if(errNum != CL_SUCCESS)
    {
        cerr << "Failed call to clGetContextInfo(..., CL_CONTEXT_DEVICES, ...)" << endl;
        return NULL;
    }
    if(deviceBufferSize <= 0)
    {
        cerr << "No device available." << endl;
        return NULL;
    }

    //Allocate memory for the devices buffer
    numDevices = deviceBufferSize / sizeof(cl_device_id);
    devices = new cl_device_id[numDevices];
    errNum = clGetContextInfo(context, CL_CONTEXT_DEVICES, deviceBufferSize, devices, NULL);
    if(errNum != CL_SUCCESS)
    {
        cerr << "Failed to get device IDs" << endl;
        return NULL;
    }

    //You would likely use all available devices or choose the highest performance device based on OpenCL device queries.
    //As an example, we get the device name information here, and you can change the parameter for the information you want.
    char buffer[BUFFERSIZE] = {0};
    cl_uint deviceIdToUse = 0;
    for(cl_uint i = 0; i < numDevices; i++)
    {
        errNum = clGetDeviceInfo(devices[i], /*Change the parameter here to get what you want*/CL_DEVICE_NAME, sizeof(char) * BUFFERSIZE, buffer, NULL);
        if(errNum != CL_SUCCESS)
        {
            cerr << "Failed to get information from device " << (int)i << "." << endl;
            continue;
        }
        //And this part is used to determine (many) deviceIdToUse
        /*
         *
         *
         */
    }
    //Again, the result of the test shows that mac has only one gpu which can be used, so we just simply choose the first device(gpu)
    deviceIdToUse = 0;

    commandQueue = clCreateCommandQueue(context, devices[deviceIdToUse], CL_QUEUE_PROFILING_ENABLE, NULL);

    if(commandQueue == NULL)
    {
        cerr << "Failed to create commandQueue for device " << (int)deviceIdToUse << endl;
        return NULL;
    }

    *device = devices[deviceIdToUse];
    delete [] devices;
    return commandQueue;
}

cl_program CreateProgram(cl_context context, cl_device_id device, const char *fileName)
{
    cl_int errNum;
    cl_program program;

    ifstream kernelFile(fileName, ios::in);
    if(!kernelFile.is_open())
    {
        cerr << "Failed to open file for reading: " << fileName << endl;
        return NULL;
    }

    ostringstream oss;
    oss << kernelFile.rdbuf();

    string srcStdStr = oss.str();
    const char *srcStr = srcStdStr.c_str();
    program = clCreateProgramWithSource(context, 1, (const char**)&srcStr, NULL, NULL);

    if(program == NULL)
    {
        cerr << "Failed to create CL program from source." << endl;
        return NULL;
    }

    errNum = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if(errNum != CL_SUCCESS)
    {
        //Determine the reason for the error
        char buildLog[BUILDLOGSIZE];
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, sizeof(buildLog), buildLog, NULL);
        cerr << "Error in kernel: " << endl;
        for(int i = 0; i < BUILDLOGSIZE; i++)
            cerr << buildLog[i];
        cerr << endl;
        //cerr << buildLog;
        clReleaseProgram(program);
        return NULL;
    }

    return program;
}

bool CreateMemObjects_cAgg(cl_context context, cl_mem memObjects[4], data *dataset, int parameterSet[PARAMETERSETSIZE], int cubeDim[3], cAgg *c_agg)
{
    memObjects[0] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(data) * DATASETSIZE, dataset, NULL);
    memObjects[1] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int) * PARAMETERSETSIZE, parameterSet, NULL);
    memObjects[2] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int) * 3, cubeDim, NULL);
    memObjects[3] = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(cAgg) * DIMX * DIMY * DIMZ, c_agg, NULL);
    if(memObjects[0] == NULL || memObjects[1] == NULL || memObjects[2] == NULL || memObjects[3] == NULL)
    {
        cerr << "Error creating memory objects." << endl;
        return false;
    }
    return true;
}

bool CreateMemObjects_dAgg(cl_context context, cl_mem memObjects[4], data *dataset, int parameterSet[PARAMETERSETSIZE], int cubeDim[3], dAgg *d_agg)
{
    memObjects[0] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(data) * DATASETSIZE, dataset, NULL);
    memObjects[1] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int) * PARAMETERSETSIZE, parameterSet, NULL);
    memObjects[2] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int) * 3, cubeDim, NULL);
    memObjects[3] = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(dAgg) * KERNELNUMBER, d_agg, NULL);
    if(memObjects[0] == NULL || memObjects[1] == NULL || memObjects[2] == NULL || memObjects[3] == NULL)
    {
        cerr << "Error creating memory objects." << endl;
        return false;
    }
    return true;
}

void Cleanup(cl_context context, cl_command_queue commandQueue, cl_program program, cl_kernel kernel, cl_mem *memObjects)
{

}

int main(int argc, char *argv[])
{
    int time_0 = clock();
    cout << time_0 << endl;

    //Initialize dataset
    data *dataset = new data [DATASETSIZE];
    bool needToGenerate = false;
    ifstream datasetIn = ifstream("dataset.txt", ios::in);
    if(datasetIn.is_open())
    {
        int tmp;
        datasetIn >> tmp; needToGenerate |= (tmp != DATASETSIZE);
        datasetIn >> tmp; needToGenerate |= (tmp != DIMX);
        datasetIn >> tmp; needToGenerate |= (tmp != DIMY);
        datasetIn >> tmp; needToGenerate |= (tmp != DIMZ);

        if(!needToGenerate)
        {
            for(int i = 0; i < DATASETSIZE; i++)
                datasetIn >> dataset[i].type_val[0] >> dataset[i].type_val[1] >> dataset[i].type_val[2] >> dataset[i].value;
        }
        else
            datasetIn.close();
    }
    if(!datasetIn.is_open() || needToGenerate)
    {
        randomDatasetGeneration(dataset, DATASETSIZE, DIMX, DIMY, DIMZ);
        ofstream datasetOut = ofstream("dataset.txt", ios::out);
        datasetOut << DATASETSIZE << " " << DIMX << " " << DIMY << " " << DIMZ << endl;
        for(int i = 0; i < DATASETSIZE; i++)
        {
            datasetOut << dataset[i].type_val[0] << " " << dataset[i].type_val[1] << " " <<  dataset[i].type_val[2] << " ";
            datasetOut << dataset[i].value << endl;
        }
        datasetOut.close();
    }

    datasetIn.close();

    int time_1 = clock();
    cout << time_1 << endl;

    //Initialize OpenCL parameters
    cl_context context = 0;
    cl_command_queue commandQueue = 0;
    cl_program program = 0;
    cl_device_id device = 0;
    cl_kernel kernel = 0;
    cl_mem memObjects[4] = {0, 0, 0, 0};
    cl_int errNum;

    //Create an OpenCL context on first available platform
    context = CreateContext();
    if(context == NULL)
    {
        cerr << "Failed to create OpenCL context." << endl;
        return 1;
    }

    //Create a command-queue on the first device available on the created context
    commandQueue = CreateCommandQueue(context, &device);
    if(commandQueue == NULL)
    {
        Cleanup(context, commandQueue, program, kernel, memObjects);
        return 1;
    }

    //Create OpenCL program from .cl kernel source
    //cAgg part
    /*program = CreateProgram(context, device, "cubeAggregationGeneration.cl");
    if(program == NULL)
    {
        Cleanup(context, commandQueue, program, kernel, memObjects);
        return 1;
    }*/
    //dAgg part
    program = CreateProgram(context, device, "deviceAggregationGeneration.cl");
    if(program == NULL)
    {
        Cleanup(context, commandQueue, program, kernel, memObjects);
        return 1;
    }

    //Create OpenCL kernel
    //cAgg part
    /*kernel = clCreateKernel(program, "cAggGen", NULL);
    if(kernel == NULL)
    {
        cerr << "Failed to create kernel" << endl;
        Cleanup(context, commandQueue, program, kernel, memObjects);
        return 1;
    }*/
    //dAgg part
    kernel = clCreateKernel(program, "dAggGen", NULL);
    if(kernel == NULL)
    {
        cerr << "Failed to create kernel" << endl;
        Cleanup(context, commandQueue, program, kernel, memObjects);
        return 1;
    }

    int time_2 = clock();
    cout << time_2 << endl;

    //Create memory objects that will be used as arguments to kernel.
    //First create host memory arrays that will be used to store the arguments to the kernel.
    //void /*__kernel*/ cAggGen(/*__global*/ data *dataset, /*__global*/ int *parameterSet, /*__global*/ int *cubeDim, /*__global*/ cAgg *c_agg)
    int parameterSet[PARAMETERSETSIZE];
    //What's in parameter set:
    //parameterSet[0]: datasetSize
    //parameterSet[1]: histogram type
    //parameterSet[2]: histogram interval number
    //parameterSet[3]: maximum value in dataset
    //parameterSet[4]: minimum value in dataset
    //ParameterSet[5]: number of executing kernel (only used in dAgg mode)
    //and?
    parameterSet[0] = DATASETSIZE;
    parameterSet[1] = HIS_TYPE_EWH;
    parameterSet[2] = HIS_INTERVAL_NUM;
    parameterSet[3] = INIT_MAXIMUM;
    parameterSet[4] = INIT_MINIMUM;
    parameterSet[5] = KERNELNUMBER;

    int cubeDim[3];
    cubeDim[0] = DIMX, cubeDim[1] = DIMY, cubeDim[2] = DIMZ;

    //cAgg initialization
    cAgg c_agg[DIMX * DIMY * DIMZ];
    cAgg c_agg_result[DIMX * DIMY * DIMZ];
    for(int i = 0; i < DIMX * DIMY * DIMZ; i++) c_agg_result[i].histogramType = 99999;
    //for(int i = 0; i < DIMX * DIMY * DIMZ; i++)
        //initCell_cAgg(&c_agg[i]);
    //dAgg initialization
    dAgg *d_agg = new dAgg[KERNELNUMBER];
    dAgg *d_agg_result = new dAgg[KERNELNUMBER];

    //Scan dataset and get maximum and minimum value of the whole set
    globalMaxMinLinearScan(dataset, DATASETSIZE, &parameterSet[3], &parameterSet[4]);

    //Memory Object generation
    //cAgg part
    /*if(!CreateMemObjects_cAgg(context, memObjects, dataset, parameterSet, cubeDim, c_agg))
    {
        Cleanup(context, commandQueue, program, kernel, memObjects);
        return 1;
    }*/
    //dAgg part
    if(!CreateMemObjects_dAgg(context, memObjects, dataset, parameterSet, cubeDim, d_agg))
    {
        Cleanup(context, commandQueue, program, kernel, memObjects);
        return 1;
    }

    //Set the kernel arguments
    errNum = clSetKernelArg(kernel, 0, sizeof(cl_mem), &memObjects[0]);
    errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &memObjects[1]);
    errNum |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &memObjects[2]);
    errNum |= clSetKernelArg(kernel, 3, sizeof(cl_mem), &memObjects[3]);
    if(errNum != CL_SUCCESS)
    {
        cerr << "Error setting kernel arguments." << endl;
        Cleanup(context, commandQueue, program, kernel, memObjects);
        return 1;
    }

    int time_3 = clock();
    cout << time_3 << endl;

    //Queue the kernel up for execution across the array
    //Using clEnqueueNDRangeKernel() function
    cl_ulong time_4_start;
    cl_ulong time_4_mid1;
    cl_ulong time_4_mid2;
    cl_ulong time_4_end;

    //Determine global kernel size (?)
    /*cAgg*///size_t globalWorkSize[1] = {DIMX * DIMY * DIMZ};
    /*dAgg*/size_t globalWorkSize[1] = {KERNELNUMBER};
    size_t localWorkSize[1] = {1};
    cl_event event;
    errNum = clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL, globalWorkSize, localWorkSize, 0, NULL, &event);
    if(errNum != CL_SUCCESS)
    {
        cerr << "Error queuing kernel for execution." << endl;
        Cleanup(context, commandQueue, program, kernel, memObjects);
        return 1;
    }
    clWaitForEvents(1, &event);
    clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(time_4_start), &time_4_start, NULL);
    clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(time_4_mid1), &time_4_mid1, NULL);

    //Read the output buffer back to the host
    //Using clEnqueueReadBuffer() function
    /*cAgg*///errNum = clEnqueueReadBuffer(commandQueue, memObjects[3], CL_TRUE, 0, DIMX * DIMY * DIMZ * sizeof(cAgg), c_agg_result, 0, NULL, &event);
    /*dAgg*/errNum = clEnqueueReadBuffer(commandQueue, memObjects[3], CL_TRUE, 0, KERNELNUMBER * sizeof(dAgg), d_agg_result, 0, NULL, &event);
    if(errNum != CL_SUCCESS)
    {
        cerr << "Error reading result buffer." << endl;
        Cleanup(context, commandQueue, program, kernel, memObjects);
        return 1;
    }
    clWaitForEvents(1, &event);
    clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(time_4_mid2), &time_4_mid2, NULL);
    clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(time_4_end), &time_4_end, NULL);

    int time_4_0 = clock();
    cout << time_4_0 << endl;

    //dAgg -> cAgg part
    for(int i = 0; i < KERNELNUMBER; i++)
    {
        for(int j = 0; j < DIMX * DIMY * DIMZ; j++)
        {
            c_agg_result[j].cubeNum = j;
            c_agg_result[j].totalCount += d_agg_result[i].d_c_agg[j].totalCount;
            c_agg_result[j].sum += d_agg_result[i].d_c_agg[j].sum;

            if(i == 0 | c_agg_result[j].max < d_agg_result[i].d_c_agg[j].max)
                c_agg_result[j].max = d_agg_result[i].d_c_agg[j].max;
            if(i == 0 | c_agg_result[j].min > d_agg_result[i].d_c_agg[j].min)
                c_agg_result[j].min = d_agg_result[i].d_c_agg[j].min;

            for(int k = 0; k <= HIS_INTERVAL_NUM; k++)
            {
                c_agg_result[j].histogramIntervalMark[k] = d_agg_result[i].d_c_agg[j].histogramIntervalMark[k];
                if(k < HIS_INTERVAL_NUM)
                    c_agg_result[j].histogramIntervalCount[k] += d_agg_result[i].d_c_agg[j].histogramIntervalCount[k];
            }
        }
    }

    int time_4 = clock();
    cout << time_4 << endl;

    //Summary
    cout << endl << "Summary:" << endl << endl;

    //Output the result buffer
    //cAgg part
    for(int i = 0; i < DIMX * DIMY * DIMZ; i++)
    {
        cout << c_agg_result[i].cubeNum << endl;
        cout << c_agg_result[i].totalCount << endl;
        cout << c_agg_result[i].max << " " << c_agg_result[i].min << endl;
        cout << c_agg_result[i].sum << endl;
        //cout << c_agg_result[i].avg << endl;

        for(int j = 0; j <= HIS_INTERVAL_NUM; j++)
        {
            cout << c_agg_result[i].histogramIntervalMark[j];
            if(j < HIS_INTERVAL_NUM)
                cout << " " << c_agg_result[i].histogramIntervalCount[j];
            cout << endl;
        }

        cout << endl;
    }
    //dAgg part

    cout << endl;

    //Output running time
    double total_running_time = (double)(time_2 - time_1) / CLOCKS_PER_SEC + (double)(time_3 - time_2) / CLOCKS_PER_SEC + (double)(time_4_mid1 - time_4_start) / CLOCKS_PER_SEC / 1000 + (double)(time_4_end - time_4_mid2) / CLOCKS_PER_SEC / 1000 + (double)(time_4 - time_4_0) / CLOCKS_PER_SEC;

    //Program initialization & Reading/Generating data part
    cout << "Program initialization & Reading/Generating data part:" << endl << (double)(time_1 - time_0) / CLOCKS_PER_SEC << endl;
    //OpenCL initialization part
    cout << "OpenCL initialization part:" << endl << (double)(time_2 - time_1) / CLOCKS_PER_SEC << " " << (double)(time_2 - time_1) / CLOCKS_PER_SEC / total_running_time * 100 << "%" << endl;
    //Memory setting & copying part
    cout << "Memory setting & copying in part:" << endl << (double)(time_3 - time_2) / CLOCKS_PER_SEC << " " << (double)(time_3 - time_2) / CLOCKS_PER_SEC / total_running_time * 100 << "%" << endl;
    //Running part
    cout << "Running part(Executing):" << endl << (double)(time_4_mid1 - time_4_start) / CLOCKS_PER_SEC / 1000 << " " << (double)(time_4_mid1 - time_4_start) / CLOCKS_PER_SEC / 1000 / total_running_time * 100 << "%" << endl;
    cout << "Running part(copying out):" << endl << (double)(time_4_end - time_4_mid2) / CLOCKS_PER_SEC / 1000 << " " << (double)(time_4_end - time_4_mid2) / CLOCKS_PER_SEC / 1000 / total_running_time * 100 << "%" << endl;
    cout << "dAgg -> cAgg part:" << endl << (double)(time_4 - time_4_0) / CLOCKS_PER_SEC << " " << (double)(time_4 - time_4_0) / CLOCKS_PER_SEC / total_running_time * 100 << "%" << endl;

    return 0;
}
