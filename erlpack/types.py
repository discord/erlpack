"""
Types based on work from Samuel Stauffer's `python-erlastic` library. See COPYING.
"""

__all__ = ['Atom', 'Reference', 'Port', 'PID', 'Export']


class Atom(str):
    def __repr__(self):
        return 'Atom(%s)' % super(Atom, self).__repr__()


class Reference(object):
    __slots__ = ['node', 'ref_id', 'creation']

    def __init__(self, node, ref_id, creation):
        if not isinstance(ref_id, tuple):
            ref_id = tuple(ref_id)

        self.node = node
        self.ref_id = ref_id
        self.creation = creation

    def __cmp__(self, other):
        if not isinstance(other, Reference):
            return 1
        if self.node == other.node and self.ref_id == other.ref_id and self.creation == other.creation:
            return 0
        return -1

    def __str__(self):
        return '#Ref<%d.%s>' % (self.creation, '.'.join(str(i) for i in self.ref_id))

    def __repr__(self):
        return '%s::%s' % (self, self.node)


class Port(object):
    __slots__ = ['node', 'port_id', 'creation']

    def __init__(self, node, port_id, creation):
        self.node = node
        self.port_id = port_id
        self.creation = creation

    def __cmp__(self, other):
        if not isinstance(other, Port):
            return 1
        if self.node == other.node and self.port_id == other.port_id and self.creation == other.creation:
            return 0
        return -1

    def __str__(self):
        return '#Port<%d.%d>' % (self.creation, self.port_id)

    def __repr__(self):
        return '%s::%s' % (self, self.node)


class PID(object):
    __slots__ = ['node', 'pid_id', 'serial', 'creation']

    def __init__(self, node, pid_id, serial, creation):
        self.node = node
        self.pid_id = pid_id
        self.serial = serial
        self.creation = creation

    def __cmp__(self, other):
        if not isinstance(other, PID):
            return 1
        if self.node == other.node and self.pid_id == other.pid_id and self.serial == other.serial and self.creation == other.creation:
            return 0
        return -1

    def __str__(self):
        return '<%d.%d.%d>' % (self.creation, self.pid_id, self.serial)

    def __repr__(self):
        return '%s::%s' % (self, self.node)


class Export(object):
    __slots__ = ['module', 'function', 'arity']

    def __init__(self, module, function, arity):
        self.module = module
        self.function = function
        self.arity = arity

    def __cmp__(self, other):
        if not isinstance(other, Export):
            return 1
        if self.module == other.module and self.function == other.function and self.arity == other.arity:
            return 0
        return -1

    def __str__(self):
        return '#Fun<%s.%s.%d>' % (self.module, self.function, self.arity)

    def __repr__(self):
        return str(self)
