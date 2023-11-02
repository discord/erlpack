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

def test_charlist_unpack():
    assert unpack(b'\x83k\x00\x04\x01\x02\x03\x04') == [1, 2, 3, 4]

def test_charlist_unpack_large_numbers():
    assert unpack(b'\x83l\x00\x00\x00\x04b\x00\x00\x04\x00b\x00\x00\x04\x01b\x00\x00\x04\x02b\x00\x00\x04\x03j') == [1024, 1025, 1026, 1027]