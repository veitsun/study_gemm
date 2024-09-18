#include "../include/func_c_girl.h"

void func_c_girl::solveProblem(int M, int N, int K, float alpha, float *A, float *B, float beta, float *C){
        for ( int i = 0; i < M; i ++ ) {
                for (int j = 0; j < N; j++ ) {
			float sum = 0.0;
                        for (int k = 0; k < K; k ++) {
                                // C[i][j] = alpha * A[i][k] * B[k][j] + beta * C[i][j];
				sum += A[i * K + k] * B[k * N + j];
                        }
			sum = sum * alpha + beta * C[i * N + j];
                }
        }
}

