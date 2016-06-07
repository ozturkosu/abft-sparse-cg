#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

// 128-bit matrix element
// Bits  0 to  31 are the colum index
// Bits 32 to  63 are the row index
// Bits 64 to 127 are the floating point value
typedef struct
{
  uint32_t col;
  uint32_t row;
  double value;
} matrix_entry;

typedef struct
{
  double value;
  uint32_t column;
} __attribute__((packed)) csr_colval;

typedef struct
{
  unsigned N;
  unsigned nnz;
  uint32_t *cols;
  uint32_t *rows;
  double   *values;
} sparse_matrix;


// Initialize ECC for a sparse matrix
// Defined in the relevant spmv-*.c file
void init_matrix_ecc(sparse_matrix M);

// Sparse matrix-vector product
// Defined in the relevant spmv-*.c file
void spmv(sparse_matrix matrix, double *vector, double *result, unsigned N);

// Flip a specific bit in a matrix element
void flip_bit(csr_colval* element, uint32_t bit);

// Load a sparse matrix from a matrix-market format file
sparse_matrix load_sparse_matrix(const char *matrix_file, int num_blocks);

#endif // COMMON_H
