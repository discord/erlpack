package com.discordapp.erlpack;

import java.io.Closeable;
import java.io.IOException;
import java.io.Reader;
import java.nio.CharBuffer;

public class ErlpackReader implements Closeable {

    private final Reader in;
    private final CharBuffer bufInt = CharBuffer.allocate(2);
    private final CharBuffer bufLong = CharBuffer.allocate(4);

    public ErlpackReader(final Reader in) {
        this.in = in;
    }

    @Override
    public void close() throws IOException {
        in.close();
    }

    private byte read8() throws IOException {
        return (byte)in.read();
    }

    private short read16() throws IOException {
        return (short)in.read();
    }

    private int read32() throws IOException {
        bufInt.clear();
        if (in.read(bufInt) == -1) {
            throw new IOException("End of stream.");
        }
        bufInt.flip();

        int result = 0;
        for (int i = 0; i < 2; i++) {
            int digit = (int)bufInt.charAt(i) - (int)'0';
            if ((digit < 0) || (digit > 9)) throw new NumberFormatException("Invalid digit.");
            result *= 10;
            result += digit;
        }
        return result;
    }

    private long read64() throws IOException {
        bufLong.clear();
        if (in.read(bufLong) == -1) {
            throw new IOException("End of stream.");
        }

        long result = 0;
        for (int i = 0; i < 4; i++) {
            int digit = (int)bufLong.charAt(i) - (int)'0';
            if ((digit < 0) || (digit > 9)) throw new NumberFormatException("Invalid digit.");
            result *= 10;
            result += digit;
        }
        return result;
    }

    public int decodeSmallInteger() throws IOException {
        return (int)read8();
    }

    public int decodeInteger() throws IOException {
        return read32();
    }
}
