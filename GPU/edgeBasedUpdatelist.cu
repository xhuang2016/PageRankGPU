#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <iostream>
#include <algorithm>
#include <stdio.h>
#include <assert.h>
#include <fstream>
#include <iomanip>
#include <cstring>
#include "csc.cpp"
#include <malloc.h>
#include <thread>
// use nvprof to caculate time
#define d 0.15
#define blockSize 64
using namespace std;

void thread_fun_read(std::string ,edges& ,int ,std::vector<int>& ,std::vector<int>&);

__global__ void setG_odata(float* g_odata, int N);

__global__ void toleranceReduce(float* d_currentPageRank, float* d_previousPageRank,float* g_odata,int N);

__global__ void change(float* currentPageRank, float* previousPageRank,int N);

__global__ void EdgeBaseUpdate(int* outdegree,int* d_col_1,int* d_col_2,const int N,const int offset, float* currentPageRank, float* previousPageRank,const int count);

__global__ void addDampingFactor(const int N, float* currentPageRank);


int main(int argc, char const *argv[]) {
  // read all we need
  clock_t t;
  t = clock();
  edges e;
  edges r_e;
  std::vector<int> row_offset_1,row_offset_2;
  std::vector<int> column_index_1,column_index_2;
  std::thread thread_1(thread_fun_read,argv[1],std::ref(e),0,std::ref(row_offset_1),std::ref(column_index_1));
  std::thread thread_2(thread_fun_read,argv[1],std::ref(r_e),1,std::ref(row_offset_2),std::ref(column_index_2));
  // std::cout<<"hardware_concurrency()"<<std::thread::hardware_concurrency()<<std::endl;
  thread_1.join();
  thread_2.join();
// exit(1);
  unsigned int N_node = row_offset_1.size()-1;
  unsigned int N_edge = column_index_1.size();

  std::vector<int> col1;
  std::vector<int> col2;
  edges e3;
  read2(argv[1],e3,0,col1,col2);

  int* outdegree = new int[N_node]();
  int* csc_offset = new int[N_node+1]();
  int* csc_outgoing = new int[N_edge]();
  int* col_1 = new int[N_edge]();
  int* col_2 = new int[N_edge]();

  for(int i = 0;i<N_node;i++){
    outdegree[i] = row_offset_1[i+1]-row_offset_1[i];
  }
  // printf("%d\n", N_node);
  // exit(1);

  for(int i = 0; i<N_node+1;i++){
    csc_offset[i] = row_offset_1[i];
  }

  for(int i = 0; i<N_edge;i++){
    csc_outgoing[i] = column_index_1[i];
    col_1[i] = col1[i];
    col_2[i] = col2[i];
  }

  int N = N_node;

  float* currentPageRank = new float[N] ();
  float* previousPageRank = new float[N] ();
  for(int i = 0; i<N;i++){
    previousPageRank[i] = 1.0/N;
  }
  float* g_odata = new float[1]();

  int* d_outdegree = NULL;
  int* d_csc_offset = NULL;
  int* d_csc_outgoing = NULL;
  float* d_currentPageRank = NULL;
  float* d_previousPageRank = NULL;
  float* d_g_odata = NULL;
  int* d_col_1 = NULL;
  int* d_col_2 = NULL;



  if(cudaMalloc(&d_outdegree,N_node*sizeof(int))!=cudaSuccess){
    std::cout<<"fail to malloc d_outdegree"<<std::endl;
    return 0;
  };
  if(cudaMalloc(&d_csc_offset,(N_node+1)*sizeof(int))!=cudaSuccess){
    std::cout<<"fail to malloc d_csc_offset"<<std::endl;
    return 0;
  };
  if(cudaMalloc(&d_csc_outgoing,N_edge*sizeof(int))!=cudaSuccess){
    std::cout<<"fail to malloc d_csc outgoing"<<std::endl;
    return 0;
  };
  if(cudaMalloc(&d_currentPageRank,N*sizeof(float))!=cudaSuccess){
    std::cout<<"fail to malloc d_currentPageRank"<<std::endl;
    return 0;
  };
  if(cudaMalloc(&d_previousPageRank,N*sizeof(float))!=cudaSuccess){
    std::cout<<"fail to malloc d_previousPageRank"<<std::endl;
    return 0;
  };

  if(cudaMalloc(&d_g_odata,(N/blockSize+1)*sizeof(float))!=cudaSuccess){
    std::cout<<"fail to malloc d_previousPageRank"<<std::endl;
    return 0;
  };
  if(cudaMalloc(&d_col_1,N_edge*sizeof(int))!=cudaSuccess){
    std::cout<<"fail to malloc d_col1"<<std::endl;
    return 0;
  };
  if(cudaMalloc(&d_col_2,N_edge*sizeof(int))!=cudaSuccess){
    std::cout<<"fail to malloc d_col2"<<std::endl;
    return 0;
  };

  // dim3 threadPerBlock(32,32);
  // int dimention = (N_edge/(threadPerBlock.x*threadPerBlock.y)+1);
  // dim3 numBlocks(dimention/512+1,512);
  if( cudaMemcpy(d_col_1,col_1,N_edge*sizeof(int),cudaMemcpyHostToDevice) != cudaSuccess){
    std::cout<<"fail to cudaMemcpy d_col_1"<<std::endl;
    return 0;
  };
  if( cudaMemcpy(d_col_2,col_2,N_edge*sizeof(int),cudaMemcpyHostToDevice) != cudaSuccess){
    std::cout<<"fail to cudaMemcpy d_col_2"<<std::endl;
    return 0;
  };

  if( cudaMemcpy(d_outdegree,outdegree,N_node*sizeof(int),cudaMemcpyHostToDevice) != cudaSuccess){
    std::cout<<"fail to cudaMemcpy d_outdegree"<<std::endl;
    return 0;
  };

  if( cudaMemcpy(d_csc_offset,csc_offset,(N_node+1)*sizeof(int),cudaMemcpyHostToDevice) != cudaSuccess){
    std::cout<<"fail to cudaMemcpy d_csc_offset"<<std::endl;
    return 0;
  };

  if( cudaMemcpy(d_csc_outgoing,csc_outgoing,N_edge*sizeof(int),cudaMemcpyHostToDevice) != cudaSuccess){
    std::cout<<"fail to cudaMemcpy d_csc_outgoing"<<std::endl;
    return 0;
  };

  if( cudaMemcpy(d_currentPageRank,currentPageRank,N*sizeof(float),cudaMemcpyHostToDevice) != cudaSuccess){
    std::cout<<"fail to cudaMemcpy d_currentPageRank"<<std::endl;
    return 0;
  };
  if( cudaMemcpy(d_previousPageRank,previousPageRank,N*sizeof(float),cudaMemcpyHostToDevice) != cudaSuccess){
    std::cout<<"fail to cudaMemcpy d_previousPageRank"<<std::endl;
    return 0;
  };

  int iteration_time = 0;
  int N_g_odata = 1;
  float cc = 0;
  dim3 threadPerBlock(1024);
  dim3 numBlocks(1000000/1024+1);
  while(iteration_time<2000000){
    cc = 0;
    setG_odata<<<1,1>>>(d_g_odata,1);
    addDampingFactor<<<N/512+1,512>>>(N,d_currentPageRank);
    if(cudaThreadSynchronize() != cudaSuccess){
      std::cout<<"Synchronize error"<<std::endl;
    };
    int calTime = 0;
    while(calTime < N_edge/1000000){
      EdgeBaseUpdate<<<numBlocks,threadPerBlock>>>(d_outdegree,d_col_1,d_col_2,N,calTime,d_currentPageRank,d_previousPageRank,1000000);
      if(cudaThreadSynchronize() != cudaSuccess){
        std::cout<<"Synchronize error"<<std::endl;
      };
      calTime++;
    }
    int remaining_edge = N_edge%1000000;
    if(remaining_edge != 0){
      EdgeBaseUpdate<<<remaining_edge/1024+1,1024>>>(d_outdegree,d_col_1,d_col_2,N,calTime,d_currentPageRank,d_previousPageRank,remaining_edge);
      if(cudaThreadSynchronize() != cudaSuccess){
        std::cout<<"Synchronize error"<<std::endl;
      };
    }
        // exit(1);
    for(int i = 0;i<N;i++){
      currentPageRank[i] = previousPageRank[i];
    }

    toleranceReduce<<<1,blockSize,blockSize>>>(d_currentPageRank,d_previousPageRank,d_g_odata,N);

    if( cudaMemcpy(g_odata,d_g_odata,sizeof(float),cudaMemcpyDeviceToHost) != cudaSuccess){

      std::cout<<"fail to cudaMemcpy g_odata"<<std::endl;
      return 0;
    };

    for(int i = 0; i<N_g_odata;i++){
      cc += g_odata[i];
    }
    cc = std::sqrt(cc);
    // std::cout<<cc<<std::endl;
    if(cc<10e-7){
      break;
    }

    if(cudaThreadSynchronize() != cudaSuccess){
      std::cout<<"Synchronize error"<<std::endl;
    };

    change<<<N/512+1,512>>>(d_currentPageRank,d_previousPageRank,N);
    if(cudaThreadSynchronize() != cudaSuccess){
      std::cout<<"Synchronize error"<<std::endl;
    };
    ++iteration_time;
  }

  if( cudaMemcpy(previousPageRank,d_currentPageRank,N*sizeof(float),cudaMemcpyDeviceToHost) != cudaSuccess){

    std::cout<<"fail to cudaMemcpy previousPageRank"<<std::endl;
    return 0;
  };

  cudaFree(d_outdegree);
  cudaFree(d_csc_offset);
  cudaFree(d_csc_outgoing);
  cudaFree(d_currentPageRank);
  cudaFree(d_previousPageRank);
  cudaFree(d_g_odata);
  cudaFree(d_col_1);
  cudaFree(d_col_2);

  t = clock() -t;
  printf ("%f\n",((float)t)/CLOCKS_PER_SEC);

  std::fstream output;
  output.open("EdgeBaseUpdatelistResult.txt",std::ios::out);
  output<<std::setprecision(15);
  double sum = 0;
  for(int j = 0;j<N;++j){
    output<<previousPageRank[j]<<std::endl;
    sum +=previousPageRank[j];
  }
  // std::cout<<"sum:"<<sum<<std::endl;
  output.close();
  // t = clock() -t;
  // printf ("%f\n",((float)t)/CLOCKS_PER_SEC);
  return 0;
}

