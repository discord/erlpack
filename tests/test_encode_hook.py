import datetime

from pytest import raises

from erlpack import ErlangTermEncoder


def test_encode_hook():
    def encode_hook(obj):
        if isinstance(obj, datetime.datetime):
            return obj.isoformat()

    encoder = ErlangTermEncoder(encode_hook=encode_hook)

    assert encoder.pack(datetime.datetime(2015, 12, 25, 12, 23, 55)) == '\x83m\x00\x00\x00\x132015-12-25T12:23:55'

    with raises(NotImplementedError):
        encoder.pack(datetime.date.today())
