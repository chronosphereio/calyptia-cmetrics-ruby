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

VALUE rb_cCounter;

static void counter_free(void* ptr);

static const rb_data_type_t rb_cmetrics_counter_type = { "cmetrics/counter",
                                                         {
                                                             0,
                                                             counter_free,
                                                             0,
                                                         },
                                                         NULL,
                                                         NULL,
                                                         RUBY_TYPED_FREE_IMMEDIATELY };


static void
counter_free(void* ptr)
{
    struct CMetricsCounter* cmetricsCounter = (struct CMetricsCounter*)ptr;

    if (!cmetricsCounter) {
        if (!cmetricsCounter->instance) {
            cmt_destroy(cmetricsCounter->instance);
        }
        if (!cmetricsCounter->counter) {
            cmt_counter_destroy(cmetricsCounter->counter);
        }
    }

    xfree(ptr);
}

static VALUE
rb_cmetrics_counter_alloc(VALUE klass)
{
    VALUE obj;
    struct CMetricsCounter* cmetricsCounter;
    obj = TypedData_Make_Struct(
            klass, struct CMetricsCounter, &rb_cmetrics_counter_type, cmetricsCounter);
    return obj;
}

/*
 * Initailize Counter class.
 *
 * @return [Counter]
 *
 */
static VALUE
rb_cmetrics_counter_initialize(VALUE self)
{
    struct CMetricsCounter* cmetricsCounter;

    TypedData_Get_Struct(
            self, struct CMetricsCounter, &rb_cmetrics_counter_type, cmetricsCounter);

    cmt_initialize();

    cmetricsCounter->instance = cmt_create();
    cmetricsCounter->counter = NULL;

    return Qnil;
}

/*
 * Create counter.
 *
 */
