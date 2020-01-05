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
	}
}

// TestPackNil is used to test that nil is output correctly.
func TestPackNil(t *testing.T) {
	b, err := Pack(nil)
	if err != nil {
		t.Error(err)
		return
	}
	err = assertBytes([]byte("\x83s\x03nil"), b)
	if err != nil {
		t.Error(err)
	}
}

// TestPackTrue is used to test that true is output correctly.
func TestPackTrue(t *testing.T) {
	b, err := Pack(true)
	if err != nil {
		t.Error(err)
		return
	}
	err = assertBytes([]byte("\x83s\x04true"), b)
	if err != nil {
		t.Error(err)
	}
}

// TestPackFalse is used to test that false is output correctly.
func TestPackFalse(t *testing.T) {
	b, err := Pack(false)
	if err != nil {
		t.Error(err)
		return
	}
	err = assertBytes([]byte("\x83s\x05false"), b)
	if err != nil {
		t.Error(err)
	}
}

// TestPackEmptySlice is used to test a empty slice.
func TestPackEmptySlice(t *testing.T) {
	b, err := Pack([]string{})
	if err != nil {
		t.Error(err)
		return
	}
	err = assertBytes([]byte("\x83j"), b)
	if err != nil {
		t.Error(err)
	}
}

// TestPackEmptyArray is used to test a empty array.
func TestPackEmptyArray(t *testing.T) {
	b, err := Pack([0]string{})
	if err != nil {
		t.Error(err)
		return
	}
	err = assertBytes([]byte("\x83j"), b)
	if err != nil {
		t.Error(err)
	}
}

// TestPackNilStringPointer is used to test a nil string pointer (this same logic applies for ALL pointers).
func TestPackNilStringPointer(t *testing.T) {
	var p *string
	b, err := Pack(p)
	if err != nil {
		t.Error(err)
		return
	}
	err = assertBytes([]byte("\x83s\x03nil"), b)
	if err != nil {
		t.Error(err)
	}
}

// TestPackNonNilStringPointer is used to test a non-nil string pointer (this same logic applies for ALL pointers).
func TestPackNonNilStringPointer(t *testing.T) {
	s := "hello world"
	b, err := Pack(&s)
	if err != nil {
		t.Error(err)
		return
	}
	err = assertBytes([]byte("\x83m\x00\x00\x00\x0bhello world"), b)
	if err != nil {
		t.Error(err)
	}
}

// TestPackInterfaceSlice is used to test a slice of various different interfaces.
func TestPackInterfaceSlice(t *testing.T) {
	b, err := Pack([]interface{}{
		1, "two", 3.1, "four", []interface{}{"five"},
	})
	if err != nil {
		t.Error(err)
		return
	}
	err = assertBytes([]byte("\x83l\x00\x00\x00\x05a\x01m\x00\x00\x00\x03twoF\x40\x08\xcc\xcc\xcc\xcc\xcc\xcdm\x00\x00\x00\x04fourl\x00\x00\x00\x01m\x00\x00\x00\x04fivejj"), b)
	if err != nil {
		t.Error(err)
	}
}

// TestPackSmallInt is used to test a small int.
func TestPackSmallInt(t *testing.T) {
	b, err := Pack(3)
	if err != nil {
		t.Error(err)
		return
	}
	err = assertBytes([]byte("\x83a\x03"), b)
	if err != nil {
		t.Error(err)
	}
}

// TestPack32BitInt is used to test a 32-bit int.
func TestPack32BitInt(t *testing.T) {
	b, err := Pack(1024)
	if err != nil {
		t.Error(err)
		return
	}
	err = assertBytes([]byte("\x83b\x00\x00\x04\x00"), b)
	if err != nil {
		t.Error(err)
	}
}

// TestPackInterfaceMap is used to test a map of different interfaces.
func TestPackInterfaceMap(t *testing.T) {
	b, err := Pack(map[interface{}]interface{}{
		"a": 1,
	})
	if err != nil {
		t.Error(err)
		return
	}
	err = assertBytes([]byte("\x83t\x00\x00\x00\x01m\x00\x00\x00\x01aa\x01"), b)
	if err != nil {
		t.Error(err)
	}
}
