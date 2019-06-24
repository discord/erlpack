from __future__ import absolute_import
from __future__ import print_function
from erlpack import pack


def test_string():
    atm = b'hello world'
    assert pack(atm) == b'\x83m\x00\x00\x00\x0bhello world'


def test_string_null_byte():
    null_byte = b'hello\x00 world'
    assert pack(null_byte) == b'\x83m\x00\x00\x00\x0chello\x00 world'
