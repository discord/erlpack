# Erlpack

Erlpack is a fast encoder and decoder for the Erlang Term Format (version 131) for Python and JavaScript.

# JavaScript

## Things that can be packed:

- [X] Null
- [X] Booleans
- [X] Strings
- [ ] Atoms
- [X] Unicode Strings
- [X] Floats
- [X] Integers
- [ ] Longs
- [ ] Longs over 64 bits
- [X] Objects
- [X] Arrays
- [ ] Tuples
- [ ] PIDs
- [ ] Ports
- [ ] Exports
- [ ] References

## How to pack:
```js
let erlpack = require("erlpack");

packed = erlpack.pack({'a': true, 'list': ['of', 3, 'things', 'to', 'pack']});
```

## How to unpack:
Note: Unpacking requires the binary data be a Uint8Array or Buffer. For those using electron/libchromium see the gotcha below. 
```js
let erlpack = require("erlpack");

let unpacked = null;
let packed = new Buffer('', 'binary');
try  {
    unpacked = erlpack.unpack(packed);
}
catch (e) {
    // got an exception parsing
}
```

## Libchromium / Electron Gotcha
Some versions of libchromium replace the native data type backing TypedArrays with a custom data type called 
blink::WebArrayBuffer. To keep erlpack' dependencies simple this data type is not supported directly. If you're using
Electron / Libchromium you need to convert the blink::WebArrayBuffer into a node::Buffer before passing to erlpack. You will
need to add this code into your native package somewhere:
```cpp
v8::Local<v8::Value> ConvertToNodeBuffer(const v8::Local<v8::Object>& blinkArray)
{
    if (node::Buffer::HasInstance(blinkArray)) {
        return blinkArray;
    }
    else if (blinkArray->IsArrayBufferView()) {
        auto byteArray = v8::ArrayBufferView::Cast(*blinkArray);
        return node::Buffer::Copy(v8::Isolate::GetCurrent(), (const char*)byteArray->Buffer()->GetContents().Data(), byteArray->ByteLength()).ToLocalChecked();
    }
    
    return v8::Local<v8::Primitive>(v8::Null(v8::Isolate::GetCurrent()));
}
```

Then in JavaScript something like:

```js
let packed = NativeUtils.convertToNodeBuffer(new Uint8Array(binaryPayload));
// unpack now using erlpack.unpack(packed)
```

# Python

## Things that can be packed:

- [X] None
- [X] Booleans
- [X] Strings
- [X] Atoms
- [X] Unicode Strings
- [X] Floats
- [X] Integers
- [X] Longs
- [ ] Longs over 64 bits
- [X] Dictionaries
- [X] Lists
- [X] Tuples
- [X] User Types (via an encode hook)
- [ ] PIDs
- [ ] Ports
- [ ] Exports
- [ ] References

## How to pack:
```py
from erlpack import pack

packed = pack(["thing", "to", "pack"])
```

## How to unpack:
```py
from erlpack import unpack

unpacked = unpack(packed)
```

## How to pack an atom:

```py
from erlpack import Atom, pack

packed = pack(Atom('hello'))
```

## How to use an encode hook.

```py
from erlpack import ErlangTermEncoder

def encode_hook(obj):
    if isinstance(obj, datetime.datetime):
        return obj.isoformat()

encoder = ErlangTermEncoder(encode_hook=encode_hook)
packed = encoder.pack(datetime.datetime(2015, 12, 25, 12, 23, 55))

```

## How to make custom types packable.

```py
from erlpack import pack, Atom

class User(object):
    def __init__(self, name, age):
        self.name = name
        self.age = age

    def __erlpack__(self):
        return {
            Atom('name'): self.name,
            Atom('age'): self.age
        }

u = User(name='Jake', age=23)
packed = pack(u)
```
