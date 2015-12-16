from erlpack import pack


def test_true():
    assert pack(True) == '\x83d\x00\x04true'