__global__ void addDampingFactor(const int N, float* currentPageRank){
  int i = blockIdx.x*blockDim.x + threadIdx.x;
  if(i<N){
    atomicAdd(&currentPageRank[i],0.15/N);

  }
}

__global__ void EdgeBaseUpdate(int* outdegree,int* d_col_1,int* d_col_2,const int N, const int offset, float* currentPageRank, float* previousPageRank,const int count){
  int i = blockIdx.x*blockDim.x + threadIdx.x;
  // int blockId = blockIdx.x + blockIdx.y * gridDim.x;
  // int i = blockId * (blockDim.x * blockDim.y) + (threadIdx.y * blockDim.x) + threadIdx.x;
  if(i<count){
    int outgoing = d_col_2[i+1000000*offset];
    int origin = d_col_1[i+1000000*offset];
    // int l = 0;
    // int r = N;
    // while(l < r){
    //   int mid = l + (r-l)/2;
    //   if(csc_offset[mid] <= i){
    //     l = mid + 1;
    //   }
    //   else{
    //     r = mid;
    //   }
    // }
    // int origin = l-1;


    float outgoingRank = 0.85*previousPageRank[origin]/outdegree[origin];
    atomicAdd(&currentPageRank[outgoing],outgoingRank);
  }
}

__global__ void change(float* currentPageRank, float* previousPageRank, int N){
  int i = blockIdx.x*blockDim.x + threadIdx.x;
  if(i<N){
    previousPageRank[i] = currentPageRank[i];
    currentPageRank[i] = 0;
  }
}

