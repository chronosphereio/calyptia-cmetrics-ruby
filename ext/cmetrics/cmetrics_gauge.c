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

VALUE rb_cGauge;

static void gauge_free(void* ptr);

static const rb_data_type_t rb_cmetrics_gauge_type = { "cmetrics/gauge",
                                                       {
                                                         0,
                                                         gauge_free,
                                                         0,
                                                       },
                                                       NULL,
                                                       NULL,
                                                       RUBY_TYPED_FREE_IMMEDIATELY };


const struct CMetricsGauge *cmetrics_gauge_get_ptr(VALUE self)
{
    struct CMetricsGauge *cmetricsGauge = NULL;

    TypedData_Get_Struct(
            self, struct CMetricsGauge, &rb_cmetrics_gauge_type, cmetricsGauge);

    if (NIL_P(self)) {
        rb_raise(rb_eRuntimeError, "Given CMetrics argument must not be nil");
    }
    if (!cmetricsGauge->gauge) {
        rb_raise(rb_eRuntimeError, "Create gauge with CMetrics::Gauge#create first.");
    }
    return cmetricsGauge;
}

static void
gauge_free(void* ptr)
{
    struct CMetricsGauge* cmetricsGauge = (struct CMetricsGauge*)ptr;

    if (!cmetricsGauge) {
        if (!cmetricsGauge->gauge) {
            cmt_gauge_destroy(cmetricsGauge->gauge);
        }
        if (!cmetricsGauge->instance) {
            cmt_destroy(cmetricsGauge->instance);
        }
    }

    xfree(ptr);
}

static VALUE
rb_cmetrics_gauge_alloc(VALUE klass)
{
    VALUE obj;
    struct CMetricsGauge* cmetricsGauge;
    obj = TypedData_Make_Struct(
            klass, struct CMetricsGauge, &rb_cmetrics_gauge_type, cmetricsGauge);
    return obj;
}

/*
 * Initailize Gauge class.
 *
 * @return [Gauge]
 *
 */
static VALUE
rb_cmetrics_gauge_initialize(VALUE self)
{
    struct CMetricsGauge* cmetricsGauge;

    TypedData_Get_Struct(
            self, struct CMetricsGauge, &rb_cmetrics_gauge_type, cmetricsGauge);

    cmt_initialize();

    cmetricsGauge->instance = cmt_create();
    cmetricsGauge->gauge = NULL;

    return Qnil;
}

/*
 * Create gauge.
 *
 */
