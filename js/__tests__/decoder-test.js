jest.dontMock('../index');

const erlpack = require('../index.js');

const helloWorldList = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11];
const helloWorldBinary = '\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B';

const helloWorldListWithNull = [1, 2, 3, 4, 5, 0, 6, 7, 8, 9, 10, 11];
const helloWorldBinaryWithNull = '\x01\x02\x03\x04\x05\x00\x06\x07\x08\x09\x0A\x0B';

describe('unpacks', () => {
    it('short list via string with null byte', () => {
        expect(erlpack.unpack(new Buffer('\x83k\x00\x0c' + helloWorldBinaryWithNull, 'binary'))).toEqual(helloWorldListWithNull);
    });

    it('short list via string without byte', () => {
        expect(erlpack.unpack(new Buffer('\x83k\x00\x0b' + helloWorldBinary, 'binary'))).toEqual(helloWorldList);
    });

    it('binary with null byte', () => {
        expect(erlpack.unpack(new Buffer('\x83m\x00\x00\x00\x0chello\x00 world', 'binary'))).toEqual('hello\x00 world');
    });

    it('binary without null byte', () => {
        expect(erlpack.unpack(new Buffer('\x83m\x00\x00\x00\x0bhello world', 'binary'))).toEqual('hello world');
    });

    it('dictionary', () => {
        const data = new Buffer(
            '\x83t\x00\x00\x00\x03a\x02a\x02a\x03l\x00\x00\x00\x03a\x01a\x02a\x03jm\x00\x00\x00\x01aa\x01',
            'binary'
        );
        const unpacked = erlpack.unpack(data);
        expect({'a': 1, 2: 2, 3: [1, 2, 3]}).toEqual(unpacked);
    });

    it('false', () => {
        expect(erlpack.unpack(new Buffer('\x83s\x05false', 'binary'))).toEqual(false);
    });

    it('true', () => {
        expect(erlpack.unpack(new Buffer('\x83s\x04true', 'binary'))).toEqual(true);
    });

    it('nil token is array', () => {
        expect(erlpack.unpack(new Buffer('\x83j', 'binary'))).toEqual([]);
    });

    it('nil atom is null', () => {
        expect(erlpack.unpack(new Buffer('\x83s\x03nil', 'binary'))).toBeNull();
    });

    it('null is null', () => {
        expect(erlpack.unpack(new Buffer('\x83s\x04null', 'binary'))).toBeNull();
    });

    it('floats', () => {
        expect(erlpack.unpack(new Buffer('\x83c2.50000000000000000000e+00\x00\x00\x00\x00\x00', 'binary'))).toEqual(2.5);
        expect(erlpack.unpack(new Buffer('\x83c5.15121238412343125000e+13\x00\x00\x00\x00\x00', 'binary'))).toEqual(51512123841234.31423412341435123412341342);
    });

    it('new floats', () => {
        expect(erlpack.unpack(new Buffer('\x83F\x40\x04\x00\x00\x00\x00\x00\x00', 'binary'))).toEqual(2.5);
        expect(erlpack.unpack(new Buffer('\x83F\x42\xC7\x6C\xCC\xEB\xED\x69\x28', 'binary'))).toEqual(51512123841234.31423412341435123412341342);
    });

    it('small int', () => {
        function check(small_int) {
            const expected = new Buffer(3);
            expected.write('\x83a', 0, 2, 'binary');
            expected.writeUInt8(small_int, 2);
            expect(erlpack.unpack(expected)).toEqual(small_int);
        }

        for(var i = 0; i < 256; ++i) {
            check(i);
        }
    });

    it('int32', () => {
        expect(erlpack.unpack(new Buffer('\x83b\x00\x00\x04\x00', 'binary'))).toEqual(1024);
        expect(erlpack.unpack(new Buffer('\x83b\x80\x00\x00\x00', 'binary'))).toEqual(-2147483648);
        expect(erlpack.unpack(new Buffer('\x83b\x7f\xff\xff\xff', 'binary'))).toEqual(2147483647);
    });

    it('small big ints', () => {
        expect(erlpack.unpack(new Buffer('\x83n\x04\x01\x01\x02\x03\x04', 'binary'))).toEqual(-67305985);
        expect(erlpack.unpack(new Buffer('\x83n\x04\x00\x01\x02\x03\x04', 'binary'))).toEqual(67305985);
        expect(erlpack.unpack(new Buffer('\x83n\x08\x01\x01\x02\x03\x04\x05\x06\x07\x08', 'binary'))).toEqual("-578437695752307201");
        expect(erlpack.unpack(new Buffer('\x83n\x08\x00\x01\x02\x03\x04\x05\x06\x07\x08', 'binary'))).toEqual("578437695752307201");
        expect(() => erlpack.unpack(new Buffer('\x83n\x0A\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A', 'binary'))).toThrow("Unable to decode big ints larger than 8 bytes");
    });

    it('large big ints', () => {
        expect(erlpack.unpack(new Buffer('\x83o\x00\x00\x00\x04\x01\x01\x02\x03\x04', 'binary'))).toEqual(-67305985);
        expect(erlpack.unpack(new Buffer('\x83o\x00\x00\x00\x04\x00\x01\x02\x03\x04', 'binary'))).toEqual(67305985);
        expect(erlpack.unpack(new Buffer('\x83o\x00\x00\x00\x08\x01\x01\x02\x03\x04\x05\x06\x07\x08', 'binary'))).toEqual("-578437695752307201");
        expect(erlpack.unpack(new Buffer('\x83o\x00\x00\x00\x08\x00\x01\x02\x03\x04\x05\x06\x07\x08', 'binary'))).toEqual("578437695752307201");
        expect(() => erlpack.unpack(new Buffer('\x83o\x00\x00\x00\x0A\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A', 'binary'))).toThrow("Unable to decode big ints larger than 8 bytes");
    });

    it('atoms', () => {
        expect(erlpack.unpack(new Buffer('\x83d\x00\x0Dguild members', 'binary'))).toEqual("guild members");
    });

    it('tuples', () => {
        expect(erlpack.unpack(new Buffer('\x83h\x03m\x00\x00\x00\x06vanisha\x01a\x04', 'binary'))).toEqual(['vanish', 1, 4]);
        expect(erlpack.unpack(new Buffer('\x83i\x00\x00\x00\x03m\x00\x00\x00\x06vanisha\x01a\x04', 'binary'))).toEqual(['vanish', 1, 4]);
    });

    it('compressed', () => {
        const expected = [2, Array.from("it's getting hot in here.").map(x => x.charCodeAt(0))];
        expect(erlpack.unpack(new Buffer('\x83l\x00\x00\x00\x02a\x02k\x00\x19it\'s getting hot in here.j', 'binary'))).toEqual(expected);
        expect(erlpack.unpack(new Buffer('\x83P\x00\x00\x00\x24\x78\x9C\xCB\x61\x60\x60\x60\x4A\x64\xCA\x66\x90\xCC\x2C\x51\x2F\x56\x48\x4F\x2D\x29\xC9\xCC\x4B\x57\xC8\xC8\x2F\x51\xC8\xCC\x53\xC8\x48\x2D\x4A\xD5\xCB\x02\x00\xA8\xA8\x0A\x9D', 'binary'))).toEqual(expected);
    });

    it('nested compressed', () => {
        const expected = [[2, Array.from("it's getting hot in here.").map(x => x.charCodeAt(0))], 3];
        expect(erlpack.unpack(new Buffer('\x83l\x00\x00\x00\x02l\x00\x00\x00\x02a\x02k\x00\x19it\'s getting hot in here.ja\x03j', 'binary'))).toEqual(expected);
        expect(erlpack.unpack(new Buffer('\x83P\x00\x00\x00\x2C\x78\x9C\xCB\x61\x60\x60\x60\xCA\x01\x11\x89\x4C\xD9\x0C\x92\x99\x25\xEA\xC5\x0A\xE9\xA9\x25\x25\x99\x79\xE9\x0A\x19\xF9\x25\x0A\x99\x79\x0A\x19\xA9\x45\xA9\x7A\x59\x89\xCC\x59\x00\xDC\xF7\x0B\xD9', 'binary'))).toEqual(expected);
    });

    it('references', () => {
        var reference = {
            "node" : "Hello",
            "id": [1245],
            "creation": 1
        };
        expect(erlpack.unpack(new Buffer('\x83em\x00\x00\x00\x05Hello\x00\x00\x04\xDD\x01', 'binary'))).toEqual(reference);

        reference = {
            "node" : "Hello",
            "id": [10, 15, 1245],
            "creation": 1
        };
        expect(erlpack.unpack(new Buffer('\x83r\x00\x03m\x00\x00\x00\x05Hello\x01\x00\x00\x00\x0A\x00\x00\x00\x0F\x00\x00\x04\xDD', 'binary'))).toEqual(reference);
    });

    it('port', () => {
        const port = {
            "node" : "Hello",
            "id": 1245,
            "creation": 1
        };
        expect(erlpack.unpack(new Buffer('\x83fm\x00\x00\x00\x05Hello\x00\x00\x04\xDD\x01', 'binary'))).toEqual(port);
    });

    it('pid', () => {
        const pid = {
            "node" : "Hello",
            "id": 1245,
            "serial": 123456,
            "creation": 1
        };
        expect(erlpack.unpack(new Buffer('\x83gm\x00\x00\x00\x05Hello\x00\x00\x04\xDD\x00\x01\xE2\x40\x01', 'binary'))).toEqual(pid);
    });

    it('export', () => {
        const exp = {
            "mod" : "guild_members",
            "fun": "append",
            "arity": 1
        };
        expect(erlpack.unpack(new Buffer('\x83qd\x00\x0Dguild_membersd\x00\x06appenda\x01', 'binary'))).toEqual(exp);
    });

    it('can unpack from ArrayBuffers', () => {
        const data = new Buffer('\x83k\x00\x0b' + helloWorldBinary, 'binary');
        var byteBuffer = new Uint8Array(data.length);
        for(var i = 0; i < data.length; ++i) {
            byteBuffer[i] = data[i];
        }
        expect(erlpack.unpack(byteBuffer)).toEqual(helloWorldList);
    });

    it('excepts from malformed token', () => {
        const data = new Buffer(
            '\x83q\x00\x00\x00\x03a\x02a\x02a\x03l\x00\x00\x00\x03a\x01a\x02a\x03jm\x00\x00\x00\x01aa\x01',
            'binary'
        );
        expect(() => erlpack.unpack(data)).toThrow("Unsupported erlang term type identifier found");
        expect(() => erlpack.unpack(new Buffer('\x83k\x00', 'binary'))).toThrow("Reading two bytes passes the end of the buffer.");
    });

    it('excepts from malformed array', () => {
       expect(() => erlpack.unpack(new Buffer('\x83t\x00\x00\x00\x03a\x02a\x02a\x03', 'binary'))).toThrow("Unpacking beyond the end of the buffer");
    });

    it('excepts from malformed object', () => {
        const data = new Buffer(
            '\x83t\x00\x00\x00\x04a\x02a\x02a\x03l\x00\x00\x00\x03a\x01a\x02a\x03jm\x00\x00\x00\x01aa\x01',
            'binary'
        );
        expect(() => erlpack.unpack(data)).toThrow("Unpacking beyond the end of the buffer");
    });

    it('excepts from malformed atom', () => {
        expect(() => erlpack.unpack(new Buffer('\x83s\x09true', 'binary'))).toThrow("Reading sequence past the end of the buffer.");
    });

    it('excepts from malformed integer', () => {
        expect(() => erlpack.unpack(new Buffer('\x83b\x00\x00\x04', 'binary'))).toThrow("Reading three bytes passes the end of the buffer.");
    });

    it('excepts from malformed float', () => {
        expect(() => erlpack.unpack(new Buffer('\x83c2.500000000000000e+00\x00\x00\x00\x00\x00', 'binary'))).toThrow("Reading sequence past the end of the buffer.");
    });

    it('excepts from malformed string ', () => {
        expect(() => erlpack.unpack(new Buffer('\x83k\x00\x0bworld', 'binary'))).toThrow("Reading sequence past the end of the buffer.");
    });

    it('excepts from malformed binary', () => {
        expect(() => erlpack.unpack(new Buffer('\x83m\x00\x00\x00\x0chel', 'binary'))).toThrow("Reading sequence past the end of the buffer.");
    });
});

