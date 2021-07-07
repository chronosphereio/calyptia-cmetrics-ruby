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

#include "cmetrics_c.h"

VALUE rb_mCMetrics;

static VALUE
rb_cmetrics_library_version(VALUE self)
{
    return rb_str_new2(cmt_version());
}

void Init_cmetrics(void)
{
    rb_mCMetrics = rb_define_module("CMetrics");

    rb_define_singleton_method(rb_mCMetrics, "library_version", rb_cmetrics_library_version , 0);

    Init_cmetrics_counter(rb_mCMetrics);
    Init_cmetrics_gauge(rb_mCMetrics);
    Init_cmetrics_serde(rb_mCMetrics);
}
