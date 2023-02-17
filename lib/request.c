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

#include "request.h"
#include "doh.h"
#include "url.h"

/* The last 3 #include files should be in this order */
#include "curl_printf.h"
#include "curl_memory.h"
#include "memdebug.h"

void Curl_req_free_state(struct SingleRequest *req)
{
  Curl_safefree(req->p.http);
  Curl_safefree(req->newurl);

#ifndef CURL_DISABLE_DOH
  if(req->doh) {
    Curl_close(&req->doh->probe[0].easy);
    Curl_close(&req->doh->probe[1].easy);
  }
#endif
}

void Curl_req_close(struct SingleRequest *req)
{
  Curl_req_free_state(req);
  /* Cleanup possible redirect junk */
  Curl_safefree(req->newurl);

#ifndef CURL_DISABLE_DOH
  if(req->doh) {
    Curl_dyn_free(&req->doh->probe[0].serverdoh);
    Curl_dyn_free(&req->doh->probe[1].serverdoh);
    curl_slist_free_all(req->doh->headers);
    Curl_safefree(req->doh);
  }
#endif
}

void Curl_req_init(struct SingleRequest *req, const struct UserDefined *set)
{
  Curl_req_free_state(req);
  memset(req, 0, sizeof(*req));
  req->size = req->maxdownload = -1;
  req->no_body = set->opt_no_body;
}

CURLcode Curl_req_start(struct SingleRequest *req)
{
  req->start = Curl_now(); /* start time */
  req->now = req->start;
  req->header = TRUE; /* assume header */
  req->bytecount = 0;
  req->ignorebody = FALSE;

  return CURLE_OK;
}
