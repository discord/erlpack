from __future__ import absolute_import
from erlpack import pack


def test_nil():
    assert pack(None) == b'\x83s\x03nil'
