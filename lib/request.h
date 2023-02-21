#ifndef HEADER_CURL_REQUEST_H
#define HEADER_CURL_REQUEST_H
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

/* This file is for lib internal stuff */

#include "curl_setup.h"

#include "dynbuf.h"
#include "timeval.h"

struct UserDefined;

enum expect100 {
  EXP100_SEND_DATA,           /* enough waiting, just send the body now */
  EXP100_AWAITING_CONTINUE,   /* waiting for the 100 Continue header */
  EXP100_SENDING_REQUEST,     /* still sending the request but will wait for
                                 the 100 header once done with the request */
  EXP100_FAILED               /* used on 417 Expectation Failed */
};

enum upgrade101 {
  UPGR101_INIT,               /* default state */
  UPGR101_WS,                 /* upgrade to WebSockets requested */
  UPGR101_H2,                 /* upgrade to HTTP/2 requested */
  UPGR101_RECEIVED,           /* 101 response received */
  UPGR101_WORKING             /* talking upgraded protocol */
};

enum badheader {
  HEADER_NORMAL,              /* no bad header at all */
  HEADER_PARTHEADER,          /* part of the chunk is a bad header, the rest
                               * is normal data */
  HEADER_ALLBAD               /* all was believed to be header.
                               * the header was deemed bad and will be
                               * written as body */
};

/**
 * Information for a specific Request-Response pair.
 *
 * A single URL transfer can involve a series of requests, e.g. when
 * redirects happen.
 */
struct Curl_download {
  curl_off_t size;        /* -1 if unknown at this point */
  curl_off_t nmax;        /* maximum amount of data bytes to fetch,
                             -1 means unlimited */
  curl_off_t nread;       /* total number of content bytes read */
  curl_off_t nhd_bytes;   /* total numbe rof header bytes read */
  curl_off_t nhd_bytes_deduct; /* this amount of bytes doesn't count when we
                             check if anything has been transferred at
                             the end of a connection. We use this
                             counter to make only a 100 reply (without a
                             following second response code) result in a
                             CURLE_GOT_NOTHING error code */
  curl_off_t offset;      /* net content offset of next read. Set by
                             Content-Range: header to detect resumption */
  struct dynbuf headerb;  /* buffer to store headers in */
  char *buf_cur;          /* current position within buf */
  char *location;         /* This points to an allocated version of the
                             Location: header data */
  struct contenc_writer *writer; /* Content unencoding stack.
                             See sec 3.5, RFC2616. */
  struct curltime start100; /* time stamp to wait for the 100 code from */

  time_t last_modified;   /* Time of last modification to transfered document,
                             if known. */
  long bodywrites;        /* how often we have written body data */
  int nhd_lines;          /* counts header lines to detect the first one */
  int httpcode;           /* HTTP or RTSP status code */
  unsigned char writer_depth; /* writer stack depth. */

  enum expect100 exp100;  /* expect 100 continue state */
  enum badheader badheader;     /* header treatment */
  enum upgrade101 upgr101; /* 101 upgrade state */

  BIT(parse_headers);     /* incoming bytes are HTTP headers */
  BIT(bodyless);          /* HTTP response status code is between 100 and 199,
                             204 or 304 */
  BIT(ignore_cl);         /* ignore content-length */
  BIT(content_range);     /* set TRUE if Content-Range: was found */
  BIT(ignore_body);       /* we read a response-body but we ignore it! */
  BIT(chunky);            /* if set, this is a chunked transfer-encoding */
};

struct Curl_upload {
  curl_off_t size;        /* -1 if unknown at this point */
  curl_off_t nwritten;    /* number of upload bytes written */
  curl_off_t offset;      /* net content offset of next write */
  curl_off_t nhd_pending; /* this many bytes left to send is actually
                              header and not body */
  char *buf;              /* data available for upload */
  ssize_t buf_len;        /* how much data is available in `buf` */
  BIT(done);              /* set to TRUE when doing chunked transfer-encoding
                             upload and we're uploading the last chunk */
  BIT(forbid_chunk);      /* used only to explicitly forbid chunk-upload for
                             specific upload buffers. See readmoredata() in
                             http.c for details. */
  BIT(chunky);            /* set TRUE if we are doing chunked transfer-encoding
                             on upload */
};

struct SingleRequest {
  struct Curl_download dl;
  struct Curl_upload ul;
  struct curltime start;  /* transfer started at this time */
  int keepon;             /* what the transfer needs to keep on doing */
  char *newurl;           /* Set to the new URL to use when a redirect or a
                             retry is wanted */
  /* Allocated protocol-specific data. Each protocol handler makes sure this
     points to data it needs. */
  union {
    struct FILEPROTO *file;
    struct FTP *ftp;
    struct HTTP *http;
    struct IMAP *imap;
    struct ldapreqinfo *ldap;
    struct MQTT *mqtt;
    struct POP3 *pop3;
    struct RTSP *rtsp;
    struct smb_request *smb;
    struct SMTP *smtp;
    struct SSHPROTO *ssh;
    struct TELNET *telnet;
  } p;
#ifndef CURL_DISABLE_DOH
  struct dohdata *doh; /* DoH specific data for this request */
#endif
  unsigned char setcookies;
  BIT(no_body);       /* transfer is told to not deliver a body */
};

/**
 * Run one-time initialization of data->req after data
 * has been callocated.
 */
CURLcode Curl_req_init(struct Curl_easy *data);

/**
 * reset all intermittent data.
 */
void Curl_req_reset(struct Curl_easy *data);

/**
 * Request is starting, perform last initialization before use.
 */
CURLcode Curl_req_start(struct Curl_easy *data);

/**
 * free all data.
 */
void Curl_req_free(struct Curl_easy *data);


#endif /* HEADER_CURL_REQUEST_H */
