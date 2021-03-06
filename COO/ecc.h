#ifndef ECC_H
#define ECC_H

#include <stdio.h>
#include <stdint.h>

// 128-bit matrix element
// Bits  0 to  31 are the colum index
// Bits 32 to  63 are the row index
// Bits 64 to 127 are the floating point value
struct coo_element
{
  uint32_t col;
  uint32_t row;
  double value;
};

#define ECC7_P1_0 0x80AAAD5B
#define ECC7_P1_1 0x55555556
#define ECC7_P1_2 0xAAAAAAAB
#define ECC7_P1_3 0xAAAAAAAA

#define ECC7_P2_0 0x4033366D
#define ECC7_P2_1 0x9999999B
#define ECC7_P2_2 0xCCCCCCCD
#define ECC7_P2_3 0xCCCCCCCC

#define ECC7_P3_0 0x20C3C78E
#define ECC7_P3_1 0xE1E1E1E3
#define ECC7_P3_2 0xF0F0F0F1
#define ECC7_P3_3 0xF0F0F0F0

#define ECC7_P4_0 0x10FC07F0
#define ECC7_P4_1 0xFE01FE03
#define ECC7_P4_2 0xFF00FF01
#define ECC7_P4_3 0xFF00FF00

#define ECC7_P5_0 0x08FFF800
#define ECC7_P5_1 0xFFFE0003
#define ECC7_P5_2 0xFFFF0001
#define ECC7_P5_3 0xFFFF0000

#define ECC7_P6_0 0x04000000
#define ECC7_P6_1 0xFFFFFFFC
#define ECC7_P6_2 0x00000001
#define ECC7_P6_3 0xFFFFFFFF

#define ECC7_P7_0 0x02000000
#define ECC7_P7_1 0x00000000
#define ECC7_P7_2 0xFFFFFFFE
#define ECC7_P7_3 0xFFFFFFFF

// This function will generate/check the 7 parity bits for the given matrix
// element, with the parity bits stored in the high order bits of the column
// index.
//
// This will return a 32-bit integer where the high 7 bits are the generated
// parity bits.
//
// To check a matrix element for errors, simply use this function again, and
// the returned value will be the error 'syndrome' which will be non-zero if
// an error occured.
static inline uint32_t ecc_compute_col8(coo_element element)
{
  uint32_t *data = (uint32_t*)&element;

  uint32_t result = 0;

  uint32_t p;

  p = (data[0] & ECC7_P1_0) ^ (data[1] & ECC7_P1_1) ^
      (data[2] & ECC7_P1_2) ^ (data[3] & ECC7_P1_3);
  result |= __builtin_parity(p) << 31U;

  p = (data[0] & ECC7_P2_0) ^ (data[1] & ECC7_P2_1) ^
      (data[2] & ECC7_P2_2) ^ (data[3] & ECC7_P2_3);
  result |= __builtin_parity(p) << 30U;

  p = (data[0] & ECC7_P3_0) ^ (data[1] & ECC7_P3_1) ^
      (data[2] & ECC7_P3_2) ^ (data[3] & ECC7_P3_3);
  result |= __builtin_parity(p) << 29U;

  p = (data[0] & ECC7_P4_0) ^ (data[1] & ECC7_P4_1) ^
      (data[2] & ECC7_P4_2) ^ (data[3] & ECC7_P4_3);
  result |= __builtin_parity(p) << 28U;

  p = (data[0] & ECC7_P5_0) ^ (data[1] & ECC7_P5_1) ^
      (data[2] & ECC7_P5_2) ^ (data[3] & ECC7_P5_3);
  result |= __builtin_parity(p) << 27U;

  p = (data[0] & ECC7_P6_0) ^ (data[1] & ECC7_P6_1) ^
      (data[2] & ECC7_P6_2) ^ (data[3] & ECC7_P6_3);
  result |= __builtin_parity(p) << 26U;

  p = (data[0] & ECC7_P7_0) ^ (data[1] & ECC7_P7_1) ^
      (data[2] & ECC7_P7_2) ^ (data[3] & ECC7_P7_3);
  result |= __builtin_parity(p) << 25U;

  return result;
}

static inline int is_power_of_2(uint32_t x)
{
  return ((x != 0) && !(x & (x - 1)));
}

// Compute the overall parity of a 128-bit matrix element
static inline uint32_t ecc_compute_overall_parity(coo_element element)
{
  uint32_t *data = (uint32_t*)&element;
  return __builtin_parity(data[0] ^ data[1] ^ data[2] ^ data[3]);
}

// This function will use the error 'syndrome' generated from a 7-bit parity
// check to determine the index of the bit that has been flipped
static inline uint32_t ecc_get_flipped_bit_col8(uint32_t syndrome)
{
  // Compute position of flipped bit
  uint32_t hamm_bit = 0;
  for (int p = 1; p <= 7; p++)
  {
    if ((syndrome >> (32-p)) & 0x1)
      hamm_bit += 0x1U<<(p-1);
  }

  // Map to actual data bit position
  uint32_t data_bit = hamm_bit - (32-__builtin_clz(hamm_bit)) - 1;
  if (is_power_of_2(hamm_bit))
    data_bit = __builtin_clz(hamm_bit);
  else if (data_bit >= 24U)
    data_bit += 8;

  return data_bit;
}

/*
void gen_ecc7_masks()
{
  for (uint32_t p = 1; p <= 7; p++)
  {
    uint32_t x = 3;
    for (int w = 0; w < 4; w++)
    {
      uint32_t mask = 0;
      for (uint32_t b = 0; b < 32; b++)
      {
        if (is_power_of_2(x))
          x++;

        uint32_t bit = w*32 + b;
        if (bit >= (32-8) && bit < 32)
        {
          if ((32-bit) == p)
            mask |= 0x1 << b;
        }
        else
        {
          if (x & (0x1<<(p-1)))
          {
            mask |= 0x1 << b;
          }
          x++;
        }
      }
      printf("#define ECC7_P%d_%d 0x%08X\n", p, w, mask);
    }
    printf("\n");
  }
}
*/

#endif // ECC_H
