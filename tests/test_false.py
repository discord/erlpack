from erlpack import pack


def test_false():
    assert pack(False) == '\x83d\x00\x05false'
