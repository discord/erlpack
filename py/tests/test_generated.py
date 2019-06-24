from __future__ import absolute_import
from erlpack import Atom, pack, unpack


def test_pack_string():
    assert pack('string') == b'\x83m\x00\x00\x00\x06string'


def test_unpack_string():
    assert unpack(b'\x83m\x00\x00\x00\x06string') == b'string'


def test_pack_empty_list():
    assert pack([]) == b'\x83j'


def test_unpack_empty_list():
    assert unpack(b'\x83j') == []


def test_pack_large_int():
    assert pack(127552384489488384) == b'\x83n\x08\x00\x00\x00\xc0\xc77(\xc5\x01'


def test_unpack_large_int():
    assert unpack(b'\x83n\x08\x00\x00\x00\xc0\xc77(\xc5\x01') == 127552384489488384


def test_pack_basic_atom():
    assert pack(Atom('hi')) == b'\x83s\x02hi'


def test_unpack_basic_atom():
    assert unpack(b'\x83d\x00\x02hi') == Atom('hi')


def test_pack_kitchen_sink():
    assert pack([Atom('someatom'), (Atom('some'), Atom('other'), 'tuple'), ['maybe', 1, []], ('with', (Atom('embedded'), ['tuples and lists']), None), 127542384389482384, 5334.32, 102, -1394, -349.2, -498384595043, [{Atom('a'): 'map', Atom('also'): ('tuples', ['and'], ['lists']), Atom('with'): 'binaries'}, {Atom('a'): 'anotherone', 3: 'int keys'}], {(Atom('something'),): 'else'}]) == b'\x83l\x00\x00\x00\x0cs\x08someatomh\x03s\x04somes\x05otherm\x00\x00\x00\x05tuplel\x00\x00\x00\x03m\x00\x00\x00\x05maybea\x01jjh\x03m\x00\x00\x00\x04withh\x02s\x08embeddedl\x00\x00\x00\x01m\x00\x00\x00\x10tuples and listsjs\x03niln\x08\x00\x90gWs\x1f\x1f\xc5\x01F@\xb4\xd6Q\xeb\x85\x1e\xb8afb\xff\xff\xfa\x8eF\xc0u\xd333333n\x05\x01ch\t\ntl\x00\x00\x00\x02t\x00\x00\x00\x03s\x01am\x00\x00\x00\x03maps\x04alsoh\x03m\x00\x00\x00\x06tuplesl\x00\x00\x00\x01m\x00\x00\x00\x03andjl\x00\x00\x00\x01m\x00\x00\x00\x05listsjs\x04withm\x00\x00\x00\x08binariest\x00\x00\x00\x02s\x01am\x00\x00\x00\nanotheronea\x03m\x00\x00\x00\x08int keysjt\x00\x00\x00\x01h\x01s\tsomethingm\x00\x00\x00\x04elsej'


def test_unpack_kitchen_sink():
    assert unpack(b'\x83l\x00\x00\x00\x0cd\x00\x08someatomh\x03d\x00\x04somed\x00\x05otherm\x00\x00\x00\x05tuplel\x00\x00\x00\x03m\x00\x00\x00\x05maybea\x01jjh\x03m\x00\x00\x00\x04withh\x02d\x00\x08embeddedl\x00\x00\x00\x01m\x00\x00\x00\x10tuples and listsjd\x00\x03niln\x08\x00\x90gWs\x1f\x1f\xc5\x01F@\xb4\xd6Q\xeb\x85\x1e\xb8afb\xff\xff\xfa\x8eF\xc0u\xd333333n\x05\x01ch\t\ntl\x00\x00\x00\x02t\x00\x00\x00\x03d\x00\x01am\x00\x00\x00\x03mapd\x00\x04alsoh\x03m\x00\x00\x00\x06tuplesl\x00\x00\x00\x01m\x00\x00\x00\x03andjl\x00\x00\x00\x01m\x00\x00\x00\x05listsjd\x00\x04withm\x00\x00\x00\x08binariest\x00\x00\x00\x02a\x03m\x00\x00\x00\x08int keysd\x00\x01am\x00\x00\x00\nanotheronejt\x00\x00\x00\x01h\x01d\x00\tsomethingm\x00\x00\x00\x04elsej') == [Atom('someatom'), (Atom('some'), Atom('other'), b'tuple'), [b'maybe', 1, []], (b'with', (Atom('embedded'), [b'tuples and lists']), None), 127542384389482384, 5334.32, 102, -1394, -349.2, -498384595043, [{Atom('a'): b'map', Atom('also'): (b'tuples', [b'and'], [b'lists']), Atom('with'): b'binaries'}, {Atom('a'): b'anotherone', 3: b'int keys'}], {(Atom('something'),): b'else'}]


def test_pack_float():
    assert pack(123.45) == b'\x83F@^\xdc\xcc\xcc\xcc\xcc\xcd'


def test_unpack_float():
    assert unpack(b'\x83F@^\xdc\xcc\xcc\xcc\xcc\xcd') == 123.45


def test_pack_binary():
    assert pack('alsdjaljf') == b'\x83m\x00\x00\x00\talsdjaljf'


def test_unpack_binary():
    assert unpack(b'\x83m\x00\x00\x00\talsdjaljf') == b'alsdjaljf'


def test_pack_int():
    assert pack(12345) == b'\x83b\x00\x0009'


def test_unpack_int():
    assert unpack(b'\x83b\x00\x0009') == 12345


def test_pack_empty_dictionary():
    assert pack(()) == b'\x83h\x00'


def test_unpack_empty_dictionary():
    assert unpack(b'\x83h\x00') == ()

