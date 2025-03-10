#ifndef HEADER_CURL_BSET_UINT_H
#define HEADER_CURL_BSET_UINT_H
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

#include <curl/curl.h>

struct bset_uint {
  curl_uint64_t *slots;
  unsigned int nslots;
};

/* Initialize the bitset for unsigned ints from 0 to `nmax`,
 * which rounds up `nmax` to the next multiple of 64. */
CURLcode Curl_bset_uint_init(struct bset_uint *bset, unsigned int nmax);

/* Resize the bitset capacity to hold numbers from 0 to `nmax`,
 * which rounds up `nmax` to the next multiple of 64. */
CURLcode Curl_bset_uint_resize(struct bset_uint *bset, unsigned int nmax);

/* Destroy the bitset, freeing all resources. */
void Curl_bset_uint_destroy(struct bset_uint *bset);

/* Get the bitset capacity, e.g. can hold numbers from 0 to capacity - 1. */
unsigned int Curl_bset_uint_capacity(struct bset_uint *bset);

/* Get the cardinality of the bitset, e.g. numbers present in the set. */
unsigned int Curl_bset_uint_count(struct bset_uint *bset);

/* Clear the bitset, making it empty. */
void Curl_bset_uint_clear(struct bset_uint *bset);

/* Add the number `i` to the bitset. Return FALSE if the number is
 * outside the set's capacity.
 * Numbers can be added more than once, without making a difference. */
bool Curl_bset_uint_add(struct bset_uint *bset, unsigned int i);

/* Remove the number `i` from the bitset. */
void Curl_bset_uint_remove(struct bset_uint *bset, unsigned int i);

/* Return TRUE of the bitset contains number `i`. */
bool Curl_bset_uint_contains(struct bset_uint *bset, unsigned int i);

/* Get the first number in the bitset, e.g. the smallest.
 * Returns FALSE when the bitset is empty. */
bool Curl_bset_uint_first(struct bset_uint *bset, unsigned int *pfirst);

/* Get the next number in the bitset, following `last` in natural order.
 * Put another way, this is the smallest number greater than `last` in
 * the bitset. `last` does not have to be present in the set.
 *
 * Returns FALSE when no such number is in the set.
 *
 * This allows to iterate the set while being modified:
 * - added numbers higher than 'last' will be picked up by the iteration.
 * - added numbers lower than 'last' will not show up.
 * - removed numbers lower or equal to 'last' will not show up.
 * - remove numbers higher than 'last' will not be visited. */
bool Curl_bset_uint_next(struct bset_uint *bset, unsigned int last,
                         unsigned int *pnext);

#endif /* HEADER_CURL_BSET_UINT_H */
