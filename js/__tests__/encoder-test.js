jest.dontMock('../index');

const erlpack = require('../index.js');

describe('packs', () => {
    it('string with null byte', () => {
        const packed = erlpack.pack('hello\x00 world');
        const expected = new Buffer('\x83m\x00\x00\x00\x0chello\x00 world', 'binary');
        expect(packed.equals(expected)).toBeTruthy();
    });

    it('string without null byte', () => {
        const packed = erlpack.pack('hello world');
        const expected = new Buffer('\x83m\x00\x00\x00\x0bhello world', 'binary');
        expect(packed.equals(expected)).toBeTruthy();
    });

    it('dictionary', () => {
        const expected = new Buffer(
            '\x83t\x00\x00\x00\x03a\x02a\x02a\x03l\x00\x00\x00\x03a\x01a\x02a\x03jm\x00\x00\x00\x01aa\x01',
            'binary'
        );
        const packed = erlpack.pack({'a': 1, 2: 2, 3: [1, 2, 3]});
        expect(packed.equals(expected)).toBeTruthy();
    });

    it('false', () => {
        const expected = new Buffer('\x83s\x05false', 'binary');
        const packed = erlpack.pack(false);
        expect(packed.equals(expected)).toBeTruthy();
    });

    it('true', () => {
        const expected = new Buffer('\x83s\x04true', 'binary');
        const packed = erlpack.pack(true);
        expect(packed.equals(expected)).toBeTruthy();
    });

    it('null is nil', () => {
        const expected = new Buffer('\x83s\x03nil', 'binary');
        const packed = erlpack.pack(null);
        expect(packed.equals(expected)).toBeTruthy();
    });

    it('undefined is nil', () => {
        const expected = new Buffer('\x83s\x03nil', 'binary');
        const packed = erlpack.pack(undefined);
        expect(packed.equals(expected)).toBeTruthy();
    });

    it('floats as new floats', () => {
        expect(erlpack.pack(2.5).equals(new Buffer('\x83F\x40\x04\x00\x00\x00\x00\x00\x00', 'binary'))).toBeTruthy();
        expect(erlpack.pack(51512123841234.31423412341435123412341342).equals(new Buffer('\x83F\x42\xc7\x6c\xcc\xeb\xed\x69\x28', 'binary'))).toBeTruthy();
    });

    it('small int', () => {
        function check(small_int) {
            expected = new Buffer(3);
            expected.write('\x83a', 0, 2, 'binary');
            expected.writeUInt8(small_int, 2);
            const packed = erlpack.pack(small_int);
            expect(expected.equals(packed)).toBeTruthy();
        }

        for(var i = 0; i < 256; ++i) {
            check(i);
        }
    });

    it('int32', () => {
        expect(erlpack.pack(1024).equals(new Buffer('\x83b\x00\x00\x04\x00', 'binary'))).toBeTruthy();
        expect(erlpack.pack(-2147483648).equals(new Buffer('\x83b\x80\x00\x00\x00', 'binary'))).toBeTruthy();
        expect(erlpack.pack(2147483647).equals(new Buffer('\x83b\x7f\xff\xff\xff', 'binary'))).toBeTruthy();
    });

    it('list', () => {
        const expected = new Buffer('\x83l\x00\x00\x00\x05a\x01m\x00\x00\x00\x03twoF\x40\x08\xcc\xcc\xcc\xcc\xcc\xcdm\x00\x00\x00\x04fourl\x00\x00\x00\x01m\x00\x00\x00\x04fivejj', 'binary');
        const packed = erlpack.pack([1, "two", 3.1, "four", ['five']]);
        expect(packed.equals(expected)).toBeTruthy();
    });

    it('empty list', () => {
        expect(erlpack.pack([]).equals(new Buffer('\x83j', 'binary'))).toBeTruthy();
    });
});
