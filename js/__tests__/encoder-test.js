jest.dontMock('bindings');

const erlpack = require('bindings')('erlpackjs');

describe('test', () => {
    it('should load', () => {
        expect(erlpack.hello()).toBe("worldly");
    });
});