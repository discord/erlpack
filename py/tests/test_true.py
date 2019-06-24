from __future__ import absolute_import
from erlpack import pack


def test_true():
    assert pack(True) == b'\x83s\x04true'