__global__ void setG_odata(float* g_odata, int N){
  unsigned int i = blockIdx.x*blockDim.x + threadIdx.x;
  if(i<N){
    g_odata[i] = 0.0;
  }
}

__global__ void toleranceReduce(float* d_currentPageRank, float* d_previousPageRank,float* g_odata,int N)
{
    extern __shared__ float sdata[];
    unsigned int tid = threadIdx.x;
    unsigned int i = blockIdx.x*(blockSize*2) + tid;
    sdata[tid] = 0;

    while (i < N)
    {
        sdata[tid] += pow((d_currentPageRank[i]-d_previousPageRank[i]),2) + pow((d_currentPageRank[i+blockSize]-d_previousPageRank[i+blockSize]),2);
        i += blockSize*2;
    }

    __syncthreads();
    if (blockSize >= 1024)
    { if (tid < 512) { sdata[tid] += sdata[tid + 512]; } __syncthreads(); }
    if (blockSize >= 512)
    { if (tid < 256) { sdata[tid] += sdata[tid + 256]; } __syncthreads(); }
    if (blockSize >= 256)
    { if (tid < 128) { sdata[tid] += sdata[tid + 128]; } __syncthreads(); }
    if (blockSize >= 128)
    { if (tid < 64) { sdata[tid] += sdata[tid + 64]; } __syncthreads(); }

    if (tid < 32)
    {
        if (blockSize >= 64)
        sdata[tid] += sdata[tid + 32];
        __syncthreads();
        if (blockSize >= 32)
        sdata[tid] += sdata[tid + 16];
        __syncthreads();
        if (blockSize >= 16)
        sdata[tid] += sdata[tid + 8];
        __syncthreads();
        if (blockSize >= 8)
        sdata[tid] += sdata[tid + 4];
        __syncthreads();
        if (blockSize >= 4)
        sdata[tid] += sdata[tid + 2];
        __syncthreads();
        if (blockSize >= 2)
        sdata[tid] += sdata[tid + 1];
        __syncthreads();
    }

    if (tid == 0) {
      g_odata[0] = sdata[0];
      // printf("%f\n", sdata[0]);
    }
}

void thread_fun_read(std::string s, edges& e,int flag,std::vector<int>& row_offset,std::vector<int>& column_index){
 graphInfo g = read(s,e,flag);
 csc* ptr = new csc(e,g);

 row_offset = ptr->row_offset;
 column_index = ptr->column_index;

 delete ptr;
}
