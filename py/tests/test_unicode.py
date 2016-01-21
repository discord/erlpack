from erlpack import pack


def test_unicode():
    atm = u'hello world'

    assert pack(atm) == '\x83m\x00\x00\x00\x0bhello world'


def test_unicode_with_actual_unicode_chars():
    atm = u'hello world\u202e'

    assert pack(atm) == '\x83m\x00\x00\x00\x0ehello world\xe2\x80\xae'
