from erlpack import pack


def test_string():
    atm = 'hello world'

    assert pack(atm) == '\x83m\x00\x00\x00\x0bhello world'
