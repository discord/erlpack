from erlpack import pack


def test_list():
    assert pack([1, "two", 3.0, "four", ['five']]) == (
        '\x83l\x00\x00\x00\x05a\x01m\x00\x00\x00\x03twoF@\x08\x00\x00\x00\x00\x00\x00m\x00\x00\x00\x04fourl\x00\x00'
        '\x00\x01m\x00\x00\x00\x04fivejj'
    )


def test_empty_list():
    assert pack([]) == '\x83j'
