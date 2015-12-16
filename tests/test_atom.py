from erlpack.types import Atom
from erlpack import pack


def test_atom():
    atm = Atom('hello world')

    assert pack(atm) == '\x83d\x00\x0bhello world'
