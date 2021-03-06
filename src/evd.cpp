#include "evd.hpp"
#include "types.hpp"
#include "util.hpp"
#include <math.h>
#include <stdlib.h>

void MatMul(double*, double*, double*, int);

void evd_classic(struct matrix_t Data_matr, struct vector_t Eigen_values,
                 int epoch) {

    double* A = Data_matr.ptr;
    const int m = Data_matr.rows;

    double* E = Eigen_values.ptr;
	int is_not_diagonal = 0;

    // Build the auxiliary matrices

    double *P, *P_t, *temp;
    P = (double*)malloc(sizeof(double) * m * m);
    P_t = (double*)malloc(sizeof(double) * m * m);
    temp = (double*)malloc(sizeof(double) * m * m);

    for (int ep = 1; ep <= epoch; ep++) {

        double val = 0.0;
        int i_max, j_max;
        double alpha, beta, cos_t, sin_t;

        identity(P, m);

        // Find the larget non-diagonal element in the
        // upper triangular matrix

        for (int i = 0; i < m; i++) {
            for (int j = i + 1; j < m; j++) {
                if (fabs(A[i * m + j]) > val) {
                    i_max = i;
                    j_max = j;
                    val = A[i * m + j];
					is_not_diagonal = 1;
                }
            }
        }

		if(! is_not_diagonal)
			break;

        // Compute cos_t and sin_t for the rotation matrix

        alpha = 2.0 * sign(A[i_max * m + i_max] - A[j_max * m + j_max]) *
                A[i_max * m + j_max];
        beta = fabs(A[i_max * m + i_max] - A[j_max * m + j_max]);
        cos_t = sqrt(0.5 * (1 + beta / sqrt(alpha * alpha + beta * beta)));
        // sin_t = (1 / 2*cos_t) * (alpha / sqrt(alpha*alpha + beta*beta));
        sin_t = sign(alpha) * sqrt(1 - cos_t * cos_t);

        // Initialize the rotation parameters in the identity matrix

        P[i_max * m + i_max] = P[j_max * m + j_max] = cos_t;
        P[j_max * m + i_max] = sin_t;
        P[i_max * m + j_max] = -1 * sin_t;

        // Perform the operation A(i) = P_t * A(i-1) * P
        // corresponding to Jacobi iteration i

        transpose(P, P_t, m);
        MatMul(temp, P_t, A, m);
        MatMul(A, temp, P, m);
    }

    // Store the generated eigen values in the vector
    for (int i = 0; i < m; i++) {
        E[i] = A[i * m + i];
    }
}

void evd_classic_tol(struct matrix_t Xmat, struct vector_t evec, struct matrix_t Qmat, double tol) {
    const int n = Xmat.cols;
    double* X = Xmat.ptr;
    double* e = evec.ptr;
    double* Q = Qmat.ptr;
    //A=QtXQ
    double *A = (double*)malloc(sizeof(double)*n*n);

    double offA=0, abs_a, eps, c, s;
    int p, q;

    for(int i = 0; i < n; ++i){
        for(int j = 0; j<n; ++j){
            A[n*i+j] = X[n*i+j];
        }
    }
    for(int i = 0; i < n; ++i){
        Q[n*i+i] = 1.0;
    }
    for(int i = 0; i < n; ++i){
        for(int j = i+1; j<n; ++j){
            double a_ij = A[n*i+j];
            offA+=2*a_ij*a_ij;
        }
    }
    for(int i = 0; i < n; ++i){
        for(int j = i; j<n; ++j){
            double a_ij = A[n*i+j];
            if(i == j){
                eps+=a_ij*a_ij;
            } else {
                eps+=2*a_ij*a_ij;
            }
        }
    }
    eps = tol*tol*eps;

    while(offA > eps){

        abs_a = 0.0;
        for(int i = 0; i < n; ++i){
            for(int j = i+1; j<n; ++j){
                double abs_ij = abs(A[i*n+j]);
                if(abs_ij > abs_a){
                    abs_a = abs_ij;
                    p = i;
                    q = j;
                }
            }
        }

        sym_jacobi_coeffs(A[p*n+p],A[p*n+q],A[q*n+q],&c,&s);

        double A_ip, A_iq;
        for(int i = 0; i < n; ++i){
            Q[n*i+p] = c*Q[n*i+p]-s*Q[n*i+q];
            Q[n*i+q] = s*Q[n*i+p]+c*Q[n*i+q];

            A_ip = A[n*i+p];
            A_iq = A[n*i+q];

            A[n*i+p] = c * A_ip - s * A_iq;
            A[n*i+q] = s * A_ip + c * A_iq;

        }
        for(int i = 0; i < n; ++i){
            A_ip = A[n*p+i];
            A_iq = A[n*q+i];

            A[n*p+i] = c * A_ip - s * A_iq;
            A[n*q+i] = s * A_ip + c * A_iq;
        }
        offA = 0;
        for(int i = 0; i < n; ++i){
            for(int j = i+1; j < n; ++j){
                double a_ij= A[n*i+j];
                offA+=2*a_ij*a_ij;
            }
        }
    }
    for(int i = 0; i < n; ++i){
        e[i]=A[n*i+i];
    }
    reorder_decomposition(evec, &Qmat, 1, greater);
    free(A);

}
void MatMul(double* P, double* Q, double* R, int n) {
    double sum = 0.0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            for (int k = 0; k < n; k++) {
                sum += Q[i * n + k] * R[k * n + j];
            }
            P[i * n + j] = sum;
            sum = 0.0;
        }
    }
}
