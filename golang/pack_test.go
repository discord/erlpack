package erlpack

import (
	"errors"
	"fmt"
	"testing"
)

// assertBytes is used to assert 2 byte arrays are the same.
func assertBytes(A []byte, B []byte) error {
	e := func() error {
		return errors.New(fmt.Sprintf("assert error: %v != %v", A, B))
	}
	if len(A) != len(B) {
		return e()
	}
	for i, v := range A {
		if B[i] != v {
			return e()
		}
	}
	return nil
}

// TestPackStringNoNull is used to test packing a string with no null byte.
func TestPackStringNoNull(t *testing.T) {
	b, err := Pack("hello world")
	if err != nil {
		t.Error(err)
		return
	}
	err = assertBytes([]byte("\x83m\x00\x00\x00\x0bhello world"), b)
	if err != nil {
		t.Error(err)
		return
	}
}

// TestPackStringNull is used to test packing a string with a null byte.
func TestPackStringNull(t *testing.T) {
	b, err := Pack("hello\x00 world")
	if err != nil {
		t.Error(err)
		return
	}
	err = assertBytes([]byte("\x83m\x00\x00\x00\x0chello\x00 world"), b)
	if err != nil {
		t.Error(err)
		return
	}
}

// TestNil is used to test that nil is output correctly.
func TestNil(t *testing.T) {
	b, err := Pack(nil)
	if err != nil {
		t.Error(err)
		return
	}
	err = assertBytes([]byte("\x83s\x03nil"), b)
	if err != nil {
		t.Error(err)
		return
	}
}