static VALUE
rb_cmetrics_counter_create(int argc, VALUE* argv, VALUE self)
{
    VALUE rb_namespace, rb_subsystem, rb_name, rb_help, rb_labels;
    struct CMetricsCounter* cmetricsCounter;
    char **labels = NULL;
    int labels_count = 0;
    VALUE tmp_label;
    long i;

    TypedData_Get_Struct(
            self, struct CMetricsCounter, &rb_cmetrics_counter_type, cmetricsCounter);

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
            cmetricsCounter->counter = cmt_counter_create(cmetricsCounter->instance,
                                                          StringValuePtr(rb_namespace),
                                                          StringValuePtr(rb_subsystem),
                                                          StringValuePtr(rb_name),
                                                          StringValuePtr(rb_help),
                                                          labels_count, labels);
            break;
        case T_SYMBOL:
            labels = (char *[]) { RSTRING_PTR(rb_sym2str(rb_labels)) };
            labels_count = 1;
            cmetricsCounter->counter = cmt_counter_create(cmetricsCounter->instance,
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
            cmetricsCounter->counter = cmt_counter_create(cmetricsCounter->instance,
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
        cmetricsCounter->counter = cmt_counter_create(cmetricsCounter->instance,
                                                      StringValuePtr(rb_namespace),
                                                      StringValuePtr(rb_subsystem),
                                                      StringValuePtr(rb_name),
                                                      StringValuePtr(rb_help),
                                                      labels_count, labels);
    }

    return Qnil;
}

/*
 * Just increment counter.
 *
 * @return [Boolean]
 *
 */
static VALUE
rb_cmetrics_counter_increment(int argc, VALUE* argv, VALUE self)
{
    VALUE rb_labels;
    struct CMetricsCounter* cmetricsCounter;
    uint64_t ts;
    int ret = 0;
    char **labels = NULL;
    int labels_count = 0;
    VALUE tmp_label;
    long i;

    TypedData_Get_Struct(
            self, struct CMetricsCounter, &rb_cmetrics_counter_type, cmetricsCounter);

    if (!cmetricsCounter->counter) {
        rb_raise(rb_eRuntimeError, "Create counter with CMetrics::Counter#create first.");
    }

    rb_scan_args(argc, argv, "01", &rb_labels);

    ts = cmt_time_now();
    if (!NIL_P(rb_labels)) {
        switch(TYPE(rb_labels)) {
        case T_STRING:
            labels = (char *[]) { StringValuePtr(rb_labels) };
            labels_count = 1;

            ret = cmt_counter_inc(cmetricsCounter->counter, ts,
                                  labels_count, labels);
            break;
        case T_SYMBOL:
            labels = (char *[]) { RSTRING_PTR(rb_sym2str(rb_labels)) };
            labels_count = 1;

            ret = cmt_counter_inc(cmetricsCounter->counter, ts,
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
            ret = cmt_counter_inc(cmetricsCounter->counter, ts,
                                  labels_count, labels);

            ALLOCV_END(tmp_label);

            break;
        default:
            rb_raise(rb_eArgError, "labels should be String, Symbol or Array class instance.");
        }
    } else {
        ret = cmt_counter_inc(cmetricsCounter->counter, ts,
                              labels_count, labels);
    }

    if (ret == 0) {
        return Qtrue;
    } else {
        return Qfalse;
    }
}

static VALUE
rb_cmetrics_counter_get_value(int argc, VALUE* argv, VALUE self)
{
    VALUE rb_labels;
    struct CMetricsCounter* cmetricsCounter;
    int ret = 0;
    double value = 0;
    char **labels = NULL;
    int labels_count = 0;
    VALUE tmp_label;
    long i;

    TypedData_Get_Struct(
            self, struct CMetricsCounter, &rb_cmetrics_counter_type, cmetricsCounter);

    if (!cmetricsCounter->counter) {
        rb_raise(rb_eRuntimeError, "Create counter with CMetrics::Counter#create first.");
    }

    rb_scan_args(argc, argv, "01", &rb_labels);

    if (!NIL_P(rb_labels)) {
        switch(TYPE(rb_labels)) {
        case T_STRING:
            labels = (char *[]) { StringValuePtr(rb_labels) };
            labels_count = 1;

            ret = cmt_counter_get_val(cmetricsCounter->counter,
                                      labels_count, labels, &value);
            break;
        case T_SYMBOL:
            labels = (char *[]) { RSTRING_PTR(rb_sym2str(rb_labels)) };
            labels_count = 1;

            ret = cmt_counter_get_val(cmetricsCounter->counter,
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

            ret = cmt_counter_get_val(cmetricsCounter->counter,
                                      labels_count, labels, &value);

            ALLOCV_END(tmp_label);

            break;
        default:
            rb_raise(rb_eArgError, "labels should be String, Symbol or Array class instance.");
        }
    } else {
        ret = cmt_counter_get_val(cmetricsCounter->counter,
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
rb_cmetrics_counter_add(int argc, VALUE* argv, VALUE self)
{
    VALUE rb_labels, rb_num;
    struct CMetricsCounter* cmetricsCounter;
    uint64_t ts;
    int ret = 0;
    double value = 0;
    char **labels = NULL;
    int labels_count = 0;
    VALUE tmp_label;
    long i;

    TypedData_Get_Struct(
            self, struct CMetricsCounter, &rb_cmetrics_counter_type, cmetricsCounter);

    if (!cmetricsCounter->counter) {
        rb_raise(rb_eRuntimeError, "Create counter with CMetrics::Counter#create first.");
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
        rb_raise(rb_eArgError, "CMetrics::Counter#add can handle numerics values only.");
    }


    ts = cmt_time_now();
    if (!NIL_P(rb_labels)) {
        switch(TYPE(rb_labels)) {
        case T_STRING:
            labels = (char *[]) { StringValuePtr(rb_labels) };
            labels_count = 1;
            ret = cmt_counter_add(cmetricsCounter->counter, ts, value,
                                  labels_count, labels);
            break;
        case T_SYMBOL:
            labels = (char *[]) { RSTRING_PTR(rb_sym2str(rb_labels)) };
            labels_count = 1;
            ret = cmt_counter_add(cmetricsCounter->counter, ts, value,
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
            ret = cmt_counter_add(cmetricsCounter->counter, ts, value,
                                  labels_count, labels);

            ALLOCV_END(tmp_label);

            break;
        default:
            rb_raise(rb_eArgError, "labels should be String, Symbol or Array class instance.");
        }
    } else {
        ret = cmt_counter_add(cmetricsCounter->counter, ts, value,
                              labels_count, labels);
    }

    if (ret == 0) {
        return Qtrue;
    } else {
        return Qfalse;
    }
}

/*
 * Set value into counter.
 *
 * @return [Boolean]
 *
 */
static VALUE
rb_cmetrics_counter_set(int argc, VALUE* argv, VALUE self)
{
    VALUE rb_labels, rb_num;
    struct CMetricsCounter* cmetricsCounter;
    uint64_t ts;
    int ret = 0;
    double value = 0;
    char **labels = NULL;
    int labels_count = 0;
    VALUE tmp_label;
    long i;

    TypedData_Get_Struct(
            self, struct CMetricsCounter, &rb_cmetrics_counter_type, cmetricsCounter);

    if (!cmetricsCounter->counter) {
        rb_raise(rb_eRuntimeError, "Create counter with CMetrics::Counter#create first.");
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
        rb_raise(rb_eArgError, "CMetrics::Counter#set can handle numerics values only.");
    }

    ts = cmt_time_now();
    if (!NIL_P(rb_labels)) {
        switch(TYPE(rb_labels)) {
        case T_STRING:
            labels = (char *[]) { StringValuePtr(rb_labels) };
            labels_count = 1;
            ret = cmt_counter_set(cmetricsCounter->counter, ts, value,
                                  labels_count, labels);
            break;
        case T_SYMBOL:
            labels = (char *[]) { RSTRING_PTR(rb_sym2str(rb_labels)) };
            labels_count = 1;
            ret = cmt_counter_set(cmetricsCounter->counter, ts, value,
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

            ret = cmt_counter_set(cmetricsCounter->counter, ts, value,
                                  labels_count, labels);

            ALLOCV_END(tmp_label);

            break;
        default:
            rb_raise(rb_eArgError, "labels should be String, Symbol or Array class instance.");
        }
    } else {
        ret = cmt_counter_set(cmetricsCounter->counter, ts, value,
                              labels_count, labels);
    }

    if (ret == 0) {
        return Qtrue;
    } else {
        return Qfalse;
    }
}

/*
 * Set label into counter.
 *
 * @return [Boolean]
 */

static VALUE
rb_cmetrics_counter_add_label(VALUE self, VALUE rb_key, VALUE rb_value)
{
    struct CMetricsCounter* cmetricsCounter;
    char *key, *value;
    int ret = 0;

    TypedData_Get_Struct(
            self, struct CMetricsCounter, &rb_cmetrics_counter_type, cmetricsCounter);

    if (!cmetricsCounter->counter) {
        rb_raise(rb_eRuntimeError, "Create counter with CMetrics::Counter#create first.");
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
        rb_raise(rb_eArgError, "key should be String or Symbol class instance.");
    }
    ret = cmt_label_add(cmetricsCounter->instance, key, value);

    if (ret == 0) {
        return Qtrue;
    } else {
        return Qfalse;
    }
}

static VALUE
rb_cmetrics_counter_to_prometheus(VALUE self)
{
    struct CMetricsCounter* cmetricsCounter;
    cmt_sds_t prom;
    VALUE str;

    TypedData_Get_Struct(
            self, struct CMetricsCounter, &rb_cmetrics_counter_type, cmetricsCounter);

    prom = cmt_encode_prometheus_create(cmetricsCounter->instance, CMT_TRUE);

    str = rb_str_new2(prom);

    cmt_encode_prometheus_destroy(prom);

    return str;
}

static VALUE
rb_cmetrics_counter_to_influx(VALUE self)
{
    struct CMetricsCounter* cmetricsCounter;
    cmt_sds_t prom;
    VALUE str;

    TypedData_Get_Struct(
            self, struct CMetricsCounter, &rb_cmetrics_counter_type, cmetricsCounter);

    prom = cmt_encode_influx_create(cmetricsCounter->instance);

    str = rb_str_new2(prom);

    cmt_encode_influx_destroy(prom);

    return str;
}

static VALUE
rb_cmetrics_counter_to_msgpack(VALUE self)
{
    struct CMetricsCounter* cmetricsCounter;
    char *buffer;
    size_t buffer_size;
    int ret = 0;

    TypedData_Get_Struct(
            self, struct CMetricsCounter, &rb_cmetrics_counter_type, cmetricsCounter);

    ret = cmt_encode_msgpack(cmetricsCounter->instance, &buffer, &buffer_size);

    if (ret == 0) {
        return rb_str_new(buffer, buffer_size);
    } else {
        return Qnil;
    }
}

static VALUE
rb_cmetrics_counter_to_text(VALUE self)
{
    struct CMetricsCounter* cmetricsCounter;
    cmt_sds_t buffer;
    VALUE text;

    TypedData_Get_Struct(
            self, struct CMetricsCounter, &rb_cmetrics_counter_type, cmetricsCounter);

    buffer = cmt_encode_text_create(cmetricsCounter->instance);
    if (buffer == NULL) {
        return Qnil;
    }

    text = rb_str_new2(buffer);

    cmt_sds_destroy(buffer);

    return text;
}

void Init_cmetrics_counter(VALUE rb_mCMetrics)
{
    rb_cCounter = rb_define_class_under(rb_mCMetrics, "Counter", rb_cObject);

    rb_define_alloc_func(rb_cCounter, rb_cmetrics_counter_alloc);

    rb_define_method(rb_cCounter, "initialize", rb_cmetrics_counter_initialize, 0);
    rb_define_method(rb_cCounter, "create", rb_cmetrics_counter_create, -1);
    rb_define_method(rb_cCounter, "inc", rb_cmetrics_counter_increment, -1);
    rb_define_method(rb_cCounter, "increment", rb_cmetrics_counter_increment, -1);
    rb_define_method(rb_cCounter, "get_value", rb_cmetrics_counter_get_value, -1);
    rb_define_method(rb_cCounter, "value", rb_cmetrics_counter_get_value, -1);
    rb_define_method(rb_cCounter, "val", rb_cmetrics_counter_get_value, -1);
    rb_define_method(rb_cCounter, "add", rb_cmetrics_counter_add, -1);
    rb_define_method(rb_cCounter, "set", rb_cmetrics_counter_set, -1);
    rb_define_method(rb_cCounter, "val=", rb_cmetrics_counter_set, -1);
    rb_define_method(rb_cCounter, "value=", rb_cmetrics_counter_set, -1);
    rb_define_method(rb_cCounter, "add_label", rb_cmetrics_counter_add_label, 2);
    rb_define_method(rb_cCounter, "to_influx", rb_cmetrics_counter_to_influx, 0);
    rb_define_method(rb_cCounter, "to_prometheus", rb_cmetrics_counter_to_prometheus, 0);
    rb_define_method(rb_cCounter, "to_msgpack", rb_cmetrics_counter_to_msgpack, 0);
    rb_define_method(rb_cCounter, "to_s", rb_cmetrics_counter_to_text, 0);
}
