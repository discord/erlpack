package com.discordapp.erlpack;

import java.io.Closeable;
import java.io.IOException;
import java.io.Reader;

public class ErlpackReader implements Closeable {

    private final Reader in;

    public ErlpackReader(final Reader in) {
        this.in = in;
    }

    @Override
    public void close() throws IOException {
        in.close();
    }
}