static VALUE
rb_cmetrics_gauge_create(int argc, VALUE* argv, VALUE self)
{
    VALUE rb_namespace, rb_subsystem, rb_name, rb_help, rb_labels;
    struct CMetricsGauge* cmetricsGauge;
    char **labels = NULL;
    int labels_count = 0;
    VALUE tmp_label;
    long i;

    TypedData_Get_Struct(
            self, struct CMetricsGauge, &rb_cmetrics_gauge_type, cmetricsGauge);

    rb_scan_args(argc, argv, "41", &rb_namespace, &rb_subsystem, &rb_name, &rb_help, &rb_labels);

    Check_Type(rb_namespace, T_STRING);
    Check_Type(rb_subsystem, T_STRING);
    Check_Type(rb_name, T_STRING);
    Check_Type(rb_help, T_STRING);

    if (!NIL_P(rb_labels)) {
        switch(TYPE(rb_labels)) {
        case T_STRING:
            labels = (char *[]) { StringValuePtr(rb_labels) };
            labels_count = 1;
            cmetricsGauge->gauge = cmt_gauge_create(cmetricsGauge->instance,
                                                    StringValuePtr(rb_namespace),
                                                    StringValuePtr(rb_subsystem),
                                                    StringValuePtr(rb_name),
                                                    StringValuePtr(rb_help),
                                                    labels_count, labels);
            break;
        case T_SYMBOL:
            labels = (char *[]) { RSTRING_PTR(rb_sym2str(rb_labels)) };
            labels_count = 1;
            cmetricsGauge->gauge = cmt_gauge_create(cmetricsGauge->instance,
                                                    StringValuePtr(rb_namespace),
                                                    StringValuePtr(rb_subsystem),
                                                    StringValuePtr(rb_name),
                                                    StringValuePtr(rb_help),
                                                    labels_count, labels);
            break;
        case T_ARRAY:
            labels_count = (int)RARRAY_LEN(rb_labels);
            labels = ALLOCV_N(char *, tmp_label, labels_count);
            for (i = 0; i < labels_count; i++) {
                VALUE item = RARRAY_AREF(rb_labels, i);
                switch(TYPE(item)) {
                case T_SYMBOL:
                    labels[i] = RSTRING_PTR(rb_sym2str(item));
                    break;
                case T_STRING:
                    labels[i] = StringValuePtr(item);
                    break;
                default:
                    rb_raise(rb_eArgError, "labels must be Symbol/String");
                }
            }
            cmetricsGauge->gauge = cmt_gauge_create(cmetricsGauge->instance,
                                                    StringValuePtr(rb_namespace),
                                                    StringValuePtr(rb_subsystem),
                                                    StringValuePtr(rb_name),
                                                    StringValuePtr(rb_help),
                                                    labels_count, labels);

            ALLOCV_END(tmp_label);
            break;
        default:
            rb_raise(rb_eArgError, "labels should be String, Symbol or Array class instance.");
        }
    } else {
        cmetricsGauge->gauge = cmt_gauge_create(cmetricsGauge->instance,
                                                StringValuePtr(rb_namespace),
                                                StringValuePtr(rb_subsystem),
                                                StringValuePtr(rb_name),
                                                StringValuePtr(rb_help),
                                                labels_count, labels);
    }

    return Qnil;
}

/*
 * Just increment gauge.
 *
 * @return [Boolean]
 *
 */
static VALUE
rb_cmetrics_gauge_increment(int argc, VALUE* argv, VALUE self)
{
    VALUE rb_labels;
    struct CMetricsGauge* cmetricsGauge;
    uint64_t ts;
    int ret = 0;
    char **labels = NULL;
    int labels_count = 0;
    VALUE tmp_label;
    long i;

    TypedData_Get_Struct(
            self, struct CMetricsGauge, &rb_cmetrics_gauge_type, cmetricsGauge);

    if (!cmetricsGauge->gauge) {
        rb_raise(rb_eRuntimeError, "Create gauge with CMetrics::Gauge#create first.");
    }

    rb_scan_args(argc, argv, "01", &rb_labels);

    ts = cmt_time_now();
    if (!NIL_P(rb_labels)) {
        switch(TYPE(rb_labels)) {
        case T_STRING:
            labels = (char *[]) { StringValuePtr(rb_labels) };
            labels_count = 1;

            ret = cmt_gauge_inc(cmetricsGauge->gauge, ts,
                                labels_count, labels);
            break;
        case T_SYMBOL:
            labels = (char *[]) { RSTRING_PTR(rb_sym2str(rb_labels)) };
            labels_count = 1;

            ret = cmt_gauge_inc(cmetricsGauge->gauge, ts,
                                labels_count, labels);
            break;
        case T_ARRAY:
            labels_count = (int)RARRAY_LEN(rb_labels);
            labels = ALLOCV_N(char *, tmp_label, labels_count);
            for (i = 0; i < labels_count; i++) {
                VALUE item = RARRAY_AREF(rb_labels, i);
                switch(TYPE(item)) {
                case T_SYMBOL:
                    labels[i] = RSTRING_PTR(rb_sym2str(item));
                    break;
                case T_STRING:
                    labels[i] = StringValuePtr(item);
                    break;
                default:
                    rb_raise(rb_eArgError, "labels must be Symbol/String");
                }
            }
            ret = cmt_gauge_inc(cmetricsGauge->gauge, ts,
                                labels_count, labels);

            ALLOCV_END(tmp_label);

            break;
        default:
            rb_raise(rb_eArgError, "labels should be String, Symbol or Array class instance.");
        }
    } else {
        ret = cmt_gauge_inc(cmetricsGauge->gauge, ts,
                            labels_count, labels);
    }

    if (ret == 0) {
        return Qtrue;
    } else {
        return Qfalse;
    }
}

