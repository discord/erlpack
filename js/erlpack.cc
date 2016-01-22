#include <nan.h>
#include "encoder.h"
#include "decoder.h"

NAN_METHOD(Pack) {
    Encoder encoder;
    const int ret = encoder.pack(info[0]);
    if (ret == -1) {
        Nan::ThrowError("Out of memory");
        info.GetReturnValue().Set(Nan::Null());
        return;
    }
    else if (ret > 0) {
        Nan::ThrowError("Unknown error");
        info.GetReturnValue().Set(Nan::Null());
        return;
    }

    info.GetReturnValue().Set(encoder.releaseAsBuffer().ToLocalChecked());
}

NAN_METHOD(Unpack) {
    if(!info[0]->IsObject()) {
        Nan::ThrowError("Attempting to unpack a non-object.");
        info.GetReturnValue().Set(Nan::Null());
        return;
    }

    Nan::TypedArrayContents<uint8_t> contents(info[0]);

    if (contents.length() == 0) {
        Nan::ThrowError("Zero length buffer.");
        info.GetReturnValue().Set(Nan::Null());
        return;
    }

    Decoder decoder(contents);
    Nan::MaybeLocal<Value> value = decoder.unpack();
    info.GetReturnValue().Set(value.ToLocalChecked());
}

void Init(Handle<Object> exports) {
    exports->Set(Nan::New("pack").ToLocalChecked(), Nan::New<FunctionTemplate>(Pack)->GetFunction());
    exports->Set(Nan::New("unpack").ToLocalChecked(), Nan::New<FunctionTemplate>(Unpack)->GetFunction());
}

NODE_MODULE(erlpack, Init);
