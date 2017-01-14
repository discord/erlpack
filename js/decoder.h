#pragma once

#include <nan.h>
#include <zlib.h>
#include <cstdio>

#include "../cpp/sysdep.h"

using namespace v8;

#define THROW(msg) Nan::ThrowError(msg); isInvalid = true; printf("[Error %s:%d] %s\n", __FILE__, __LINE__, msg)

class Decoder {
public:
    Decoder(const Nan::TypedArrayContents<uint8_t>& array)
    : data(*array)
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

   Decoder(const uint8_t* data_, size_t length_, bool skipVersion = false)
    : data(data_)
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
            THROW("Reading a byte passes the end of the buffer.");
            return 0;
        }
        auto val = *reinterpret_cast<const uint8_t*>(data + offset);
        offset += sizeof(uint8_t);
        return val;
    }

    uint16_t read16() {
        if (offset + sizeof(uint16_t) > size) {
            THROW("Reading two bytes passes the end of the buffer.");
            return 0;
        }

        uint16_t val = _erlpack_be16(*reinterpret_cast<const uint16_t*>(data + offset));
        offset += sizeof(uint16_t);
        return val;
    }

    uint32_t read32() {
        if (offset + sizeof(uint32_t) > size) {
            THROW("Reading three bytes passes the end of the buffer.");
            return 0;
        }

        uint32_t val = _erlpack_be32(*reinterpret_cast<const uint32_t*>(data + offset));
        offset += sizeof(uint32_t);
        return val;
    }

    uint64_t read64() {
        if (offset + sizeof(uint64_t) > size) {
            THROW("Reading four bytes passes the end of the buffer.");
            return 0;
        }

        uint64_t val = _erlpack_be64(*reinterpret_cast<const uint64_t*>(data + offset));
        offset += sizeof(val);
        return val;
    }

    Local<Value> decodeSmallInteger() {
        return Nan::New<Integer>(read8());
    }

    Local<Value> decodeInteger() {
        return Nan::New<Integer>((int32_t)read32());
    }

    Local<Value> decodeArray(uint32_t length) {
        Local<Object> array = Nan::New<Array>(length);
        for(uint32_t i = 0; i < length; ++i) {
            auto value = unpack();
            if (isInvalid) {
                return Nan::Undefined();
            }
            array->Set(i, value);
        }
        return array;
    }

    Local<Value> decodeList() {
        const uint32_t length = read32();
        auto array = decodeArray(length);

        const auto tailMarker = read8();
        if (tailMarker != NIL_EXT) {
            THROW("List doesn't end with a tail marker, but it must!");
            return Nan::Null();
        }

        return array;
    }

    Local<Value> decodeTuple(uint32_t length) {
        return decodeArray(length);
    }

    Local<Value> decodeNil() {
        Local<Object> array = Nan::New<Array>(0);
        return array;
    }

    Local<Value> decodeMap() {
        const uint32_t length = read32();
        auto map = Nan::New<Object>();

        for(uint32_t i = 0; i < length; ++i) {
            const auto key = unpack();
            const auto value = unpack();
            if (isInvalid) {
                return Nan::Undefined();
            }
            map->Set(key, value);
        }

        return map;
    }

    const char* readString(uint32_t length) {
        if (offset + length > size) {
            THROW("Reading sequence past the end of the buffer.");
            return NULL;
        }

        const uint8_t* str = data + offset;
        offset += length;
        return (const char*)str;
    }

    Local<Value> processAtom(const char* atom, uint16_t length) {
        if (atom == NULL) {
            return Nan::Undefined();
        }

        if (length >= 3 && length <= 5) {
            if (length == 3 && strncmp(atom, "nil", 3) == 0) {
                return Nan::Null();
            }
            else if (length == 4 && strncmp(atom, "null", 4) == 0) {
                return Nan::Null();
            }
            else if(length == 4 && strncmp(atom, "true", 4) == 0) {
                return Nan::True();
            }
            else if (length == 5 && strncmp(atom, "false", 5) == 0) {
                return Nan::False();
            }
        }

        return Nan::New(atom, length).ToLocalChecked();
    }

    Local<Value> decodeAtom() {
        auto length = read16();
        const char* atom = readString(length);
        return processAtom(atom, length);
    }

    Local<Value> decodeSmallAtom() {
        auto length = read8();
        const char* atom = readString(length);
        return processAtom(atom, length);
    }

    Local<Value> decodeFloat() {
        const uint8_t FLOAT_LENGTH = 31;
        const char* floatStr = readString(FLOAT_LENGTH);
        if (floatStr == NULL) {
            return Nan::Undefined();
        }

        double number;
        char nullTerimated[FLOAT_LENGTH + 1] = {0};
        memcpy(nullTerimated, floatStr, FLOAT_LENGTH);

        auto count = sscanf(nullTerimated, "%lf", &number);
        if (count != 1) {
            THROW("Invalid float encoded.");
            return Nan::Null();
        }

        return Nan::New<Number>(number);
    }

    Local<Value> decodeNewFloat() {
        union {
            uint64_t ui64;
            double df;
        } val;
        val.ui64 = read64();
        return Nan::New<Number>(val.df);
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
                return Nan::New<Integer>(static_cast<uint32_t>(value));
            }

            const bool isSignBitAvailable = (value & (1 << 31)) == 0;
            if (isSignBitAvailable) {
                int32_t negativeValue = -static_cast<int32_t>(value);
                return Nan::New<Integer>(negativeValue);
            }
        }

        char outBuffer[32] = {0}; // 9223372036854775807
        const char* const formatString = sign == 0 ? "%llu" : "-%llu";
        const int res = sprintf(outBuffer, formatString, value);

        if (res < 0) {
            THROW("Unable to convert big int to string");
            return Nan::Null();
        }
        const uint8_t length = static_cast<const uint8_t>(res);

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

    Local<Value> decodeBinaryAsString() {
        const auto length = read32();
        const char* str = readString(length);
        if (str == NULL) {
            return Nan::Undefined();
        }
        auto binaryString = Nan::New(str, length);
        return binaryString.ToLocalChecked();
    }

    Local<Value> decodeString() {
        const auto length = read16();
        const char* str = readString(length);
        if (str == NULL) {
            return Nan::Undefined();
        }
        auto binaryString = Nan::New(str, length);
        return binaryString.ToLocalChecked();
    }

    Local<Value> decodeStringAsList() {
        const auto length = read16();
        if (offset + length > size) {
            THROW("Reading sequence past the end of the buffer.");
            return Nan::Null();
        }

        Local<Object> array = Nan::New<Array>(length);
        for(uint16_t i = 0; i < length; ++i) {
            array->Set(i, decodeSmallInteger());
        }
        
        return array;
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
        uint8_t* outBuffer = (uint8_t*)malloc(uncompressedSize);
        const int ret = uncompress(outBuffer, &sourceSize, (const unsigned char*)(data + offset), (uLong)(size - offset));

        offset += sourceSize;
        if (ret != Z_OK) {
            free(outBuffer);
            THROW("Failed to uncompresss compressed item");
            return Nan::Null();
        }

        Decoder children(outBuffer, uncompressedSize, true);
        Nan::MaybeLocal<Value> value = children.unpack();
        free(outBuffer);
        return value.ToLocalChecked();
    }

    Local<Value> decodeReference() {
        auto reference = Nan::New<Object>();
        reference->Set(Nan::New("node").ToLocalChecked(), unpack());

        Local<Object> ids = Nan::New<Array>(1);
        ids->Set(0, Nan::New<Integer>(read32()));
        reference->Set(Nan::New("id").ToLocalChecked(), ids);

        reference->Set(Nan::New("creation").ToLocalChecked(), Nan::New<Integer>(read8()));

        return reference;
    }

    Local<Value> decodeNewReference() {
        auto reference = Nan::New<Object>();

        uint16_t len = read16();
        reference->Set(Nan::New("node").ToLocalChecked(), unpack());
        reference->Set(Nan::New("creation").ToLocalChecked(), Nan::New<Integer>(read8()));

        Local<Object> ids = Nan::New<Array>(len);
        for(uint16_t i = 0; i < len; ++i) {
            ids->Set(i, Nan::New<Integer>(read32()));
        }
        reference->Set(Nan::New("id").ToLocalChecked(), ids);

        return reference;
    }

    Local<Value> decodePort() {
        auto port = Nan::New<Object>();
        port->Set(Nan::New("node").ToLocalChecked(), unpack());
        port->Set(Nan::New("id").ToLocalChecked(), Nan::New<Integer>(read32()));
        port->Set(Nan::New("creation").ToLocalChecked(), Nan::New<Integer>(read8()));
        return port;
    }

    Local<Value> decodePID() {
        auto pid = Nan::New<Object>();
        pid->Set(Nan::New("node").ToLocalChecked(), unpack());
        pid->Set(Nan::New("id").ToLocalChecked(), Nan::New<Integer>(read32()));
        pid->Set(Nan::New("serial").ToLocalChecked(), Nan::New<Integer>(read32()));
        pid->Set(Nan::New("creation").ToLocalChecked(), Nan::New<Integer>(read8()));
        return pid;
    }

    Local<Value> decodeExport() {
        auto exp = Nan::New<Object>();
        exp->Set(Nan::New("mod").ToLocalChecked(), unpack());
        exp->Set(Nan::New("fun").ToLocalChecked(), unpack());
        exp->Set(Nan::New("arity").ToLocalChecked(), unpack());
        return exp;
    }

    Local<Value> unpack() {
        if (isInvalid) {
            return Nan::Undefined();
        }

        if(offset >= size) {
            THROW("Unpacking beyond the end of the buffer");
            return Nan::Undefined();
        }

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
                return decodeStringAsList();
            case LIST_EXT:
                return decodeList();
            case MAP_EXT:
                return decodeMap();
            case BINARY_EXT:
                return decodeBinaryAsString();
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
                return Nan::Undefined();
        }

        return Nan::Undefined();
    }
private:
    const uint8_t* const data;
    const size_t size;
    bool isInvalid;
    size_t offset;
};
