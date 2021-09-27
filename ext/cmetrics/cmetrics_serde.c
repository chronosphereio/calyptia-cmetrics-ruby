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
#include <cmetrics/cmt_map.h>
#include <cmetrics/cmt_metric.h>
#include <cmetrics/cmt_cat.h>

VALUE rb_cSerde;

extern VALUE rb_cCounter;
extern VALUE rb_cGauge;
extern VALUE rb_cUntyped;

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

    if (cmetricsSerde) {
        if (cmetricsSerde->instance) {
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
    cmetricsSerde->unpack_msgpack_offset = 0;

    return Qnil;
}

static VALUE
rb_cmetrics_serde_from_msgpack(int argc, VALUE *argv, VALUE self)
{
    VALUE rb_msgpack_buffer, rb_msgpack_length, rb_offset;
    struct CMetricsSerde* cmetricsSerde;
    int ret = 0;
    struct cmt *cmt = NULL;
    size_t offset = 0;
    size_t msgpack_length = 0;

    TypedData_Get_Struct(
            self, struct CMetricsSerde, &rb_cmetrics_serde_type, cmetricsSerde);

    rb_scan_args(argc, argv, "12", &rb_msgpack_buffer, &rb_msgpack_length, &rb_offset);
    if (!NIL_P(rb_msgpack_length)) {
        Check_Type(rb_msgpack_length, T_FIXNUM);
        msgpack_length = INT2NUM(rb_msgpack_length);
    } else {
        msgpack_length = RSTRING_LEN(rb_msgpack_buffer);
    }
    if (!NIL_P(rb_offset)) {
        Check_Type(rb_offset, T_FIXNUM);
        offset = NUM2INT(rb_offset);
    } else {
        offset = cmetricsSerde->unpack_msgpack_offset;
    }

    if (offset >= msgpack_length) {
        rb_raise(rb_eRuntimeError, "offset should be smaller than msgpack buffer size.");
    }

    if (!NIL_P(rb_msgpack_buffer)) {
        ret = cmt_decode_msgpack_create(&cmt, StringValuePtr(rb_msgpack_buffer), msgpack_length, &offset);
    } else {
        rb_raise(rb_eArgError, "nil is not valid value for buffer");
    }

    if (ret == 0) {
        cmetricsSerde->instance = cmt;
        cmetricsSerde->unpack_msgpack_offset = offset;

        return Qtrue;
    } else {
        return Qnil;
    }
}

static VALUE
rb_cmetrics_serde_from_msgpack_feed_each_impl(VALUE self, VALUE rb_msgpack_buffer, size_t msgpack_length)
{
    struct CMetricsSerde* cmetricsSerde;
    struct cmt *cmt = NULL;
    int ret = 0;
    size_t offset = 0;

    RETURN_ENUMERATOR(self, 0, 0);

    TypedData_Get_Struct(
            self, struct CMetricsSerde, &rb_cmetrics_serde_type, cmetricsSerde);

    for (offset = 0; offset <= msgpack_length; ) {
        ret = cmt_decode_msgpack_create(&cmt, StringValuePtr(rb_msgpack_buffer), msgpack_length, &offset);
        if (ret == 0) {
            cmetricsSerde->instance = cmt;
            cmetricsSerde->unpack_msgpack_offset = offset;

            rb_yield(self);
        } else {
            return Qnil;
        }
    }

    return Qnil;
}

static VALUE
rb_cmetrics_serde_from_msgpack_feed_each(VALUE self, VALUE rb_data)
{
    RETURN_ENUMERATOR(self, 0, 0);
    if (!NIL_P(rb_data)) {
        return rb_cmetrics_serde_from_msgpack_feed_each_impl(self, rb_data, RSTRING_LEN(rb_data));
    } else {
        rb_raise(rb_eArgError, "nil is not valid value for buffer");
    }
}

static VALUE
rb_cmetrics_serde_concat_metric(VALUE self, VALUE rb_data)
{
    struct CMetricsSerde* cmetricsSerde = NULL;
    struct CMetricsCounter* cmetricsCounter = NULL;
    struct CMetricsGauge* cmetricsGauge = NULL;
    struct CMetricsUntyped* cmetricsUntyped = NULL;

    TypedData_Get_Struct(
            self, struct CMetricsSerde, &rb_cmetrics_serde_type, cmetricsSerde);

    if (!NIL_P(rb_data)) {
        if (cmetricsSerde->instance == NULL) {
            cmetricsSerde->instance = cmt_create();
        }

        if (rb_obj_is_kind_of(rb_data, rb_cCounter)) {
            cmetricsCounter = (struct CMetricsCounter *)cmetrics_counter_get_ptr(rb_data);
            cmt_cat(cmetricsSerde->instance, cmetricsCounter->instance);
        } else if (rb_obj_is_kind_of(rb_data, rb_cGauge)) {
            cmetricsGauge = (struct CMetricsGauge *)cmetrics_gauge_get_ptr(rb_data);
            cmt_cat(cmetricsSerde->instance, cmetricsGauge->instance);
        } else if (rb_obj_is_kind_of(rb_data, rb_cUntyped)) {
            cmetricsUntyped = (struct CMetricsUntyped *)cmetrics_untyped_get_ptr(rb_data);
            cmt_cat(cmetricsSerde->instance, cmetricsUntyped->instance);
        } else {
            rb_raise(rb_eArgError, "specified type of instance is not supported.");
        }
    } else {
        rb_raise(rb_eArgError, "nil is not valid value for concatenating");
    }

    return Qnil;
}

static VALUE
rb_cmetrics_serde_to_prometheus(VALUE self)
{
    struct CMetricsSerde* cmetricsSerde;
    cmt_sds_t prom;
    VALUE str;

    TypedData_Get_Struct(
            self, struct CMetricsSerde, &rb_cmetrics_serde_type, cmetricsSerde);

    if (cmetricsSerde->instance == NULL) {
        rb_raise(rb_eRuntimeError, "Invalid cmt context");
    }

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

    if (cmetricsSerde->instance == NULL) {
        rb_raise(rb_eRuntimeError, "Invalid cmt context");
    }

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

    if (cmetricsSerde->instance == NULL) {
        rb_raise(rb_eRuntimeError, "Invalid cmt context");
    }

    ret = cmt_encode_msgpack_create(cmetricsSerde->instance, &buffer, &buffer_size);

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

    if (cmetricsSerde->instance == NULL) {
        rb_raise(rb_eRuntimeError, "Invalid cmt context");
    }

    buffer = cmt_encode_text_create(cmetricsSerde->instance);
    if (buffer == NULL) {
        return Qnil;
    }

    text = rb_str_new2(buffer);

    cmt_sds_destroy(buffer);

    return text;
}

static VALUE
append_metric_value(struct cmt_map *map,
                    VALUE rbHash, struct cmt_metric *metric)
{
    uint64_t ts;
    double val;
    struct cmt_opts *opts;

    opts = map->opts;

    /* Retrieve metric value */
    val = cmt_metric_get_value(metric);

    ts = cmt_metric_get_timestamp(metric);

    rb_hash_aset(rbHash, rb_str_new2("name"), rb_str_new2(opts->name));
    rb_hash_aset(rbHash, rb_str_new2("description"), rb_str_new2(opts->description));
    rb_hash_aset(rbHash, rb_str_new2("value"), DBL2NUM(val));
    rb_hash_aset(rbHash, rb_str_new2("timestamp"), DBL2NUM(ts/1000000000.0));

    return rbHash;
}

static VALUE
format_metric(struct cmt *cmt, struct cmt_map *map,
              struct cmt_metric *metric)
{
    int n;
    int static_labels = 0;
    struct cmt_map_label *label_k;
    struct cmt_map_label *label_v;
    struct mk_list *head;
    struct cmt_opts *opts;
    struct cmt_label *slabel;
    VALUE rb_hash = rb_hash_new();
    VALUE shash = rb_hash_new();
    VALUE lhash = rb_hash_new();

    opts = map->opts;

    /* Measurement */
    rb_hash_aset(rb_hash, rb_str_new2("namespace"), rb_str_new2(opts->ns));
    rb_hash_aset(rb_hash, rb_str_new2("subsystem"), rb_str_new2(opts->subsystem));

    /* Static labels (tags) */
    static_labels = cmt_labels_count(cmt->static_labels);
    if (static_labels > 0) {
        mk_list_foreach(head, &cmt->static_labels->list) {
            slabel = mk_list_entry(head, struct cmt_label, _head);
            rb_hash_aset(shash, rb_str_new2(slabel->key), rb_str_new2(slabel->val));
        }
        rb_hash_aset(rb_hash, rb_str_new2("static_labels"), shash);
    }

    /* Labels / Tags */
    n = mk_list_size(&metric->labels);
    if (n > 0) {
        label_k = mk_list_entry_first(&map->label_keys, struct cmt_map_label, _head);

        mk_list_foreach(head, &metric->labels) {
            label_v = mk_list_entry(head, struct cmt_map_label, _head);

            rb_hash_aset(lhash, rb_str_new2(label_k->name), rb_str_new2(label_v->name));

            label_k = mk_list_entry_next(&label_k->_head, struct cmt_map_label,
                                         _head, &map->label_keys);
        }
        rb_hash_aset(rb_hash, rb_str_new2("labels"), lhash);
    }

    rb_hash = append_metric_value(map, rb_hash, metric);

    return rb_hash;
}

static VALUE
format_metrics(struct cmt *cmt,
               struct cmt_map *map, int add_timestamp)
{
    VALUE rbMetrics = rb_ary_new();
    VALUE rbMetric;
    struct mk_list *head;
    struct cmt_metric *metric;

    /* Simple metric, no labels */
    if (map->metric_static_set == 1) {
        rbMetric = format_metric(cmt, map, &map->metric);
        rb_ary_push(rbMetrics, rbMetric);
    }

    mk_list_foreach(head, &map->metrics) {
        metric = mk_list_entry(head, struct cmt_metric, _head);
        rbMetric = format_metric(cmt, map, metric);
        rb_ary_push(rbMetrics, rbMetric);
    }

    return rbMetrics;
}

static VALUE
rb_cmetrics_serde_get_metrics(VALUE self)
{
    VALUE rbMetrics = rb_ary_new();
    VALUE rbMetricsInner = rb_ary_new();
    struct CMetricsSerde* cmetricsSerde;
    struct mk_list *head;
    struct cmt_gauge *gauge;
    struct cmt_counter *counter;
    struct cmt_untyped *untyped;
    struct cmt *cmt;
    int add_timestamp = CMT_TRUE;

    TypedData_Get_Struct(
            self, struct CMetricsSerde, &rb_cmetrics_serde_type, cmetricsSerde);

    cmt = cmetricsSerde->instance;

    if (cmt == NULL) {
        rb_raise(rb_eRuntimeError, "Invalid cmt context");
    }

    /* Counters */
    mk_list_foreach(head, &cmt->counters) {
        counter = mk_list_entry(head, struct cmt_counter, _head);
        rbMetricsInner = format_metrics(cmt, counter->map, add_timestamp);
        rb_ary_push(rbMetrics, rbMetricsInner);
    }

    /* Gauges */
    mk_list_foreach(head, &cmt->gauges) {
        gauge = mk_list_entry(head, struct cmt_gauge, _head);
        rbMetricsInner = format_metrics(cmt, gauge->map, add_timestamp);
        rb_ary_push(rbMetrics, rbMetricsInner);
    }

    /* Untyped */
    mk_list_foreach(head, &cmt->untypeds) {
        untyped = mk_list_entry(head, struct cmt_untyped, _head);
        rbMetricsInner = format_metrics(cmt, untyped->map, add_timestamp);
        rb_ary_push(rbMetrics, rbMetricsInner);
    }

    return rbMetrics;
}

void Init_cmetrics_serde(VALUE rb_mCMetrics)
{
    rb_cSerde = rb_define_class_under(rb_mCMetrics, "Serde", rb_cObject);

    rb_define_alloc_func(rb_cSerde, rb_cmetrics_serde_alloc);

    rb_define_method(rb_cSerde, "initialize", rb_cmetrics_serde_initialize, 0);
    rb_define_method(rb_cSerde, "concat", rb_cmetrics_serde_concat_metric, 1);
    rb_define_method(rb_cSerde, "from_msgpack", rb_cmetrics_serde_from_msgpack, -1);
    rb_define_method(rb_cSerde, "to_prometheus", rb_cmetrics_serde_to_prometheus, 0);
    rb_define_method(rb_cSerde, "to_influx", rb_cmetrics_serde_to_influx, 0);
    rb_define_method(rb_cSerde, "to_msgpack", rb_cmetrics_serde_to_msgpack, 0);
    rb_define_method(rb_cSerde, "feed_each", rb_cmetrics_serde_from_msgpack_feed_each, 1);
    rb_define_method(rb_cSerde, "to_s", rb_cmetrics_serde_to_text, 0);
    rb_define_method(rb_cSerde, "get_metrics", rb_cmetrics_serde_get_metrics, 0);
    rb_define_method(rb_cSerde, "metrics", rb_cmetrics_serde_get_metrics, 0);
}
