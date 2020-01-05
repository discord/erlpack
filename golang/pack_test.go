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
	}
}

// TestTrue is used to test that true is output correctly.
func TestTrue(t *testing.T) {
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

// TestFalse is used to test that false is output correctly.
func TestFalse(t *testing.T) {
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

// TestEmptySlice is used to test a empty slice.
func TestEmptySlice(t *testing.T) {
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

// TestEmptyArray is used to test a empty array.
func TestEmptyArray(t *testing.T) {
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

// TestNilStringPointer is used to test a nil string pointer (this same logic applies for ALL pointers).
func TestNilStringPointer(t *testing.T) {
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

// TestNonNilStringPointer is used to test a non-nil string pointer (this same logic applies for ALL pointers).
func TestNonNilStringPointer(t *testing.T) {
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

// TestInterfaceSlice is used to test a slice of various different interfaces.
func TestInterfaceSlice(t *testing.T) {
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

// TestInterfaceMap is used to test a map of various different interfaces.
func TestInterfaceMap(t *testing.T) {
	b, err := Pack(map[interface{}]interface{}{
		"a": 1, 2: 2, 3: []int{1, 2, 3},
	})
	if err != nil {
		t.Error(err)
		return
	}
	err = assertBytes([]byte("\x83t\x00\x00\x00\x03a\x02a\x02a\x03l\x00\x00\x00\x03a\x01a\x02a\x03jm\x00\x00\x00\x01aa\x01"), b)
	if err != nil {
		t.Error(err)
	}
}
