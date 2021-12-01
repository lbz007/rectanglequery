/* @file: dp_compute_wl.cu
   @date: <date>
*/

#include "utility/src/DetailedPlaceDB.cuh"

DREAMPLACE_BEGIN_NAMESPACE

__global__ void compute_total_hpwl_kernel_edi(DetailedPlaceDB<int> db, const int* xx, const int* yy, int64_t* net_hpwls)
{
    for (int i = blockIdx.x * blockDim.x + threadIdx.x; i < db.num_nets; i += blockDim.x * gridDim.x)
    {
        net_hpwls[i] = static_cast<int64_t>(db.compute_net_hpwl(i, xx, yy))*HPWLScaleTraits<int>::scale; 
    }
}

int64_t compute_total_hpwl_edi(const DetailedPlaceDB<int>& db, const int* xx, const int* yy, int64_t* net_hpwls)
{
    compute_total_hpwl_kernel_edi<<<CPUCeilDiv(db.num_nets, 512), 512>>>(db, xx, yy, net_hpwls); 

    int64_t* d_out = NULL; 
    // Determine temporary device storage requirements
    void     *d_temp_storage = NULL;
    size_t   temp_storage_bytes = 0;
    cub::DeviceReduce::Sum(d_temp_storage, temp_storage_bytes, net_hpwls, d_out, db.num_nets);
    // Allocate temporary storage
    checkCUDA(cudaMalloc(&d_temp_storage, temp_storage_bytes));
    checkCUDA(cudaMalloc(&d_out, sizeof(int64_t))); 
    // Run sum-reduction
    cub::DeviceReduce::Sum(d_temp_storage, temp_storage_bytes, net_hpwls, d_out, db.num_nets);
    // copy d_out to hpwl  
    int64_t hpwl = 0; 
    checkCUDA(cudaMemcpy(&hpwl, d_out, sizeof(int64_t), cudaMemcpyDeviceToHost)); 
    destroyCUDA(d_temp_storage); 
    destroyCUDA(d_out); 

    return hpwl/(db.site_width)/HPWLScaleTraits<int>::scale;
}

DREAMPLACE_END_NAMESPACE
