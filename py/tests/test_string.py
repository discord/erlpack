from __future__ import absolute_import
from __future__ import print_function
from erlpack import pack, unpack


def test_string():
    atm = 'hello world'
    assert pack(atm) == b'\x83m\x00\x00\x00\x0bhello world'


def test_string_null_byte():
    null_byte = 'hello\x00 world'
    assert pack(null_byte) == b'\x83m\x00\x00\x00\x0chello\x00 world'


def test_string_ext_unpack():
    assert unpack(b'\x83k\x0b\x00hello world') == \
        [104, 101, 108, 108, 111, 32, 119, 111, 114, 108, 100]