/*
 * Just decrement gauge.
 *
 * @return [Boolean]
 *
 */
static VALUE
rb_cmetrics_gauge_decrement(int argc, VALUE* argv, VALUE self)
{
    VALUE rb_labels;
    struct CMetricsGauge* cmetricsGauge;
    uint64_t ts;
    int ret = 0;
    char **labels = NULL;
    int labels_count = 0;
    VALUE tmp_label;
    long i;

    TypedData_Get_Struct(
            self, struct CMetricsGauge, &rb_cmetrics_gauge_type, cmetricsGauge);

    if (!cmetricsGauge->gauge) {
        rb_raise(rb_eRuntimeError, "Create gauge with CMetrics::Gauge#create first.");
    }

    rb_scan_args(argc, argv, "01", &rb_labels);

    ts = cmt_time_now();
    if (!NIL_P(rb_labels)) {
        switch(TYPE(rb_labels)) {
        case T_STRING:
            labels = (char *[]) { StringValuePtr(rb_labels) };
            labels_count = 1;

            ret = cmt_gauge_dec(cmetricsGauge->gauge, ts,
                                labels_count, labels);
            break;
        case T_SYMBOL:
            labels = (char *[]) { RSTRING_PTR(rb_sym2str(rb_labels)) };
            labels_count = 1;

            ret = cmt_gauge_dec(cmetricsGauge->gauge, ts,
                                labels_count, labels);
            break;
        case T_ARRAY:
            labels_count = (int)RARRAY_LEN(rb_labels);
            labels = ALLOCV_N(char *, tmp_label, labels_count);
            for (i = 0; i < labels_count; i++) {
                VALUE item = RARRAY_AREF(rb_labels, i);
                switch(TYPE(item)) {
                case T_SYMBOL:
                    labels[i] = RSTRING_PTR(rb_sym2str(item));
                    break;
                case T_STRING:
                    labels[i] = StringValuePtr(item);
                    break;
                default:
                    rb_raise(rb_eArgError, "labels must be Symbol/String");
                }
            }
            ret = cmt_gauge_dec(cmetricsGauge->gauge, ts,
                                labels_count, labels);

            ALLOCV_END(tmp_label);

            break;
        default:
            rb_raise(rb_eArgError, "labels should be String, Symbol or Array class instance.");
        }
    } else {
        ret = cmt_gauge_dec(cmetricsGauge->gauge, ts,
                            labels_count, labels);
    }

    if (ret == 0) {
        return Qtrue;
    } else {
        return Qfalse;
    }
}

