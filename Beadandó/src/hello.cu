#include <cstdio>
#include <cuda_runtime.h>

__global__ void hello_kernel()
{
    printf("Hello from GPU thread %d (block %d)\n", threadIdx.x, blockIdx.x);
}

static bool check(cudaError_t err, const char* what)
{
    if (err != cudaSuccess) {
        fprintf(stderr, "%s failed: %s\n", what, cudaGetErrorString(err));
        return false;
    }
    return true;
}

int main()
{
    int device = 0;
    if (!check(cudaGetDevice(&device), "cudaGetDevice")) return 1;

    cudaDeviceProp prop;
    if (!check(cudaGetDeviceProperties(&prop, device), "cudaGetDeviceProperties")) return 1;
    printf("Device %d: %s, compute capability %d.%d, %d SMs, %.1f GiB\n", device, prop.name,
           prop.major, prop.minor, prop.multiProcessorCount,
           prop.totalGlobalMem / (1024.0 * 1024.0 * 1024.0));

    hello_kernel<<<2, 4>>>();
    if (!check(cudaGetLastError(), "kernel launch")) return 1;
    if (!check(cudaDeviceSynchronize(), "cudaDeviceSynchronize")) return 1;
    return 0;
}
