"""
Unpacker based on work from Samuel Stauffer's `python-erlastic` library. See COPYING.
"""

from __future__ import division
import struct
import zlib
from .constants import *
from .types import *
from cpython cimport *
import cython

__all__ = ['ErlangTermDecoder', 'ErlangTermDecodeError']


class ErlangTermDecodeError(Exception):
    pass


# noinspection PyMethodMayBeStatic,PyPep8Naming,PyShadowingBuiltins,PyUnusedLocal
cdef class ErlangTermDecoder(object):
    cdef object encoding
    def __init__(self, encoding=None):
        self.encoding = encoding

    def loads(self, bytes, offset=0):
        version = ord(bytes[offset:offset+1])
        if version != FORMAT_VERSION:
            raise ErlangTermDecodeError('Bad version number. Expected %d found %d' % (FORMAT_VERSION, version))
        return self.decode_part(bytes, offset + 1)[0]

    cdef object decode_part(self, bytes, offset=0):
        opcode = bytes[offset:offset+1]

        if opcode == b'a':
            return self.decode_a(bytes, offset + 1)

        elif opcode == b'b':
            return self.decode_b(bytes, offset + 1)

        elif opcode == b'c':
            return self.decode_c(bytes, offset + 1)

        elif opcode == b'F':
            return self.decode_F(bytes, offset + 1)

        elif opcode == b'd':
            return self.decode_d(bytes, offset + 1)

        elif opcode == b's':
            return self.decode_s(bytes, offset + 1)

        elif opcode == b'v':
            return self.decode_v(bytes, offset + 1)

        elif opcode == b'w':
            return self.decode_w(bytes, offset + 1)

        elif opcode == b't':
            return self.decode_t(bytes, offset + 1)

        elif opcode == b'h':
            return self.decode_h(bytes, offset + 1)

        elif opcode == b'i':
            return self.decode_i(bytes, offset + 1)

        elif opcode == b'j':
            return self.decode_j(bytes, offset + 1)

        elif opcode == b'k':
            return self.decode_k(bytes, offset + 1)

        elif opcode == b'l':
            return self.decode_l(bytes, offset + 1)

        elif opcode == b'm':
            return self.decode_m(bytes, offset + 1)

        elif opcode == b'n':
            return self.decode_n(bytes, offset + 1)

        elif opcode == b'o':
            return self.decode_o(bytes, offset + 1)

        elif opcode == b't':
            return self.decode_t(bytes, offset + 1)

        elif opcode == b'e':
            return self.decode_e(bytes, offset + 1)

        elif opcode == b'r':
            return self.decode_r(bytes, offset + 1)

        elif opcode == b'f':
            return self.decode_f(bytes, offset + 1)

        elif opcode == b'g':
            return self.decode_g(bytes, offset + 1)

        elif opcode == b'q':
            return self.decode_q(bytes, offset + 1)

        elif opcode == b'P':
            return self.decode_P(bytes, offset + 1)

        else:
            raise ValueError('Unexpected opcode %s' % opcode)

    cdef object decode_a(self, bytes, offset):
        """SMALL_INTEGER_EXT"""
        return ord(bytes[offset:offset+1]), offset + 1

    cdef object decode_b(self, bytes, offset):
        """INTEGER_EXT"""
        return struct.unpack('>l', bytes[offset:offset + 4])[0], offset + 4

    cdef object decode_c(self, bytes, offset):
        """FLOAT_EXT"""
        return float(bytes[offset:offset + 31].split('\x00', 1)[0]), offset + 31

    cdef object decode_F(self, bytes, offset):
        """NEW_FLOAT_EXT"""
        return struct.unpack('>d', bytes[offset:offset + 8])[0], offset + 8

    cdef object decode_d(self, bytes, offset):
        """ATOM_EXT"""
        atom_len, = struct.unpack('>H', bytes[offset:offset + 2])
        offset += 2
        atom = bytes[offset:offset + atom_len]
        return self.convert_atom(atom), offset + atom_len

    cdef object decode_s(self, bytes, offset):
        """SMALL_ATOM_EXT"""
        atom_len = ord(bytes[offset:offset+1])
        offset += 1
        atom = bytes[offset:offset + atom_len]
        return self.convert_atom(atom), offset + atom_len

    cdef object decode_v(self, bytes, offset):
        """ATOM_UTF8_EXT"""
        atom_len, = struct.unpack('>H', bytes[offset:offset + 2])
        offset += 2
        atom = bytes[offset:offset + atom_len]
        return self.convert_atom(atom, encoding='utf-8'), offset + atom_len

    cdef object decode_w(self, bytes, offset):
        """SMALL_ATOM_UTF8_EXT"""
        atom_len = ord(bytes[offset:offset+1])
        offset += 1
        atom = bytes[offset:offset + atom_len]
        return self.convert_atom(atom, encoding='utf-8'), offset + atom_len

    cdef object decode_t(self, bytes, offset):
        """MAP_EXT"""
        arity, = struct.unpack('>L', bytes[offset:offset + 4])
        offset += 4
        kv = {}
        for i in xrange(arity):
            key, offset = self.decode_part(bytes, offset)
            value, offset = self.decode_part(bytes, offset)
            kv[key] = value
        return kv, offset

    cdef object decode_h(self, bytes, offset):
        """SMALL_TUPLE_EXT"""
        arity = ord(bytes[offset:offset+1])
        offset += 1
        items = []
        for i in xrange(arity):
            val, offset = self.decode_part(bytes, offset)
            items.append(val)
        return tuple(items), offset

    cdef object decode_i(self, bytes, offset):
        """LARGE_TUPLE_EXT"""
        arity, = struct.unpack('>L', bytes[offset:offset + 4])
        offset += 4
        items = []
        for i in xrange(arity):
            val, offset = self.decode_part(bytes, offset)
            items.append(val)
        return tuple(items), offset

    cdef object decode_j(self, bytes, offset):
        """NIL_EXT"""
        return [], offset

    cdef object decode_k(self, bytes, offset):
        """STRING_EXT"""
        length, = struct.unpack('>H', bytes[offset:offset + 2])
        offset += 2
        st = bytes[offset:offset + length]
        byte_elements_are_ints = isinstance(b'a'[0], int)
        if byte_elements_are_ints:
            st = list(st)
        else:
            st = [ord(x) for x in st]
        return st, offset + length

    cdef object decode_l(self, bytes, offset):
        """LIST_EXT"""
        length, = struct.unpack('>L', bytes[offset:offset + 4])
        offset += 4
        items = []
        for i in xrange(length):
            val, offset = self.decode_part(bytes, offset)
            items.append(val)
        tail, offset = self.decode_part(bytes, offset)
        # noinspection PySimplifyBooleanCheck
        if tail != []:
            # TODO: Not sure what to do with the tail
            raise NotImplementedError('Lists with non empty tails are not supported')
        return items, offset

    cdef object decode_m(self, bytes, offset):
        """BINARY_EXT"""
        length, = struct.unpack('>L', bytes[offset:offset + 4])
        offset += 4
        rv = bytes[offset:offset + length]
        if self.encoding:
            rv = rv.decode(self.encoding)
        return rv, offset + length

    cdef object decode_n(self, bytes, offset):
        """SMALL_BIG_EXT"""
        n = ord(bytes[offset:offset+1])
        offset += 1
        return self.decode_bigint(n, bytes, offset)

    cdef object decode_o(self, bytes, offset):
        """LARGE_BIG_EXT"""
        n, = struct.unpack('>L', bytes[offset:offset + 4])
        offset += 4
        return self.decode_bigint(n, bytes, offset)

    @cython.boundscheck(False)
    cdef object decode_bigint(self, n, bytes, unsigned int offset):
        cdef unsigned char* cd = <unsigned char*>PyBytes_AsString(bytes)
        cdef unsigned long long ull
        cdef unsigned char pos = 0

        if offset + 1 + n > PyBytes_Size(bytes):
            raise OverflowError("Overflown! %s %s" % (offset + 1 + n, len(bytes)))

        sign = cd[offset]
        offset += 1

        if sign == 0 and n <= 8:
            ull = 0
            for i in range(n):
                ull |= <unsigned long long>(cd[offset]) << pos
                pos += 8
                offset += 1

            return ull, offset

        val = 0
        b = 1
        for i in range(n):
            val += cd[offset] * b
            b <<= 8
            offset += 1
        if sign != 0:
            val = -val
        return val, offset

    cdef object decode_e(self, bytes, offset):
        """REFERENCE_EXT"""
        node, offset = self.decode_part(bytes, offset)
        if not isinstance(node, Atom):
            raise ErlangTermDecodeError('Expected atom while parsing REFERENCE_EXT, found %r instead' % node)
        reference_id, creation = struct.unpack('>LB', bytes[offset: offset + 5])
        offset += 5
        return Reference(node, [reference_id], creation), offset

    cdef object decode_r(self, bytes, offset):
        """NEW_REFERENCE_EXT"""
        id_len, = struct.unpack('>H', bytes[offset:offset + 2])
        offset += 2
        node, offset = self.decode_part(bytes, offset)
        if not isinstance(node, Atom):
            raise ErlangTermDecodeError('Expected atom while parsing NEW_REFERENCE_EXT, found %r instead' % node)
        creation = ord(bytes[offset])
        offset += 1
        reference_id = struct.unpack('>%dL' % id_len, bytes[offset:offset + (4 * id_len)])
        offset += (4 * id_len)
        return Reference(node, reference_id, creation), offset

    cdef object decode_f(self, bytes, offset):
        """PORT_EXT"""
        node, offset = self.decode_part(bytes, offset)
        if not isinstance(node, Atom):
            raise ErlangTermDecodeError('Expected atom while parsing PORT_EXT, found %r instead' % node)
        port_id, creation = struct.unpack('>LB', bytes[offset:offset + 5])
        offset += 5
        return Port(node, port_id, creation), offset

    cdef object decode_g(self, bytes, offset):
        """PID_EXT"""
        node, offset = self.decode_part(bytes, offset)
        if not isinstance(node, Atom):
            raise ErlangTermDecodeError('Expected atom while parsing PID_EXT, found %r instead' % node)
        pid_id, serial, creation = struct.unpack('>LLB', bytes[offset:offset + 9])
        offset += 9
        return PID(node, pid_id, serial, creation), offset

    cdef object decode_q(self, bytes, offset):
        """EXPORT_EXT"""
        module, offset = self.decode_part(bytes, offset)
        if not isinstance(module, Atom):
            raise ErlangTermDecodeError('Expected atom while parsing EXPORT_EXT, found %r instead' % module)
        function, offset = self.decode_part(bytes, offset)
        if not isinstance(function, Atom):
            raise ErlangTermDecodeError('Expected atom while parsing EXPORT_EXT, found %r instead' % function)
        arity, offset = self.decode_part(bytes, offset)
        if not isinstance(arity, int):
            raise ErlangTermDecodeError('Expected integer while parsing EXPORT_EXT, found %r instead' % arity)
        return Export(module, function, arity), offset + 1

    cdef object decode_P(self, bytes, offset):
        """Compressed term"""
        usize, = struct.unpack('>L', bytes[offset:offset + 4])
        offset += 4
        bytes = zlib.decompress(bytes[offset:offset + usize])
        return self.decode_part(bytes, 0)

    cdef object convert_atom(self, atom, encoding='latin1'):
        """Convert an atom (bytes) into an appropriate Python object,
        using specified encoding. The default encoding is latin1, which
        is the old-style encoding."""
        if atom == b'true':
            return True
        elif atom == b'false':
            return False
        elif atom == b'nil':
            return None
        return Atom(atom.decode(encoding))
