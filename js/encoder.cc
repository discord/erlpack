#include <nan.h>
#include "../cpp/encoder.h"
#include <cmath>
#include <limits>

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

    Nan::MaybeLocal<Object> releaseAsBuffer() {
        if (pk.buf == nullptr) {
            return Nan::MaybeLocal<Object>();
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
            String::Utf8Value string(value->ToString(isolate));
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

class Decoder {
public:
    Decoder(Local<Value> value, Isolate* isolate_)
    : isolate(isolate_)
    , data(node::Buffer::Data(value->ToObject()))
    , size(node::Buffer::Length(value->ToObject()))
    , offset(0)
    {
        const auto version = read8();
        if (version != FORMAT_VERSION) {
            Nan::ThrowError("Bad version number."); // Expected %d found %d", FORMAT_VERSION, version);
        }
    }

    uint8_t read8() {
        auto val = *reinterpret_cast<const uint8_t*>(data + offset);
        offset += sizeof(uint8_t);
        return val;
    }

    uint16_t read16() {
        uint16_t val = ntohs(*reinterpret_cast<const uint16_t*>(data + offset));
        offset += sizeof(uint16_t);
        return val;
    }

    uint32_t read32() {
        uint32_t val = ntohl(*reinterpret_cast<const uint32_t*>(data + offset));
        offset += sizeof(uint32_t);
        return val;
    }

    uint64_t read64() {
        uint64_t val = ntohll(*reinterpret_cast<const uint64_t*>(data + offset));
        offset += sizeof(val);
        return val;
    }

    Local<Value> decodeSmallInteger() {
        return Integer::New(isolate, read8());
    }

    Local<Value> decodeInteger() {
        return Integer::New(isolate, read32());
    }

    Local<Value> decodeList() {
        const uint32_t length = read32();
        Local<Object> array = Array::New(isolate, length);
        for(uint32_t i = 0; i < length; ++i) {
            array->Set(i, unpack());
        }

        const auto tailMarker = read8();
        if (tailMarker != NIL_EXT) {
            Nan::ThrowError("List doesn't end with a tail marker, but it must!");
        }

        return array;
    }

    Local<Value> decodeNil() {
        return Local<Value>();
    }

    Local<Value> decodeMap() {
        const uint32_t length = read32();
        auto map = Object::New(isolate);

        for(uint32_t i = 0; i < length; ++i) {
            const auto key = unpack();
            const auto value = unpack();
            map->Set(key, value);
        }

        return map;
    }

    const char* readString(uint32_t length) {
        const char* str = data + offset;
        offset += length;
        return str;
    }

    Local<Value> decodeSmallAtom() {
        auto length = read8();
        const char* atom = readString(length);

        if (length == 3) { // nil
            return Nan::Null();
        }
        else if (length == 4) { // true or null

            if (atom[0] == 'n') { // null
                return Nan::Null();
            }

            return Nan::True();
        }
        else if (length == 5) { // false
            return Nan::False();
        }

        return Nan::New(atom, length).ToLocalChecked();
    }

    Local<Value> decodeFloat() {
        const uint8_t FLOAT_LENGTH = 31;
        const char* floatStr = readString(FLOAT_LENGTH);
        double number;
        char nullTerimated[FLOAT_LENGTH + 1] = {0};
        memcpy(nullTerimated, floatStr, FLOAT_LENGTH);

        auto count = sscanf(nullTerimated, "%lf", &number);
        if (count != 1) {
            Nan::ThrowError("Invalid float encoded.");
        }

        return Number::New(isolate, number);
    }

    Local<Value> decodeNewFloat() {
        uint64_t val = read64();
        return Number::New(isolate, *reinterpret_cast<double*>(&val));
    }

    Local<Value> decodeBig(uint32_t digits) {
        const uint8_t sign = read8();

        if (digits > 8) {
            Nan::ThrowError("Unable to decode big ints larger than 8 bytes");
        }

        uint64_t value = 0;
        uint64_t b = 1;
        for(uint32_t i = 0; i < digits; ++i) {
            uint64_t digit = read8();
            value += digit * b;
            b <<= 8;
        }

        if (digits <= 4) {
            if (sign == 0) {
                return Integer::New(isolate, static_cast<uint32_t>(value));
            }

            const bool isSignBitAvailable = (value & (1 << 31)) == 0;
            if (isSignBitAvailable) {
                int32_t negativeValue = -static_cast<int32_t>(value);
                return Integer::New(isolate, negativeValue);
            }
        }

        char outBuffer[32] = {0}; // 9223372036854775807
        const char* const formatString = sign == 0 ? "%llu" : "-%llu";
        const uint8_t length = sprintf(outBuffer, formatString, value);

        if (length < 0) {
            Nan::ThrowError("Unable to convert big int to string");
        }

        return Nan::New(outBuffer, length).ToLocalChecked();
    }

    Local<Value> decodeSmallBig() {
        const auto bytes = read8();
        return decodeBig(bytes);
    }

    Local<Value> decodeLargeBig() {
        const auto bytes = read32();
        return decodeBig(bytes);
    }

    Local<Value> decodeBinary() {
        return decodeString();
    }

    Local<Value> decodeString() {
        const auto length = read32();
        const char* str = readString(length);
        auto binaryString = Nan::New(str, length);
        return binaryString.ToLocalChecked();
    }
    
    Local<Value> unpack() {
        while(offset < size) {
            const auto type = read8();
            switch(type) {
                case SMALL_INTEGER_EXT:
                    return decodeSmallInteger();
                case INTEGER_EXT:
                    return decodeInteger();
                case FLOAT_EXT:
                    return decodeFloat();
                case NEW_FLOAT_EXT:
                    return decodeNewFloat();
//                case ATOM_EXT:
//                    return decodeAtom();
                case SMALL_ATOM_EXT:
                    return decodeSmallAtom();
//                case SMALL_TUPLE_EXT:
//                    return decodeSmallTuple();
//                case LARGE_TUPLE_EXT:
//                    return decodeLargeTuple();
                case NIL_EXT:
                    return decodeNil();
                case STRING_EXT:
                    return decodeString();
                case LIST_EXT:
                    return decodeList();
                case MAP_EXT:
                    return decodeMap();
                case BINARY_EXT:
                    return decodeBinary();
                case SMALL_BIG_EXT:
                    return decodeSmallBig();
                case LARGE_BIG_EXT:
                    return decodeLargeBig();
//                case REFERENCE_EXT:
//                    return decodeReference();
//                case NEW_REFERENCE_EXT:
//                    return decodeNewReference();
//                case PORT_EXT:
//                    return decodePort();
//                case PID_EXT:
//                    return decodePID();
//                case EXPORT_EXT:
//                    return decodeExport();
//                case COMPRESSED:
//                    return decodeCompressed();
                  default:
                    Nan::ThrowError("Unsupported erlang term type identifier found");
            }
        }

        return Local<Value>();
    }
private:
    Isolate* isolate;
    const char* const data;
    const size_t size;

    size_t offset;
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

NAN_METHOD(Unpack) {
    Isolate* isolate = info.GetIsolate();

    if(!info[0]->IsObject()) {
        Nan::ThrowError("Attempting to unpack a non-object.");
    }

    Decoder decoder(info[0], isolate);
    MaybeLocal<Value> value = decoder.unpack();
    info.GetReturnValue().Set(value.ToLocalChecked());
}

bool is_big_endian()
{
    union {
        uint32_t i;
        char c[4];
    } bint = {0x01020304};

    return bint.c[0] == 1;
}

void Init(Handle<Object> exports) {
    printf("\n\nAm II big endian? %s\n\n", is_big_endian() ? "BE!" : "LE!");
    uint64_t num = std::numeric_limits<uint64_t>::max() / 2;
    printf("%llu", num);

    uint64_t bytesMax = *((uint64_t*)&num);
    printf("LE: ");
    for(int i = 0; i < 8; ++i) {
        printf("%02hhX", (((const char*)&bytesMax)[i]));
    }
    printf("\n");


    uint64_t bigEndianBytes = htonll(bytesMax);
    printf("BE: ");
    for(int i = 0; i < 8; ++i) {
        printf("%02hhX", (((const char*)&bigEndianBytes)[i]));
    }
    printf("\n");

    exports->Set(Nan::New("pack").ToLocalChecked(), Nan::New<FunctionTemplate>(Pack)->GetFunction());
    exports->Set(Nan::New("unpack").ToLocalChecked(), Nan::New<FunctionTemplate>(Unpack)->GetFunction());
}

NODE_MODULE(erlpackjs, Init);