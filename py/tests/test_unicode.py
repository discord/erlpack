# -*- coding: utf-8 -*-

from __future__ import absolute_import
from erlpack import Atom, ErlangTermDecoder, pack, unpack


def test_unicode():
    atm = u'hello world'
    assert pack(atm) == b'\x83m\x00\x00\x00\x0bhello world'


def test_unicode_with_actual_unicode_chars():
    atm = u'hello world\u202e'
    assert pack(atm) == b'\x83m\x00\x00\x00\x0ehello world\xe2\x80\xae'


def test_unicode_atom_encode_raises():
    # ATM, we only allow packing latin-1 encoded Atoms because the underlying
    # library sends ATOM_EXT instead of ATOM_UTF8_EXT. Update this test when
    # we are ready to start sending UTF-8 atoms.
    atm = Atom(u'こんにちは世界') # hello world
    try:
        pack(atm)
        raise Exception('did not raise UnicodeEncodeError')
    except UnicodeEncodeError:
        pass


def test_unicode_atom_decodes():
    atm = unpack(b'\x83w\x15\xe3\x81\x93\xe3\x82\x93\xe3\x81\xab\xe3\x81\xa1\xe3\x81\xaf\xe4\xb8\x96\xe7\x95\x8c')
    assert atm == Atom(u'こんにちは世界')


def test_unicode_string_decodes_if_encoding_set():
    unicode_string = u'こんにちは世界'
    decoder = ErlangTermDecoder(encoding='utf-8')
    packed = pack(unicode_string)
    assert decoder.loads(packed) == unicode_string
