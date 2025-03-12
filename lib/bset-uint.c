/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at https://curl.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 * SPDX-License-Identifier: curl
 *
 ***************************************************************************/

#include "curl_setup.h"
#include "bset-uint.h"

/* The last 3 #include files should be in this order */
#include "curl_printf.h"
#include "curl_memory.h"
#include "memdebug.h"


static unsigned int bset_uint_pop_count(curl_uint64_t x)
{
  /* Compute the "Hamming Distance" between 'x' and 0,
   * which is the number of set bits in 'x'.
   * See: https://en.wikipedia.org/wiki/Hamming_weight */
  const curl_uint64_t m1  = 0x5555555555555555UL; /* 0101... */
  const curl_uint64_t m2  = 0x3333333333333333UL; /* 00110011... */
  const curl_uint64_t m4  = 0x0f0f0f0f0f0f0f0fUL; /* 00001111... */
   /* 1 + 256^1 + 256^2 + 256^3 + ... + 256^7 */
  const curl_uint64_t h01 = 0x0101010101010101UL;
  x -= (x >> 1) & m1;             /* replace every 2 bits with bits present */
  x = (x & m2) + ((x >> 2) & m2); /* replace every nibble with bits present */
  x = (x + (x >> 4)) & m4;        /* replace every byte with bits present */
  /* top 8 bits of x + (x<<8) + (x<<16) + (x<<24) + ... which makes the
   * top byte the sum of all individual 8 bytes, throw away the rest */
  return (unsigned int)((x * h01) >> 56);
}


static unsigned int bset_uint_trailing0s(curl_uint64_t x)
{
  /* divide and conquer to find the number of lower 0 bits */
  const curl_uint64_t ml32 = 0xFFFFFFFFUL; /* lower 32 bits */
  const curl_uint64_t ml16 = 0x0000FFFFUL; /* lower 16 bits */
  const curl_uint64_t ml8  = 0x000000FFUL; /* lower 8 bits */
  const curl_uint64_t ml4  = 0x0000000FUL; /* lower 4 bits */
  const curl_uint64_t ml2  = 0x00000003UL; /* lower 2 bits */
  unsigned int n;

  if(!x)
    return 64;
  n = 1;
  if(!(x & ml32)) {
    n = n + 32;
    x = x >> 32;
  }
  if(!(x & ml16)) {
    n = n + 16;
    x = x >> 16;
  }
  if(!(x & ml8)) {
    n = n + 8;
    x = x >> 8;
  }
  if(!(x & ml4)) {
    n = n + 4;
    x = x >> 4;
  }
  if(!(x & ml2)) {
    n = n + 2;
    x = x >> 2;
  }
  return n - (x & 1);
}


CURLcode Curl_bset_uint_init(struct bset_uint *bset, unsigned int nmax)
{
  memset(bset, 0, sizeof(*bset));
  return Curl_bset_uint_resize(bset, nmax);
}


CURLcode Curl_bset_uint_resize(struct bset_uint *bset, unsigned int nmax)
{
  unsigned int nslots = (nmax + 63) / 64;

  if(nslots != bset->nslots) {
    curl_uint64_t *slots = calloc(nslots, sizeof(curl_uint64_t));
    if(!slots)
      return CURLE_OUT_OF_MEMORY;

    memcpy(slots, bset->slots,
           (CURLMIN(nslots, bset->nslots) * sizeof(curl_uint64_t)));
    free(bset->slots);
    bset->slots = slots;
    bset->nslots = nslots;
  }
  return CURLE_OK;
}


void Curl_bset_uint_destroy(struct bset_uint *bset)
{
  free(bset->slots);
  memset(bset, 0, sizeof(*bset));
}


unsigned int Curl_bset_uint_capacity(struct bset_uint *bset)
{
  size_t capacity = bset->nslots * 64;
  DEBUGASSERT(capacity <= UINT_MAX);
  return (unsigned int)capacity;
}


unsigned int Curl_bset_uint_count(struct bset_uint *bset)
{
  unsigned int i;
  unsigned int n = 0;
  for(i = 0; i < bset->nslots; ++i) {
    if(bset->slots[i])
      n += bset_uint_pop_count(bset->slots[i]);
  }
  return n;
}


void Curl_bset_uint_clear(struct bset_uint *bset)
{
  memset(bset->slots, 0, bset->nslots * sizeof(curl_uint64_t));
}


bool Curl_bset_uint_add(struct bset_uint *bset, unsigned int i)
{
  unsigned int islot = i / 64;
  if(islot >= bset->nslots)
    return FALSE;
  bset->slots[islot] |= ((curl_uint64_t)1 << (i % 64));
  return TRUE;
}


void Curl_bset_uint_remove(struct bset_uint *bset, unsigned int i)
{
  size_t islot = i / 64;
  if(islot < bset->nslots)
    bset->slots[islot] &= ~((curl_uint64_t)1 << (i % 64));
}


bool Curl_bset_uint_contains(struct bset_uint *bset, unsigned int i)
{
  unsigned int islot = i / 64;
  if(islot >= bset->nslots)
    return FALSE;
  return (bset->slots[islot] & ((curl_uint64_t)1 << (i % 64))) != 0;
}


bool Curl_bset_uint_first(struct bset_uint *bset, unsigned int *pfirst)
{
  unsigned int i;
  for(i = 0; i < bset->nslots; ++i) {
    if(bset->slots[i]) {
      *pfirst = (i * 64) + bset_uint_trailing0s(bset->slots[i]);
      return TRUE;
    }
  }
  *pfirst = 0; /* give it a defined value even if it should not be used */
  return FALSE;
}

bool Curl_bset_uint_next(struct bset_uint *bset, unsigned int last,
                         unsigned int *pnext)
{
  unsigned int islot;
  curl_uint64_t x;

  ++last; /* look for number one higher than last */
  islot = last / 64; /* the slot this would be in */
  if(islot < bset->nslots) {
    /* shift aways the bits we already iterated in this slot */
    x = (bset->slots[islot] >> (last % 64));
    if(x) {
      /* more bits set, next is `last` + trailing0s of the shifted slot */
      *pnext = last + bset_uint_trailing0s(x);
      return TRUE;
    }
    /* no more bits set in the last slot, scan forward */
    for(islot = islot + 1; islot < bset->nslots; ++islot) {
      if(bset->slots[islot]) {
        *pnext = (islot * 64) + bset_uint_trailing0s(bset->slots[islot]);
        return TRUE;
      }
    }
  }
  *pnext = 0; /* give it a value, although it should not be used */
  return FALSE;
}
