from erlpack import pack


def test_nil():
    assert pack(None) == '\x83s\x03nil'
