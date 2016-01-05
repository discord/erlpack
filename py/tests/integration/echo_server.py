#!/usr/bin/env python

from erlpack import pack, unpack
import socket
import struct
from string import Template

PORT = 5001
HOST = ''
BACKLOG = 5

file_tmpl = Template("""
from erlpack import Atom, pack, unpack

${testcases}
""".strip())

testcase_pack_tmpl = Template("""
def test_pack_${name}():
    assert pack(${python_rep}) == ${packed_rep}

""")

testcase_unpack_tmpl = Template("""
def test_unpack_${name}():
    assert unpack(${packed_rep}) == ${python_rep}

""")


def start_server():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
    s.bind((HOST, PORT))
    s.listen(BACKLOG)
    client, address = s.accept()
    clientfile = client.makefile()
    testcases = ''
    is_name = True
    name = ''
    while 1:
        size_str = clientfile.read(4)
        if not size_str:
            break
        else:
            (size,) = struct.unpack("!L", size_str)
            data_str = clientfile.read(size)
            term = unpack(data_str)
            encoded_term = pack(term)
            out_size = len(encoded_term)
            clientfile.write(struct.pack("!L", long(out_size)))
            clientfile.write(encoded_term)
            clientfile.flush()
            if is_name:
                name = term
            else:
                testcases += testcase_pack_tmpl.substitute(name=name,
                                                           python_rep=repr(term),
                                                           packed_rep=repr(encoded_term))
                testcases += testcase_unpack_tmpl.substitute(name=name,
                                                             python_rep=repr(term),
                                                             packed_rep=repr(data_str))

            is_name = not is_name

    with open('tests/test_generated.py', 'w') as f:
        f.write(file_tmpl.substitute(testcases=testcases))

    clientfile.close()
    client.close()

start_server()