static VALUE
rb_cmetrics_gauge_get_value(int argc, VALUE* argv, VALUE self)
{
    VALUE rb_labels;
    struct CMetricsGauge* cmetricsGauge;
    int ret = 0;
    double value = 0;
    char **labels = NULL;
    int labels_count = 0;
    VALUE tmp_label;
    long i;

    TypedData_Get_Struct(
            self, struct CMetricsGauge, &rb_cmetrics_gauge_type, cmetricsGauge);

    if (!cmetricsGauge->gauge) {
        rb_raise(rb_eRuntimeError, "Create gauge with CMetrics::Gauge#create first.");
    }

    rb_scan_args(argc, argv, "01", &rb_labels);

    if (!NIL_P(rb_labels)) {
        switch(TYPE(rb_labels)) {
        case T_STRING:
            labels = (char *[]) { StringValuePtr(rb_labels) };
            labels_count = 1;

            ret = cmt_gauge_get_val(cmetricsGauge->gauge,
                                    labels_count, labels, &value);
            break;
        case T_SYMBOL:
            labels = (char *[]) { RSTRING_PTR(rb_sym2str(rb_labels)) };
            labels_count = 1;

            ret = cmt_gauge_get_val(cmetricsGauge->gauge,
                                      labels_count, labels, &value);
            break;
        case T_ARRAY:
            labels_count = (int)RARRAY_LEN(rb_labels);
            labels = ALLOCV_N(char *, tmp_label, labels_count);
            for (i = 0; i < labels_count; i++) {
                VALUE item = RARRAY_AREF(rb_labels, i);
                switch(TYPE(item)) {
                case T_SYMBOL:
                    labels[i] = RSTRING_PTR(rb_sym2str(item));
                    break;
                case T_STRING:
                    labels[i] = StringValuePtr(item);
                    break;
                default:
                    rb_raise(rb_eArgError, "labels must be Symbol/String");
                }
            }

            ret = cmt_gauge_get_val(cmetricsGauge->gauge,
                                    labels_count, labels, &value);

            ALLOCV_END(tmp_label);

            break;
        default:
            rb_raise(rb_eArgError, "labels should be String, Symbol or Array class instance.");
        }
    } else {
        ret = cmt_gauge_get_val(cmetricsGauge->gauge,
                                labels_count, labels, &value);
    }

    if (ret == 0) {
        return DBL2NUM(value);
    } else {
        return Qnil;
    }
}

/*
 * Add value into counter.
 *
 * @return [Boolean]
 *
 */
static VALUE
rb_cmetrics_gauge_add(int argc, VALUE* argv, VALUE self)
{
    VALUE rb_labels, rb_num;
    struct CMetricsGauge* cmetricsGauge;
    uint64_t ts;
    int ret = 0;
    double value = 0;
    char **labels = NULL;
    int labels_count = 0;
    VALUE tmp_label;
    long i;

    TypedData_Get_Struct(
            self, struct CMetricsGauge, &rb_cmetrics_gauge_type, cmetricsGauge);

    if (!cmetricsGauge->gauge) {
        rb_raise(rb_eRuntimeError, "Create gauge with CMetrics::Gauge#create first.");
    }

    rb_scan_args(argc, argv, "11", &rb_num, &rb_labels);

    switch(TYPE(rb_num)) {
    case T_FLOAT:
    case T_FIXNUM:
    case T_BIGNUM:
    case T_RATIONAL:
        value = NUM2DBL(rb_num);
        break;
    default:
        rb_raise(rb_eArgError, "CMetrics::Gauge#add can handle numerics values only.");
    }


    ts = cmt_time_now();
    if (!NIL_P(rb_labels)) {
        switch(TYPE(rb_labels)) {
        case T_STRING:
            labels = (char *[]) { StringValuePtr(rb_labels) };
            labels_count = 1;
            ret = cmt_gauge_add(cmetricsGauge->gauge, ts, value,
                                labels_count, labels);
            break;
        case T_SYMBOL:
            labels = (char *[]) { RSTRING_PTR(rb_sym2str(rb_labels)) };
            labels_count = 1;
            ret = cmt_gauge_add(cmetricsGauge->gauge, ts, value,
                                labels_count, labels);
            break;
        case T_ARRAY:
            labels_count = (int)RARRAY_LEN(rb_labels);
            labels = ALLOCV_N(char *, tmp_label, labels_count);
            for (i = 0; i < labels_count; i++) {
                VALUE item = RARRAY_AREF(rb_labels, i);
                switch(TYPE(item)) {
                case T_SYMBOL:
                    labels[i] = RSTRING_PTR(rb_sym2str(item));
                    break;
                case T_STRING:
                    labels[i] = StringValuePtr(item);
                    break;
                default:
                    rb_raise(rb_eArgError, "labels must be Symbol/String");
                }
            }
            ret = cmt_gauge_add(cmetricsGauge->gauge, ts, value,
                                labels_count, labels);

            ALLOCV_END(tmp_label);

            break;
        default:
            rb_raise(rb_eArgError, "labels should be String, Symbol or Array class instance.");
        }
    } else {
        ret = cmt_gauge_add(cmetricsGauge->gauge, ts, value,
                            labels_count, labels);
    }

    if (ret == 0) {
        return Qtrue;
    } else {
        return Qfalse;
    }
}

