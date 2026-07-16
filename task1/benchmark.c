#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "blis.h"

double get_time() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}


void benchmark_gemm(int n, int run_num) {
    double alpha = 1.0;
    double beta = 0.0;


    double* A = (double*)malloc(n * n * sizeof(double));
    double* B = (double*)malloc(n * n * sizeof(double));
    double* C = (double*)malloc(n * n * sizeof(double));

    
    if (A == NULL || B == NULL || C == NULL) {
        printf("Size: %5dx%5d [Run %2d/10] | SKIPPED (Insufficient RAM)\n", n, n, run_num);
        free(A); free(B); free(C);
        return;
    }

    
    for (int i = 0; i < n * n; i++) {
        A[i] = 1.0;
        B[i] = 2.0;
        C[i] = 0.0;
    }

    double start = get_time();

    // Call BLIS double-precision GEMM
    // Storage: Column-major (row stride = 1, column stride = n)
    bli_dgemm(BLIS_NO_TRANSPOSE, BLIS_NO_TRANSPOSE, 
              n, n, n, 
              &alpha, 
              A, 1, n,   
              B, 1, n, 
              &beta, 
              C, 1, n);

    double end = get_time();
    double elapsed = end - start;

    // Calculate performance: (2 * N^3 operations) / elapsed time
    double gflops = (2.0 * n * n * n) / (elapsed * 1e9);

    printf("Size: %5dx%5d [Run %2d/10] | Time: %6.2f sec | Performance: %7.2f GFLOPS\n", 
           n, n, run_num, elapsed, gflops);

    
    free(A);
    free(B);
    free(C);
}

int main() {
  
    int sizes[10] = {2000, 4000, 6000, 8000, 10000, 12000, 14000, 16000, 18000, 20000};

    printf("=================================================================================\n");
    printf("                          BLIS GEMM C BENCHMARK RESULTS                          \n");
    printf("=================================================================================\n");

   
    for (int i = 0; i < 10; i++) {
     
        for (int run = 1; run <= 10; run++) {
            benchmark_gemm(sizes[i], run);
        }
        printf("---------------------------------------------------------------------------------\n");
    }

    printf("=================================================================================\n");
    return 0;
}
