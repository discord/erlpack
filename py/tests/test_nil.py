from erlpack import pack, ErlangTermEncoder


def test_nil():
    assert pack(None) == '\x83s\x03nil'


def test_custom_nil():

    assert ErlangTermEncoder(none_atom='none').pack(None) == '\x83s\x04none'
    assert ErlangTermEncoder(none_atom='null').pack(None) == '\x83s\x04null'