/*
 * Subtract value into counter.
 *
 * @return [Boolean]
 *
 */
static VALUE
rb_cmetrics_gauge_sub(int argc, VALUE* argv, VALUE self)
{
    VALUE rb_labels, rb_num;
    struct CMetricsGauge* cmetricsGauge;
    uint64_t ts;
    int ret = 0;
    double value = 0;
    char **labels = NULL;
    int labels_count = 0;
    VALUE tmp_label;
    long i;

    TypedData_Get_Struct(
            self, struct CMetricsGauge, &rb_cmetrics_gauge_type, cmetricsGauge);

    if (!cmetricsGauge->gauge) {
        rb_raise(rb_eRuntimeError, "Create gauge with CMetrics::Gauge#create first.");
    }

    rb_scan_args(argc, argv, "11", &rb_num, &rb_labels);

    switch(TYPE(rb_num)) {
    case T_FLOAT:
    case T_FIXNUM:
    case T_BIGNUM:
    case T_RATIONAL:
        value = NUM2DBL(rb_num);
        break;
    default:
        rb_raise(rb_eArgError, "CMetrics::Gauge#add can handle numerics values only.");
    }


    ts = cmt_time_now();
    if (!NIL_P(rb_labels)) {
        switch(TYPE(rb_labels)) {
        case T_STRING:
            labels = (char *[]) { StringValuePtr(rb_labels) };
            labels_count = 1;
            ret = cmt_gauge_sub(cmetricsGauge->gauge, ts, value,
                                labels_count, labels);
            break;
        case T_SYMBOL:
            labels = (char *[]) { RSTRING_PTR(rb_sym2str(rb_labels)) };
            labels_count = 1;
            ret = cmt_gauge_sub(cmetricsGauge->gauge, ts, value,
                                labels_count, labels);
            break;
        case T_ARRAY:
            labels_count = (int)RARRAY_LEN(rb_labels);
            labels = ALLOCV_N(char *, tmp_label, labels_count);
            for (i = 0; i < labels_count; i++) {
                VALUE item = RARRAY_AREF(rb_labels, i);
                switch(TYPE(item)) {
                case T_SYMBOL:
                    labels[i] = RSTRING_PTR(rb_sym2str(item));
                    break;
                case T_STRING:
                    labels[i] = StringValuePtr(item);
                    break;
                default:
                    rb_raise(rb_eArgError, "labels must be Symbol/String");
                }
            }
            ret = cmt_gauge_sub(cmetricsGauge->gauge, ts, value,
                                labels_count, labels);

            ALLOCV_END(tmp_label);

            break;
        default:
            rb_raise(rb_eArgError, "labels should be String, Symbol or Array class instance.");
        }
    } else {
        ret = cmt_gauge_sub(cmetricsGauge->gauge, ts, value,
                            labels_count, labels);
    }

    if (ret == 0) {
        return Qtrue;
    } else {
        return Qfalse;
    }
}

/*
 * Set value into gauge.
 *
 * @return [Boolean]
 *
 */
