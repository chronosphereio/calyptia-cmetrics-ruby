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

VALUE rb_cSerde;

static void serde_free(void* ptr);

static const rb_data_type_t rb_cmetrics_serde_type = { "cmetrics/serde",
                                                         {
                                                             0,
                                                             serde_free,
                                                             0,
                                                         },
                                                         NULL,
                                                         NULL,
                                                         RUBY_TYPED_FREE_IMMEDIATELY };


static void
serde_free(void* ptr)
{
    struct CMetricsSerde* cmetricsSerde = (struct CMetricsSerde*)ptr;

    if (!cmetricsSerde) {
        if (!cmetricsSerde->instance) {
            cmt_destroy(cmetricsSerde->instance);
        }
    }

    xfree(ptr);
}

static VALUE
rb_cmetrics_serde_alloc(VALUE klass)
{
    VALUE obj;
    struct CMetricsSerde* cmetricsSerde;
    obj = TypedData_Make_Struct(
            klass, struct CMetricsSerde, &rb_cmetrics_serde_type, cmetricsSerde);
    return obj;
}

/*
 * Initailize Serde class.
 *
 * @return [Serde]
 *
 */
static VALUE
rb_cmetrics_serde_initialize(VALUE self)
{
    struct CMetricsSerde* cmetricsSerde;

    TypedData_Get_Struct(
            self, struct CMetricsSerde, &rb_cmetrics_serde_type, cmetricsSerde);

    cmt_initialize();

    cmetricsSerde->instance = NULL;

    return Qnil;
}

static VALUE
rb_cmetrics_serde_from_msgpack(VALUE self, VALUE rb_msgpack_buffer)
{
    struct CMetricsSerde* cmetricsSerde;
    char *buffer;
    size_t buffer_size;
    int ret = 0;
    struct cmt *cmt = NULL;

    TypedData_Get_Struct(
            self, struct CMetricsSerde, &rb_cmetrics_serde_type, cmetricsSerde);

    ret = cmt_decode_msgpack(&cmt, StringValuePtr(rb_msgpack_buffer), RSTRING_LEN(rb_msgpack_buffer));


    if (ret == 0) {
        cmetricsSerde->instance = cmt;
        return Qtrue;
    } else {
        return Qnil;
    }
}

static VALUE
rb_cmetrics_serde_to_prometheus(VALUE self)
{
    struct CMetricsSerde* cmetricsSerde;
    cmt_sds_t prom;
    VALUE str;

    TypedData_Get_Struct(
            self, struct CMetricsSerde, &rb_cmetrics_serde_type, cmetricsSerde);

    prom = cmt_encode_prometheus_create(cmetricsSerde->instance, CMT_TRUE);

    str = rb_str_new2(prom);

    cmt_encode_prometheus_destroy(prom);

    return str;
}

static VALUE
rb_cmetrics_serde_to_influx(VALUE self)
{
    struct CMetricsSerde* cmetricsSerde;
    cmt_sds_t prom;
    VALUE str;

    TypedData_Get_Struct(
            self, struct CMetricsSerde, &rb_cmetrics_serde_type, cmetricsSerde);

    prom = cmt_encode_influx_create(cmetricsSerde->instance);

    str = rb_str_new2(prom);

    cmt_encode_influx_destroy(prom);

    return str;
}
static VALUE
rb_cmetrics_serde_to_msgpack(VALUE self)
{
    struct CMetricsSerde* cmetricsSerde;
    char *buffer;
    size_t buffer_size;
    int ret = 0;

    TypedData_Get_Struct(
            self, struct CMetricsSerde, &rb_cmetrics_serde_type, cmetricsSerde);

    ret = cmt_encode_msgpack(cmetricsSerde->instance, &buffer, &buffer_size);

    if (ret == 0) {
        return rb_str_new(buffer, buffer_size);
    } else {
        return Qnil;
    }
}

static VALUE
rb_cmetrics_serde_to_text(VALUE self)
{
    struct CMetricsSerde* cmetricsSerde;
    cmt_sds_t buffer;
    VALUE text;

    TypedData_Get_Struct(
            self, struct CMetricsSerde, &rb_cmetrics_serde_type, cmetricsSerde);

    buffer = cmt_encode_text_create(cmetricsSerde->instance, CMT_TRUE);
    if (buffer == NULL) {
        return Qnil;
    }

    text = rb_str_new2(buffer);

    cmt_sds_destroy(buffer);

    return text;
}

void Init_cmetrics_serde(VALUE rb_mCMetrics)
{
    rb_cSerde = rb_define_class_under(rb_mCMetrics, "Serde", rb_cObject);

    rb_define_alloc_func(rb_cSerde, rb_cmetrics_serde_alloc);

    rb_define_method(rb_cSerde, "from_msgpack", rb_cmetrics_serde_from_msgpack, 1);
    rb_define_method(rb_cSerde, "to_prometheus", rb_cmetrics_serde_to_prometheus, 0);
    rb_define_method(rb_cSerde, "to_influx", rb_cmetrics_serde_to_influx, 0);
    rb_define_method(rb_cSerde, "to_msgpack", rb_cmetrics_serde_to_msgpack, 0);
    rb_define_method(rb_cSerde, "to_s", rb_cmetrics_serde_to_text, 0);
}
