#include <nan.h>
#include "encoder.h"
#include "decoder.h"

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

NAN_METHOD(Unpack) {
    Isolate* isolate = info.GetIsolate();

    if(!info[0]->IsObject()) {
        Nan::ThrowError("Attempting to unpack a non-object.");
    }

    Decoder decoder(info[0], isolate);
    Nan::MaybeLocal<Value> value = decoder.unpack();
    info.GetReturnValue().Set(value.ToLocalChecked());
}

void Init(Handle<Object> exports) {
    exports->Set(Nan::New("pack").ToLocalChecked(), Nan::New<FunctionTemplate>(Pack)->GetFunction());
    exports->Set(Nan::New("unpack").ToLocalChecked(), Nan::New<FunctionTemplate>(Unpack)->GetFunction());
}

NODE_MODULE(erlpackjs, Init);