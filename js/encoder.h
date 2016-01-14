#pragma once

#include <nan.h>
#include <cmath>
#include "../cpp/encoder.h"

using namespace v8;

class Encoder {
    static const size_t DEFAULT_RECURSE_LIMIT = 256;
    static const size_t INITIAL_BUFFER_SIZE = 1024 * 1024;
    static const size_t MAX_SIZE = (2L << 32) - 1;

public:
    Encoder() {
        ret = 0;
        pk.buf = (char*)malloc(INITIAL_BUFFER_SIZE);
        pk.length = 0;
        pk.allocated_size = INITIAL_BUFFER_SIZE;

        ret = erlpack_append_version(&pk);
        if (ret == -1) {
            Nan::ThrowError("Unable to allocate large buffer for encoding.");
        }
    }

    Nan::MaybeLocal<Object> releaseAsBuffer() {
        if (pk.buf == NULL) {
            return Nan::MaybeLocal<Object>();
        }

        auto buffer = Nan::NewBuffer(pk.buf, pk.length);
        pk.buf = NULL;
        pk.length = 0;
        pk.allocated_size = 0;
        return buffer;
    }

    ~Encoder() {
        if (pk.buf) {
            free(pk.buf);
        }

        pk.buf = NULL;
        pk.length = 0;
        pk.allocated_size = 0;
    }

    int pack(Local<Value> value, Isolate* isolate, const int nestLimit = DEFAULT_RECURSE_LIMIT) {
        ret = 0;

        if (nestLimit < 0) {
            Nan::ThrowError("Reached recursion limit");
        }

        if (value->IsInt32() || value->IsUint32()) {
            int number = value->Int32Value();
            if (number >= 0 && number <= 255) {
                unsigned char num = (unsigned char)number;
                ret = erlpack_append_small_integer(&pk, num);
            }
            else if (value->IsInt32()) {
                ret = erlpack_append_integer(&pk, number);
            }
            else if (value->IsUint32()) {
                auto uNum = (unsigned long long)value->Uint32Value();
                ret = erlpack_append_unsigned_long_long(&pk, uNum);
            }
        }
        else if(value->IsNumber()) {
            double decimal = value->NumberValue();
            ret = erlpack_append_double(&pk, decimal);
        }
        else if (value->IsNull() || value->IsUndefined()) {
            ret = erlpack_append_nil(&pk);
        }
        else if (value->IsTrue()) {
            ret = erlpack_append_true(&pk);
        }
        else if(value->IsFalse()) {
            ret = erlpack_append_false(&pk);
        }
        else if(value->IsString()) {
            Nan::Utf8String string(value);
            ret = erlpack_append_binary(&pk, *string, string.length());
        }
        else if (value->IsArray()) {
            auto array = Nan::To<Object>(value).ToLocalChecked();
            const auto properties = Nan::GetOwnPropertyNames(array).ToLocalChecked();
            const size_t length = properties->Length();
            if (length == 0) {
                ret = erlpack_append_nil_ext(&pk);
            }
            else {
                if (length > MAX_SIZE) {
                    Nan::ThrowError("List is too large");
                }

                ret = erlpack_append_list_header(&pk, length);
                if (ret != 0) {
                    return ret;
                }

                for(size_t i = 0; i < length; ++i) {
                    const auto k = properties->Get(i);
                    const auto v = Nan::Get(array, k).ToLocalChecked();
                    ret = pack(v, isolate, nestLimit - 1);
                    if (ret != 0) {
                       return ret;
                    }
                }

                ret = erlpack_append_nil_ext(&pk);
            }
        }
        else if (value->IsObject()) {
            auto object = Nan::To<Object>(value).ToLocalChecked();
            const auto properties = Nan::GetOwnPropertyNames(object).ToLocalChecked();

            const size_t len = properties->Length();
            if (len > MAX_SIZE) {
                Nan::ThrowError("Dictionary has too many properties");
            }

            ret = erlpack_append_map_header(&pk, len);
            if (ret != 0) {
                return ret;
            }

            for(size_t i = 0; i < len; ++i) {
                const auto k = properties->Get(i);
                const auto v = Nan::Get(object, k).ToLocalChecked();

                ret = pack(k, isolate, nestLimit - 1);
                if (ret != 0) {
                    return ret;
                }

                ret = pack(v, isolate, nestLimit - 1);
                if (ret != 0) {
                    return ret;
                }
            }
        }

        return ret;
    }

private:
    int ret;
    erlpack_buffer pk;
};