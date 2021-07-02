/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  CMetrics-Ruby
 *  =============
 *  Copyright 2021 Hiroshi Hatake <hatake@calyptia.com>
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef _CMETRICS_C_H_
#define _CMETRICS_C_H_

#ifdef HAVE_GMTIME_S
# define CMT_HAVE_GMTIME_S 1
#endif

#include <ruby.h>
#include <ruby/encoding.h>
/* Include cmetrics header */
#include <cmetrics/cmetrics.h>
#include <cmetrics/cmt_counter.h>
#include <cmetrics/cmt_gauge.h>
#include <cmetrics/cmt_encode_influx.h>
#include <cmetrics/cmt_encode_prometheus.h>
#include <cmetrics/cmt_encode_msgpack.h>
#include <cmetrics/cmt_encode_text.h>
#include <cmetrics/cmt_decode_msgpack.h>

struct CMetricsCounter {
    struct cmt *instance;
    struct cmt_counter *counter;
};

struct CMetricsGauge {
    struct cmt *instance;
    struct cmt_gauge *gauge;
};

struct CMetricsSerde {
    struct cmt *instance;
};

void Init_cmetrics_counter(VALUE rb_mCMetrics);
void Init_cmetrics_gauge(VALUE rb_mCMetrics);
void Init_cmetrics_serde(VALUE rb_mCMetrics);

#endif // _CMETRICS_C_H
