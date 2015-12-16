from erlpack import pack


def test_dict():
    assert pack({'a': 1, 2: 2, 3: [1, 2, 3]}) == \
           '\x83t\x00\x00\x00\x03m\x00\x00\x00\x01aa\x01a\x02a\x02a\x03l\x00\x00\x00\x03a\x01a\x02a\x03j'


def test_userdict():
    items_called = [False]

    class UserDict(dict):
        def items(self):
            items_called[0] = True
            return super(UserDict, self).items()

    assert pack(UserDict({'a': 1, 2: 2, 3: [1, 2, 3]})) == \
           '\x83t\x00\x00\x00\x03m\x00\x00\x00\x01aa\x01a\x02a\x02a\x03l\x00\x00\x00\x03a\x01a\x02a\x03j'

    assert items_called[0]
