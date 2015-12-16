from erlpack import pack


def test_false():
    assert pack(False) == '\x83s\x05false'
