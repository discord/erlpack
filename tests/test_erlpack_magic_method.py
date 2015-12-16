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
    assert pack(u) == '\x83t\x00\x00\x00\x02s\x03agea\x17s\x04namem\x00\x00\x00\x04jake'
