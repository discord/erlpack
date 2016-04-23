package com.discordapp.erlpack;

import org.junit.Test;

import java.io.StringReader;

import static org.junit.Assert.assertEquals;

/**
 * Created by Miguel Gaeta on 4/22/16.
 */
public class ErlpackReaderTest {

    @Test
    public void test_small_atom() throws Exception {
        final ErlpackReader reader = new ErlpackReader(new StringReader("\\x83s\\x0bhello world"));

        assertEquals(4, 2 + 2);
    }

    @Test
    public void test_large_atom() throws Exception {

    }
}
