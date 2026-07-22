// LowLevel.cpp

#include "Core/System/LowLevel.h"
#include "immintrin.h"

/**
 * SIMD Naming:
 *  __mm256 / _mm512 -> Bucket Size;
 * 
 * _cmp : Compare operation
 * 
 * eq: Equal condition
 * 
 * epi: Extended Packed Integer -> Treat Data as integers
 * 
 * 32: Chunk size -> in 256 register means 8 chunks of 32 bits each to manipulate
 */

int32 LowLevel::FindSimdAvx2(int32 target, int32 *pBlock) {
  __m256i vTarget = _mm256_set1_epi32(target);

  __m256i vData = _mm256_loadu_si256((__m256i*)pBlock);

  __m256i vCmp = _mm256_cmpeq_epi32(vData, vTarget);

  return _mm256_movemask_epi8(vCmp);
}