static VALUE
rb_cmetrics_gauge_set(int argc, VALUE* argv, VALUE self)
{
    VALUE rb_labels, rb_num;
    struct CMetricsGauge* cmetricsGauge;
    uint64_t ts;
    int ret = 0;
    double value = 0;
    char **labels = NULL;
    int labels_count = 0;
    VALUE tmp_label;
    long i;

    TypedData_Get_Struct(
            self, struct CMetricsGauge, &rb_cmetrics_gauge_type, cmetricsGauge);

    if (!cmetricsGauge->gauge) {
        rb_raise(rb_eRuntimeError, "Create gauge with CMetrics::Gauge#create first.");
    }

    rb_scan_args(argc, argv, "11", &rb_num, &rb_labels);

    switch(TYPE(rb_num)) {
    case T_FLOAT:
    case T_FIXNUM:
    case T_BIGNUM:
    case T_RATIONAL:
        value = NUM2DBL(rb_num);
        break;
    default:
        rb_raise(rb_eArgError, "CMetrics::Gauge#set can handle numerics values only.");
    }

    ts = cmt_time_now();
    if (!NIL_P(rb_labels)) {
        switch(TYPE(rb_labels)) {
        case T_STRING:
            labels = (char *[]) { StringValuePtr(rb_labels) };
            labels_count = 1;
            ret = cmt_gauge_set(cmetricsGauge->gauge, ts, value,
                                labels_count, labels);
            break;
        case T_SYMBOL:
            labels = (char *[]) { RSTRING_PTR(rb_sym2str(rb_labels)) };
            labels_count = 1;
            ret = cmt_gauge_set(cmetricsGauge->gauge, ts, value,
                                labels_count, labels);
            break;
        case T_ARRAY:
            labels_count = (int)RARRAY_LEN(rb_labels);
            labels = ALLOCV_N(char *, tmp_label, labels_count);
            for (i = 0; i < labels_count; i++) {
                VALUE item = RARRAY_AREF(rb_labels, i);
                switch(TYPE(item)) {
                case T_SYMBOL:
                    labels[i] = RSTRING_PTR(rb_sym2str(item));
                    break;
                case T_STRING:
                    labels[i] = StringValuePtr(item);
                    break;
                default:
                    rb_raise(rb_eArgError, "labels must be Symbol/String");
                }
            }

            ret = cmt_gauge_set(cmetricsGauge->gauge, ts, value,
                                labels_count, labels);

            ALLOCV_END(tmp_label);

            break;
        default:
            rb_raise(rb_eArgError, "labels should be String, Symbol or Array class instance.");
        }
    } else {
        ret = cmt_gauge_set(cmetricsGauge->gauge, ts, value,
                            labels_count, labels);
    }

    if (ret == 0) {
        return Qtrue;
    } else {
        return Qfalse;
    }
}

/*
 * Set label into gauge.
 *
 * @return [Boolean]
 */

static VALUE
rb_cmetrics_gauge_add_label(VALUE self, VALUE rb_key, VALUE rb_value)
{
    struct CMetricsGauge* cmetricsGauge;
    char *key, *value;
    int ret = 0;

    TypedData_Get_Struct(
            self, struct CMetricsGauge, &rb_cmetrics_gauge_type, cmetricsGauge);

    if (!cmetricsGauge->gauge) {
        rb_raise(rb_eRuntimeError, "Create gauge with CMetrics::Gauge#create first.");
    }

    switch(TYPE(rb_key)) {
    case T_STRING:
        key = StringValuePtr(rb_key);
        break;
    case T_SYMBOL:
        key = RSTRING_PTR(rb_sym2str(rb_key));
        break;
    default:
        rb_raise(rb_eArgError, "key should be String or Symbol class instance.");
    }

    switch(TYPE(rb_value)) {
    case T_STRING:
        value = StringValuePtr(rb_value);
        break;
    case T_SYMBOL:
        value = RSTRING_PTR(rb_sym2str(rb_value));
        break;
    default:
        rb_raise(rb_eArgError, "value should be String or Symbol class instance.");
    }

    ret = cmt_label_add(cmetricsGauge->instance, key, value);

    if (ret == 0) {
        return Qtrue;
    } else {
        return Qfalse;
    }
}

static VALUE
rb_cmetrics_gauge_to_influx(VALUE self)
{
    struct CMetricsGauge* cmetricsGauge;
    cmt_sds_t prom;
    VALUE str;

    TypedData_Get_Struct(
            self, struct CMetricsGauge, &rb_cmetrics_gauge_type, cmetricsGauge);

    prom = cmt_encode_influx_create(cmetricsGauge->instance);

    str = rb_str_new2(prom);

    cmt_encode_influx_destroy(prom);

    return str;
}

