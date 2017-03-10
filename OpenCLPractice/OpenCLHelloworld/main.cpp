#include <iostream>

#ifdef __linux__
#include <CL/cl.h>
#elif __APPLE__
#include <OpenCL/cl.h>
#else
#endif

using namespace std;

cl_context CreateContext()
{
    cl_int errNum;
    cl_uint numPlatforms;
    cl_platform_id firstPlatformId;
    cl_context context = NULL;

    //First, select an OpenCL platform to run on.
    //For this example, we simply choose the first available platform.
    //Normally, you would query for all available platforms and select the most appropriate one.
    errNum = clGetPlatformIDs(1, &firstPlatformId, &numPlatforms);
    if(errNum != CL_SUCCESS || numPlatforms <= 0)
    {
        cerr << "Failed to find any OpenCL platforms.\n";
        return NULL;
    }

}

int main(int argc, char *argv[])
{
}
