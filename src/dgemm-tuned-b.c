const char* dgemm_desc = "Simple blocked dgemm.";

#if !defined(BLOCK_SIZE)
#define BLOCK_SIZE 64
#endif

#define min(a,b) (((a)<(b))?(a):(b))

/* This version includes L1 cache oprimization, changing the order of the loops and copy optimizations.
*/
/* This auxiliary subroutine performs a smaller dgemm operation
 *  C := C + A * B
 * where C is M-by-N, A is M-by-K, and B is K-by-N. */
static inline void do_block (int lda, int M, int N, int K, double* restrict A, double* restrict B, double* restrict C)
{
  for (int k = 0; k < K; ++k)
    for (int j = 0; j < N; ++j) 
    {
      double b_kj = B[k+j*lda];      
      for (int i = 0; i < M-3; i+=4)
	{
        C[i+j*lda] += A[i+k*lda] * b_kj;
	C[i+j*lda+1] += A[i+k*lda+1] * b_kj;
	C[i+j*lda+2] += A[i+k*lda+2] * b_kj;
	C[i+j*lda+3] += A[i+k*lda+3] * b_kj;
	}
      if ( M%4 !=0 )
	{
	for (int i = (M-M%4); i < M; i++)
             C[i+j*lda] += A[i+k*lda] * b_kj;
	}
     }
}

/* This routine performs a dgemm operation
 *  C := C + A * B
 * where A, B, and C are lda-by-lda matrices stored in column-major format. 
 * On exit, A and B maintain their input values. */  
void square_dgemm (int lda, double* restrict A, double* restrict B, double* restrict C)
{
  /* For each block-row of A */ 
  for (int i = 0; i < lda; i += BLOCK_SIZE)
    /* For each block-column of B */
    for (int j = 0; j < lda; j += BLOCK_SIZE)
      /* Accumulate block dgemms into block of C */
      for (int k = 0; k < lda; k += BLOCK_SIZE)
      {
        /* Correct block dimensions if block "goes off edge of" the matrix */
        int M = min (BLOCK_SIZE, lda-i);
        int N = min (BLOCK_SIZE, lda-j);
        int K = min (BLOCK_SIZE, lda-k);

        /* Perform individual block dgemm */
        do_block(lda, M, N, K, A + i + k*lda, B + k + j*lda, C + i + j*lda);
      }
}
