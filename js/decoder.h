#pragma once

#include <nan.h>
#include <zlib.h>

#define THROW(msg) Nan::ThrowError(msg); printf("[Error %s:%d] %s\n", __FILE__, __LINE__, msg)

using namespace v8;

class Decoder {
public:
    Decoder(const Nan::TypedArrayContents<uint8_t>& array, Isolate* isolate_)
    : isolate(isolate_)
    , data(*array)
    , size(array.length())
    , isInvalid(false)
    , offset(0)
    {
        const auto version = read8();
        if (version != FORMAT_VERSION) {
            THROW("Bad version number.");
            isInvalid = true;
        }
    }

   Decoder(Isolate* isolate_, const uint8_t* data_, size_t length_, bool skipVersion = false)
    : isolate(isolate_)
    , data(data_)
    , size(length_)
    , isInvalid(false)
    , offset(0)
    {
        if (!skipVersion) {
            const auto version = read8();
            if (version != FORMAT_VERSION) {
                THROW("Bad version number.");
                isInvalid = true;
            }
        }
    }

    uint8_t read8() {
        if (offset + sizeof(uint8_t) > size) {
            THROW("Reading past the end of the buffer.");
            return 0;
        }
        auto val = *reinterpret_cast<const uint8_t*>(data + offset);
        offset += sizeof(uint8_t);
        return val;
    }

    uint16_t read16() {
        if (offset + sizeof(uint16_t) > size) {
            THROW("Reading past the end of the buffer.");
            return 0;
        }

        uint16_t val = ntohs(*reinterpret_cast<const uint16_t*>(data + offset));
        offset += sizeof(uint16_t);
        return val;
    }

    uint32_t read32() {
        if (offset + sizeof(uint32_t) > size) {
            THROW("Reading past the end of the buffer.");
            return 0;
        }

        uint32_t val = ntohl(*reinterpret_cast<const uint32_t*>(data + offset));
        offset += sizeof(uint32_t);
        return val;
    }

    uint64_t read64() {
        if (offset + sizeof(uint64_t) > size) {
            THROW("Reading past the end of the buffer.");
            return 0;
        }

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
            THROW("List doesn't end with a tail marker, but it must!");
            return Nan::Null();
        }

        return array;
    }

    Local<Value> decodeTuple(uint32_t length) {
        Local<Object> array = Array::New(isolate, length);
        for(uint32_t i = 0; i < length; ++i) {
            array->Set(i, unpack());
        }
        return array;
    }

    Local<Value> decodeNil() {
        Local<Object> array = Array::New(isolate, 0);
        return array;
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
        if (offset + length > size) {
            THROW("Reading past the end of the buffer.");
            return NULL;
        }

        const uint8_t* str = data + offset;
        offset += length;
        return (const char*)str;
    }

    Local<Value> decodeAtom() {
        auto length = read16();
        const char* atom = readString(length);
        return Nan::New(atom, length).ToLocalChecked();
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
            THROW("Invalid float encoded.");
            return Nan::Null();
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
            THROW("Unable to decode big ints larger than 8 bytes");
            return Nan::Null();
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
            THROW("Unable to convert big int to string");
            return Nan::Null();
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
        const auto length = read32();
        const char* str = readString(length);
        auto binaryString = Nan::New(str, length);
        return binaryString.ToLocalChecked();
    }

    Local<Value> decodeString() {
        const auto length = read16();
        const char* str = readString(length);
        auto binaryString = Nan::New(str, length);
        return binaryString.ToLocalChecked();
    }

    Local<Value> decodeSmallTuple() {
        return decodeTuple(read8());
    }

    Local<Value> decodeLargeTuple() {
        return decodeTuple(read32());
    }

    Local<Value> decodeCompressed() {
        const uint32_t uncompressedSize = read32();

        unsigned long sourceSize = uncompressedSize;
        std::auto_ptr<uint8_t> outBuffer((uint8_t*)malloc(uncompressedSize));
        const int ret = uncompress(outBuffer.get(), &sourceSize, (const unsigned char*)(data + offset), size - offset);

        offset += sourceSize;
        if (ret != Z_OK) {
            THROW("Failed to uncompresss compressed item");
            return Nan::Null();
        }

        Decoder children(isolate, outBuffer.get(), uncompressedSize, true);
        Nan::MaybeLocal<Value> value = children.unpack();
        return value.ToLocalChecked();
    }

    Local<Value> decodeReference() {
        auto reference = Object::New(isolate);
        reference->Set(Nan::New("node").ToLocalChecked(), unpack());

        Local<Object> ids = Array::New(isolate, 1);
        ids->Set(0, Integer::New(isolate, read32()));
        reference->Set(Nan::New("id").ToLocalChecked(), ids);

        reference->Set(Nan::New("creation").ToLocalChecked(), Integer::New(isolate, read8()));

        return reference;
    }

    Local<Value> decodeNewReference() {
        auto reference = Object::New(isolate);

        uint16_t len = read16();
        reference->Set(Nan::New("node").ToLocalChecked(), unpack());
        reference->Set(Nan::New("creation").ToLocalChecked(), Integer::New(isolate, read8()));

        Local<Object> ids = Array::New(isolate, len);
        for(uint16_t i = 0; i < len; ++i) {
            ids->Set(i, Integer::New(isolate, read32()));
        }
        reference->Set(Nan::New("id").ToLocalChecked(), ids);

        return reference;
    }

    Local<Value> decodePort() {
        auto port = Object::New(isolate);
        port->Set(Nan::New("node").ToLocalChecked(), unpack());
        port->Set(Nan::New("id").ToLocalChecked(), Integer::New(isolate, read32()));
        port->Set(Nan::New("creation").ToLocalChecked(), Integer::New(isolate, read8()));
        return port;
    }

    Local<Value> decodePID() {
        auto pid = Object::New(isolate);
        pid->Set(Nan::New("node").ToLocalChecked(), unpack());
        pid->Set(Nan::New("id").ToLocalChecked(), Integer::New(isolate, read32()));
        pid->Set(Nan::New("serial").ToLocalChecked(), Integer::New(isolate, read32()));
        pid->Set(Nan::New("creation").ToLocalChecked(), Integer::New(isolate, read8()));
        return pid;
    }

    Local<Value> decodeExport() {
        auto exp = Object::New(isolate);
        exp->Set(Nan::New("mod").ToLocalChecked(), unpack());
        exp->Set(Nan::New("fun").ToLocalChecked(), unpack());
        exp->Set(Nan::New("arity").ToLocalChecked(), unpack());
        return exp;
    }

    Local<Value> unpack() {
        if (isInvalid) {
            return Nan::Null();
        }

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
                case ATOM_EXT:
                    return decodeAtom();
                case SMALL_ATOM_EXT:
                    return decodeSmallAtom();
                case SMALL_TUPLE_EXT:
                    return decodeSmallTuple();
                case LARGE_TUPLE_EXT:
                    return decodeLargeTuple();
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
                case REFERENCE_EXT:
                    return decodeReference();
                case NEW_REFERENCE_EXT:
                    return decodeNewReference();
                case PORT_EXT:
                    return decodePort();
                case PID_EXT:
                    return decodePID();
                case EXPORT_EXT:
                    return decodeExport();
                case COMPRESSED:
                    return decodeCompressed();
                default:
                    THROW("Unsupported erlang term type identifier found");
                    return Nan::Null();
            }
        }

        return Local<Value>();
    }
private:
    Isolate* isolate;
    const uint8_t* const data;
    const size_t size;
    bool isInvalid;
    size_t offset;
};