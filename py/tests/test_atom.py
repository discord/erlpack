from erlpack.types import Atom
from erlpack import pack


def test_small_atom():
    atm = Atom('hello world')

    assert pack(atm) == '\x83s\x0bhello world'


def test_large_atom():
    atm = Atom('test ' * 100)
    assert pack(atm) == (
        '\x83d\x01\xf4test test test test test test test test test test test test test test test test test test test '
        'test test test test test test test test test test test test test test test test test test test test test '
        'test test test test test test test test test test test test test test test test test test test test test '
        'test test test test test test test test test test test test test test test test test test test test test '
        'test test test test test test test test test test test test test test test test test test '
    )