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

#include "content_encoding.h"
#include "http_chunks.h"
#include "request.h"
#include "doh.h"
#include "url.h"

/* The last 3 #include files should be in this order */
#include "curl_printf.h"
#include "curl_memory.h"
#include "memdebug.h"

static void dl_reset(struct Curl_easy *data)
{
  struct SingleRequest *req = &data->req;
  struct dynbuf hd_save;

  /* Cleanup writer stack */
  Curl_unencode_cleanup(data);

  Curl_safefree(data->req.dl.location);
  Curl_dyn_free(&data->req.dl.headerb);
#ifndef CURL_DISABLE_HTTP
  Curl_httpchunk_free(data);
#endif
  hd_save = data->req.dl.headerb;
  memset(&req->dl, 0, sizeof(req->dl));
  data->req.dl.headerb = hd_save;
  req->dl.size = req->dl.nmax = -1;
  req->dl.parse_headers = TRUE; /* assume header */
}

static void ul_reset(struct Curl_easy *data)
{
  struct SingleRequest *req = &data->req;
  struct dynbuf tr_save;

#ifndef CURL_DISABLE_HTTP
  tr_save = data->req.ul.trailers_buf;
  Curl_dyn_free(&data->req.ul.trailers_buf);
#endif
  memset(&req->ul, 0, sizeof(req->ul));
#ifndef CURL_DISABLE_HTTP
  data->req.ul.trailers_buf = tr_save;
#endif
  req->ul.size = -1;
}

void Curl_req_reset(struct Curl_easy *data)
{
  struct SingleRequest *req = &data->req;

  dl_reset(data);
  ul_reset(data);
  req->no_body = data->set.opt_no_body;

  Curl_safefree(req->p.http);
  Curl_safefree(data->req.newurl);

  Curl_doh_reset(data);
}

void Curl_req_free(struct Curl_easy *data)
{
  Curl_req_reset(data);
  Curl_dyn_free(&data->req.dl.headerb);
#ifndef CURL_DISABLE_HTTP
  Curl_dyn_free(&data->req.ul.trailers_buf);
#endif
  Curl_doh_free(data);
}

CURLcode Curl_req_init(struct Curl_easy *data)
{
  (void)data;
  Curl_dyn_init(&data->req.dl.headerb, CURL_MAX_HTTP_HEADER);
#ifndef CURL_DISABLE_HTTP
  Curl_dyn_init(&data->req.ul.trailers_buf, DYN_TRAILERS);
#endif
  return CURLE_OK;
}

CURLcode Curl_req_start(struct Curl_easy *data)
{
  struct SingleRequest *req = &data->req;

  dl_reset(data);
  ul_reset(data);
  req->start = Curl_now(); /* start time */

  return CURLE_OK;
}

