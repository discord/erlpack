jest.dontMock('bindings');

const erlpack = require('bindings')('erlpackjs');

describe('unpacks', () => {
    it('string with null byte', () => {
        expect(erlpack.unpack(new Buffer('\x83k\x00\x00\x00\x0chello\x00 world', 'binary'))).toEqual('hello\x00 world');
    });

    it('string without byte', () => {
        expect(erlpack.unpack(new Buffer('\x83k\x00\x00\x00\x0bhello world', 'binary'))).toEqual('hello world');
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
    //
    it('true', () => {
        expect(erlpack.unpack(new Buffer('\x83s\x04true', 'binary'))).toEqual(true);
    });

    it('nil is null', () => {
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
            expected = new Buffer(3);
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
});
