from __future__ import absolute_import
import datetime

from pytest import raises

from erlpack import ErlangTermEncoder

class A:
    pass

class HookWrapper:
    def __init__(self):
        self.encoder = ErlangTermEncoder()

    def encode_hook(self, obj):
        if isinstance(obj, A):
            return self.encoder.pack("Encoding an A")
        

def test_hook_reentrancy():
    wrapper = HookWrapper()
    encoder = ErlangTermEncoder(encode_hook=wrapper.encode_hook)

    assert encoder.pack(A()) != None

    wrapper.encoder = encoder

    with raises(RuntimeError):
        encoder.pack(A())

    # even if we errored out, the encoder should return to
    # functional afterward
    assert encoder.pack("hello") != None
