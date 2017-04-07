#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include "aggregation.h"
#include "data.h"

#ifdef __linux__
#include <CL/cl.h>
#elif __APPLE__
#include <OpenCL/cl.h>
#else
#endif

#define DATASETSIZE 16384
#define DIMX 7
#define DIMY 11
#define DIMZ 13
#define HIS_INTERVAL_NUM 10

#define PARAMETERSETSIZE 5

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
    //Next, create an OpenCL context on the platform.
    //Attempt to create a GPU-based context, and if that fails, try to create a CPU-based context
    cl_context_properties contextProperties[] =
    {
        CL_CONTEXT_PLATFORM, (cl_context_properties)platformId[platformIdToUse], 0
    };
    context = clCreateContextFromType(contextProperties, CL_DEVICE_TYPE_GPU, NULL, NULL, &errNum);
    if(errNum != CL_SUCCESS)
    {
        cout << "Could not create GPU context, trying CPU..." << endl;
        context = clCreateContextFromType(contextProperties, CL_DEVICE_TYPE_CPU, NULL, NULL, &errNum);
        if(errNum != CL_SUCCESS)
        {
            cerr << "Could not create an OpenCL GPU or CPU context." << endl;
            return NULL;
        }
    }
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

    commandQueue = clCreateCommandQueue(context, devices[deviceIdToUse], 0, NULL);

    if(commandQueue == NULL)
    {
        cerr << "Failed to create commandQueue for device 0" << endl;
        return NULL;
    }

    *device = devices[0];
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
        cerr << buildLog;
        clReleaseProgram(program);
        return NULL;
    }

    return program;
}

bool CreateMemObjects(cl_context context, cl_mem memObjects[4], data *dataset, int parameterSet[PARAMETERSETSIZE], int cubeDim[3], cAgg *c_agg)
{
    memObjects[0] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(data) * DATASETSIZE, dataset, NULL);
    memObjects[1] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int) * PARAMETERSETSIZE, parameterSet, NULL);
    memObjects[2] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int) * 3, cubeDim, NULL);
    memObjects[3] = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(cAgg) * DIMX * DIMY * DIMZ, c_agg, NULL);
    if(memObjects[0] == NULL || memObjects[1] == NULL || memObjects[2] == NULL || memObjects[3] == NULL)
    {
        cerr << "Error creating memory objects." << endl;
        return false;
    }
    return true;
}

void Cleanup(cl_context context, cl_command_queue commandQueue, cl_program program, cl_kernel kernel, cl_mem memObjects)
{

}

int main(int argc, char *argv[])
{
    data dataset[DATASETSIZE];
    /*Init dataset*/

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
    program = CreateProgram(context, device, "cubeAggregationGeneration.cl");
    if(program == NULL)
    {
        Cleanup(context, commandQueue, program, kernel, memObjects);
        return 1;
    }

    //Create OpenCL kernel
    kernel = clCreateKernel(program, "cubeAggregationGeneration", NULL);
    if(kernel == NULL)
    {
        cerr << "Failed to create kernel" << endl;
        Cleanup(context, commandQueue, program, kernel, memObjects);
        return 1;
    }

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
    //and?
    parameterSet[0] = DATASETSIZE;
    parameterSet[1] = HIS_TYPE_EWH;
    parameterSet[2] = HIS_INTERVAL_NUM;

    int cubeDim[3];
    cubeDim[0] = DIMX, cubeDim[1] = DIMY, cubeDim[2] = DIMZ;

    //cAgg initialization
    cAgg c_agg[DIMX * DIMY * DIMZ];
    for(int i = 0; i < DIMX * DIMY * DIMZ; i++)
        initCell_cAgg(c_agg[i], HIS_INTERVAL_NUM);

    /*Scan dataset and get maximum and minimum value of the whole set*/


    //Memory Object generation
    if(!CreateMemObjects(context, memObjects, dataset, parameterSet, cubeDim, c_agg))
    {
        Cleanup(context, commandQueue, program, kernel, memObjects);
        return 1;
    }

    //Set the kernel arguments
    errNum = clSetKernelArg(kernel, 0, sizeof(cl_mem), &memObjects[0]) |
            clSetKernelArg(kernel, 1, sizeof(cl_mem), &memObjects[1]) |
            clSetKernelArg(kernel, 2, sizeof(cl_mem), &memObjects[2]) |
            clSetKernelArg(kernel, 3, sizeof(cl_mem), &memObjects[3]);
    if(errNum != CL_SUCCESS)
    {
        cerr << "Error setting kernel arguments." << endl;
        Cleanup(context, commandQueue, program, kernel, memObjects);
        return 1;
    }

    //Queue the kernel up for execution across the array
    //Using clEnqueueNDRangeKernel() function
    int globalWorkSize[3] = {DIMX, DIMY, DIMZ};
    int localWorkSize[3] = {1, 1, 1};
    errNum = clEnqueueNDRangeKernel(commandQueue, kernel, 3, NULL, globalWorkSize, localWorkSize, 0, NULL, NULL);
    if(errNum != CL_SUCCESS)
    {
        cerr << "Error queuing kernel for execution." << endl;
        Cleanup(context, commandQueue, program, kernel, memObjects);
        return 1;
    }

    //Read the output buffer back to the host
    //Using clEnqueueReadBuffer() function
    errNum = clEnqueueReadBuffer(commandQueue, memObjects[3], CL_TRUE, 0, DIMX * DIMY * DIMZ * sizeof(cAgg), c_agg, 0, NULL, NULL);
    if(errNum != CL_SUCCESS)
    {
        cerr << "Error reading result buffer." << endl;
        Cleanup(context, commandQueue, program, kernel, memObjects);
        return 1;
    }

    //Output the result buffer

    return 0;
}
