#include <nan.h>

using namespace v8;

NAN_METHOD(Method) {
    info.GetReturnValue().Set(Nan::New("worldly").ToLocalChecked());
}

void Init(Handle<Object> exports) {
    exports->Set(Nan::New("hello").ToLocalChecked(), Nan::New<v8::FunctionTemplate>(Method)->GetFunction());
}

NODE_MODULE(erlpackjs, Init);