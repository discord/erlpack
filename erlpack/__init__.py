from ._packer import ErlangTermEncoder
from ._unpacker import ErlangTermDecoder
from .types import Atom, Export, PID, Port, Reference

encoder = ErlangTermEncoder()
pack = encoder.pack

decoder = ErlangTermDecoder()
unpack = decoder.loads

__all__ = ['pack', 'unpack', 'Atom', 'Export', 'PID', 'Port', 'Reference', 'ErlangTermEncoder']
