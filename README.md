# Erlpack

Erlpack is a fast encoder (and soon decoder) for the Erlang Term Format (version 131) for Python.

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