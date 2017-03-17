#include <iostream>

#ifdef __linux__
#include <CL/cl.h>
#elif __APPLE__
#include <OpenCL/cl.h>
#else
#endif

#define BUFFERSIZE 1024
#define BUILDLOGSIZE 16384

using namespace std;

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

int main(int argc, char *argv[])
{
    cout << "Hello World!" << endl;
    return 0;
}
