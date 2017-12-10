#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

/* 
 * y'' = exp(-y)
 * y(0) = 1, y(1) = b 
 */

#define F(y) (exp(-(y)))
#define dF(y) (-exp(-(y)))
#define X(i) ((x0) + (i)*(h))

static double *y;
static double *A;
static double *B;
static double *C;
static double *D;
static double *d;

static int alloc_arrays(size_t size)
{
    if (!(y = (double *)malloc(size)))
        goto out_y;
    if (!(A = (double *)malloc(size)))
        goto out_A;
    if (!(B = (double *)malloc(size)))
        goto out_B;
    if (!(C = (double *)malloc(size)))
        goto out_C;
    if (!(D = (double *)malloc(size)))
        goto out_D;
    if (!(d = (double *)malloc(size)))
        goto out_d;

    return 0;

out_d:
    free(d);
out_D:
    free(D);
out_C:
    free(C);
out_B:
    free(B);
out_A:
    free(A);
out_y:
    free(y);

    return -1;
}

static void free_arrays()
{
    free(d);
    free(D);
    free(C);
    free(B);
    free(A);
    free(y);
}

static void reduction(int N, double h)
{
    const double p = h*h/12.0, q = h*h*5.0/6.0;
    
    A[0] = 0;
    B[0] = 1.0;
    C[0] = 0;
    D[0] = 0;
    
    A[N-1] = 0;
    B[N-1] = 1.0;
    C[N-1] = 0;
    D[N-1] = 0;
    
    #pragma omp parallel for
    for (int i = 1; i < N - 1; ++i) {
        A[i] = 1 - dF(y[i - 1])*p;
        B[i] = -2 - dF(y[i])*q; 
        C[i] = 1 - dF(y[i + 1])*p;
        D[i] = (y[i - 1]*dF(y[i - 1]) + F(y[i - 1]))*p + (y[i]*dF(y[i]) + F(y[i]))*q +
               (y[i + 1]*dF(y[i + 1]) + F(y[i + 1]))*p - (y[i - 1] - 2*y[i] + y[i + 1]);
    }
    
    int stride = 1;

    for (int n = N, low = 2; n > 1; n /= 2, low *= 2, stride *= 2) {
        #pragma omp parallel for
        for (int i = low - 1; i < N; i += stride * 2) {
            double alpha = -A[i]/B[i - stride];
            double beta  = -C[i]/B[i + stride];
            
            A[i] = alpha*A[i - stride]; 
            B[i] = B[i] + alpha*C[i - stride] + beta*A[i + stride];
            C[i] = beta*C[i + stride];
            D[i] = D[i] + alpha*D[i - stride] + beta*D[i + stride];
        }   
    }
    
    d[0] = 0;
    d[N/2] = D[N/2]/B[N/2];
    d[N - 1] = 0;
    
    for (stride /= 2; stride >= 1; stride /= 2) {
        #pragma omp parallel for
        for (int i = stride - 1; i < N; i += stride * 2)
            d[i] = (D[i] - ((i - stride > 0) ? (A[i] * d[i - stride]) : 0.)
                             - ((i + stride < N) ? (C[i] * d[i + stride]) : 0.))/B[i];
    }
}

int main(int argc, char** argv)
{
    double b0 = 0, b1 = 1.7, db = 0.1;
    double x0 = 0, xN = 1.0;
    double epsilon = 0.000001;
    int N = 3000;
    int err = 0;
    
    double h = fabs(xN - x0) / N;

    if (alloc_arrays((N + 1)*sizeof(double))) {
        return -1;
    }

    FILE* out_file = fopen("out.txt", "w");
    if (!out_file) {
        err = -1;
        goto out_arrays;
    }

    fprintf(out_file, "plot ");

    for (double b = b0; b < b1; b += db)
        fprintf(out_file, "'-' pt 7 ps 0.25 title 'b = %2.2lf', ", b);
    fprintf(out_file, "'-' pt 7 ps 0.25 title 'b = %2.2lf'\n", b1);
    
    for (double b = b0; b <= b1 + epsilon; b += db) {
        fprintf(stderr, "b = %lf\n", b);
        
        #pragma omp parallel for
        for (int i = 0; i < N + 1; ++i)
            y[i] = 1.0 + X(i)*(b - 1.0);

        for (;;) {
            double max_delta = -1.;
            
            reduction(N + 1, h);
            
            #pragma omp parallel for reduction(max:max_delta)
            for (int i = 1; i < N; ++i) {
                y[i] += d[i];
                if (fabs(d[i]) > max_delta)
                    max_delta = fabs(d[i]);
            }
            
            if (max_delta < epsilon)
                break;
        }
        
        for (int i = 0; i < N + 1; ++i)
            fprintf(out_file, "%lf %lf\n", X(i), y[i]);
        
        fprintf(out_file, "e\n");
    }

    fclose(out_file);
    
out_arrays:
    free_arrays();

    return err;
};
