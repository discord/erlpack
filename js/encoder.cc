#include <nan.h>
#include "../cpp/encoder.h"
#include <cmath>

using namespace v8;

const size_t DEFAULT_RECURSE_LIMIT = 256;
const size_t BIG_BUF_SIZE = 1024 * 1024 * 2;
const size_t INITIAL_BUFFER_SIZE = 1024 * 1024;
const size_t MAX_SIZE = pow(2, 32) - 1;

class Encoder {
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

    Nan::MaybeLocal<v8::Object> releaseAsBuffer() {
        if (pk.buf == nullptr) {
            return Nan::MaybeLocal<v8::Object>();
        }

        auto buffer = Nan::NewBuffer(pk.buf, pk.length);
        pk.buf = nullptr;
        pk.length = 0;
        pk.allocated_size = 0;
        return buffer;
    }

    ~Encoder() {
        if (pk.buf) {
            free(pk.buf);
        }

        pk.buf = nullptr;
        pk.length = 0;
        pk.allocated_size = 0;
    }

    int pack(v8::Local<v8::Value> value, Isolate* isolate, const int nestLimit = DEFAULT_RECURSE_LIMIT) {
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
            v8::String::Utf8Value string(value->ToString(isolate));
            ret = erlpack_append_binary(&pk, *string, string.length());
        }
        else if (value->IsArray()) {
            auto array = value->ToObject(isolate);
            const auto properties = array->GetOwnPropertyNames();
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
                    const auto v = array->Get(k);
                    ret = pack(v, isolate, nestLimit - 1);
                    if (ret != 0) {
                       return ret;
                    }
                }

                ret = erlpack_append_nil_ext(&pk);
            }
        }
        else if (value->IsObject()) {
            auto object = value->ToObject(isolate);
            const auto properties = object->GetOwnPropertyNames();

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
                const auto v = object->Get(k);

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

NAN_METHOD(Pack) {
    Isolate* isolate = info.GetIsolate();

    Encoder encoder;
    const int ret = encoder.pack(info[0], isolate);
    if (ret == -1) {
        Nan::ThrowError("Out of memory");
    }
    else if (ret > 0) {
        Nan::ThrowError("Unknown error");
    }

    info.GetReturnValue().Set(encoder.releaseAsBuffer().ToLocalChecked());
}

void Init(Handle<Object> exports) {
    exports->Set(Nan::New("pack").ToLocalChecked(), Nan::New<FunctionTemplate>(Pack)->GetFunction());
}

NODE_MODULE(erlpackjs, Init);