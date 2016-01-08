from erlpack import pack


def test_string():
    atm = 'hello world'
    assert pack(atm) == '\x83m\x00\x00\x00\x0bhello world'


def test_string_null_byte():
    null_byte = 'hello\x00 world'
    print "null_byte", null_byte
    assert pack(null_byte) == '\x83m\x00\x00\x00\x0chello\x00 world'