static VALUE
rb_cmetrics_gauge_to_prometheus(VALUE self)
{
    struct CMetricsGauge* cmetricsGauge;
    cmt_sds_t prom;
    VALUE str;

    TypedData_Get_Struct(
            self, struct CMetricsGauge, &rb_cmetrics_gauge_type, cmetricsGauge);

    prom = cmt_encode_prometheus_create(cmetricsGauge->instance, CMT_TRUE);

    str = rb_str_new2(prom);

    cmt_encode_prometheus_destroy(prom);

    return str;
}

static VALUE
rb_cmetrics_gauge_to_msgpack(VALUE self)
{
    struct CMetricsGauge* cmetricsGauge;
    char *buffer;
    size_t buffer_size;
    int ret = 0;

    TypedData_Get_Struct(
            self, struct CMetricsGauge, &rb_cmetrics_gauge_type, cmetricsGauge);

    ret = cmt_encode_msgpack_create(cmetricsGauge->instance, &buffer, &buffer_size);

    if (ret == 0) {
        return rb_str_new(buffer, buffer_size);
    } else {
        return Qnil;
    }
}

static VALUE
rb_cmetrics_gauge_to_text(VALUE self)
{
    struct CMetricsGauge* cmetricsGauge;
    cmt_sds_t  buffer;
    VALUE text;

    TypedData_Get_Struct(
            self, struct CMetricsGauge, &rb_cmetrics_gauge_type, cmetricsGauge);

    buffer = cmt_encode_text_create(cmetricsGauge->instance);
    if (buffer == NULL) {
        return Qnil;
    }

    text = rb_str_new2(buffer);

    cmt_sds_destroy(buffer);

    return text;
}

void Init_cmetrics_gauge(VALUE rb_mCMetrics)
{
    rb_cGauge = rb_define_class_under(rb_mCMetrics, "Gauge", rb_cObject);

    rb_define_alloc_func(rb_cGauge, rb_cmetrics_gauge_alloc);

    rb_define_method(rb_cGauge, "initialize", rb_cmetrics_gauge_initialize, 0);
    rb_define_method(rb_cGauge, "create", rb_cmetrics_gauge_create, -1);
    rb_define_method(rb_cGauge, "inc", rb_cmetrics_gauge_increment, -1);
    rb_define_method(rb_cGauge, "increment", rb_cmetrics_gauge_increment, -1);
    rb_define_method(rb_cGauge, "dec", rb_cmetrics_gauge_decrement, -1);
    rb_define_method(rb_cGauge, "decrement", rb_cmetrics_gauge_decrement, -1);
    rb_define_method(rb_cGauge, "get_value", rb_cmetrics_gauge_get_value, -1);
    rb_define_method(rb_cGauge, "value", rb_cmetrics_gauge_get_value, -1);
    rb_define_method(rb_cGauge, "val", rb_cmetrics_gauge_get_value, -1);
    rb_define_method(rb_cGauge, "add", rb_cmetrics_gauge_add, -1);
    rb_define_method(rb_cGauge, "sub", rb_cmetrics_gauge_sub, -1);
    rb_define_method(rb_cGauge, "set", rb_cmetrics_gauge_set, -1);
    rb_define_method(rb_cGauge, "val=", rb_cmetrics_gauge_set, -1);
    rb_define_method(rb_cGauge, "value=", rb_cmetrics_gauge_set, -1);
    rb_define_method(rb_cGauge, "add_label", rb_cmetrics_gauge_add_label, 2);
    rb_define_method(rb_cGauge, "to_influx", rb_cmetrics_gauge_to_influx, 0);
    rb_define_method(rb_cGauge, "to_prometheus", rb_cmetrics_gauge_to_prometheus, 0);
    rb_define_method(rb_cGauge, "to_msgpack", rb_cmetrics_gauge_to_msgpack, 0);
    rb_define_method(rb_cGauge, "to_s", rb_cmetrics_gauge_to_text, 0);
}
