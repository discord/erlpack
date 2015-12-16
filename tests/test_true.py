from erlpack import pack


def test_true():
    assert pack(True) == '\x83s\x04true'
