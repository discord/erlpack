from __future__ import absolute_import
from erlpack import pack
from erlpack.types import Atom


class User(object):
    def __init__(self, name, age):
        self.name = name
        self.age = age

    def __erlpack__(self):
        return {
            Atom('name'): self.name,
            Atom('age'): self.age
        }


def test_erlpack_magic_method():
    u = User('jake', 23)
    packed = pack(u)

    # Could be packed in either order
    assert packed == b'\x83t\x00\x00\x00\x02s\x03agea\x17s\x04namem\x00\x00\x00\x04jake' \
        or packed == b'\x83t\x00\x00\x00\x02s\x04namem\x00\x00\x00\x04jakes\x03agea\x17'
