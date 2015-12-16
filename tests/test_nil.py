from erlpack import pack


def test_nil():
    assert pack(None) == '\x83d\x00\x03nil'
