#include <iostream>

#ifdef __linux__
#include <CL/cl.h>
#elif __APPLE__
#include <OpenCL/cl.h>
#else
#endif

using namespace std;

int main(int argc, char *argv[])
{
    cout << "Hello World!" << endl;
    return 0;
}
