package erlpack

import (
	"reflect"
	"testing"
)

// TestUnpackTrue is used to unpack the true boolean.
func TestUnpackTrue(t *testing.T) {
	// Test boolean unpacking.
	var x bool
	err := Unpack([]byte("\x83s\x04true"), &x)
	if err != nil {
		t.Fatal(err)
	}
	if !x {
		t.Fatal("didn't deserialize properly")
	}

	// Test atom unpacking.
	var s Atom
	err = Unpack([]byte("\x83s\x04true"), &s)
	if err != nil {
		t.Fatal(err)
	}
	if s != "true" {
		t.Fatal("didn't deserialize properly")
	}
}

// TestUnpackFalse is used to unpack the false boolean.
func TestUnpackFalse(t *testing.T) {
	// Test boolean unpacking.
	var x bool
	err := Unpack([]byte("\x83s\x05false"), &x)
	if err != nil {
		t.Fatal(err)
	}
	if x {
		t.Fatal("didn't deserialize properly")
	}

	// Test atom unpacking.
	var s Atom
	err = Unpack([]byte("\x83s\x05false"), &s)
	if err != nil {
		t.Fatal(err)
	}
	if s != "false" {
		t.Fatal("didn't deserialize properly")
	}
}

// TestUnpackNil is used to unpack nil.
func TestUnpackNil(t *testing.T) {
	// Test boolean pointer unpacking.
	var x *bool
	err := Unpack([]byte("\x83s\x03nil"), &x)
	if err != nil {
		t.Fatal(err)
	}
	if x != nil {
		t.Fatal("should be nil")
	}

	// Test atom unpacking.
	var s Atom
	err = Unpack([]byte("\x83s\x03nil"), &s)
	if err != nil {
		t.Fatal(err)
	}
	if s != "nil" {
		t.Fatal("didn't deserialize properly")
	}
}

// TestUnpackStrings is used to unpack strings.
func TestUnpackStrings(t *testing.T) {
	var x string
	err := Unpack([]byte("\x83m\x00\x00\x00\x0bhello world"), &x)
	if err != nil {
		t.Fatal(err)
	}
	if x != "hello world" {
		t.Fatal("unknown string")
	}
}

// TestUnpackGenericArray is used to unpack a generic array.
func TestUnpackGenericArray(t *testing.T) {
	packed := []byte("\x83l\x00\x00\x00\x05a\x01m\x00\x00\x00\x03twoF\x40\x08\xcc\xcc\xcc\xcc\xcc\xcdm\x00\x00\x00\x04fourl\x00\x00\x00\x01m\x00\x00\x00\x04fivejj")
	var a []interface{}
	err := Unpack(packed, &a)
	if err != nil {
		t.Fatal(err)
	}
	expected := []interface{}{
		uint8(1), []byte("two"), 3.1, []byte("four"), []interface{}{[]byte("five")},
	}
	if len(a) != len(expected) {
		t.Fatal("length is different")
	}
	for i, v := range expected {
		if !reflect.DeepEqual(v, a[i]) {
			t.Fatal("unexpected result:", a[i], reflect.TypeOf(a[i]), "!=", v, reflect.TypeOf(v))
		}
	}
}

// TestUnpackcArray is used to unpack a array.
func TestUnpackArray(t *testing.T) {
	packed := []byte("\x83l\x00\x00\x00\x01a\x01")
	var a []int
	err := Unpack(packed, &a)
	if err != nil {
		t.Fatal(err)
	}
	if len(a) != 1 {
		t.Fatal("length is not 1")
	}
	if a[0] != 1 {
		t.Fatal("should be 1")
	}
}

// TestUnpackEmptyArray is used to unpack an empty array.
func TestUnpackEmptyArray(t *testing.T) {
	packed := []byte("\x83j")
	var a []interface{}
	err := Unpack(packed, &a)
	if err != nil {
		t.Fatal(err)
	}
	if len(a) != 0 {
		t.Fatal("length is meant to be 0")
	}
}

// TestUnpack32BitInt is used to test a 32-bit int.
func TestUnpack32BitInt(t *testing.T) {
	var i int32
	err := Unpack([]byte("\x83b\x00\x00\x04\x00"), &i)
	if err != nil {
		t.Fatal(err)
	}
	if i != 1024 {
		t.Fatal("unexpected result:", i)
	}
}

// TestUnpackGenericMap is used to test a generic map.
func TestUnpackGenericMap(t *testing.T) {
	var x map[interface{}]interface{}
	err := Unpack([]byte("\x83t\x00\x00\x00\x01m\x00\x00\x00\x01aa\x01"), &x)
	if err != nil {
		t.Error(err)
	}
	r, ok := x["a"].(uint8)
	if !ok {
		t.Fatal("not ok")
	}
	if r != 1 {
		t.Fatal("not 1")
	}
}

// TestUnpackMap is used to test a non-generic map.
func TestUnpackMap(t *testing.T) {
	var x map[string]int
	err := Unpack([]byte("\x83t\x00\x00\x00\x01m\x00\x00\x00\x01aa\x01"), &x)
	if err != nil {
		t.Fatal(err)
	}
	r, ok := x["a"]
	if !ok {
		t.Fatal("not ok")
	}
	if r != 1 {
		t.Fatal("not 1")
	}
}

// TestUnpackStruct is used to test unpacking to a struct.
func TestUnpackStruct(t *testing.T) {
	type test struct {
		A *int `erlpack:"a"`
	}
	var x test
	err := Unpack([]byte("\x83t\x00\x00\x00\x01m\x00\x00\x00\x01aa\x01"), &x)
	if err != nil {
		t.Fatal(err)
	}
	if *x.A != 1 {
		t.Fatal("not 1")
	}
}

// TestUnpackUncastedResult is used to test unpacking a uncasted result to a struct.
func TestUnpackUncastedResult(t *testing.T) {
	type test struct {
		A *int `erlpack:"a"`
	}
	var a UncastedResult
	err := Unpack([]byte("\x83t\x00\x00\x00\x01m\x00\x00\x00\x01aa\x01"), &a)
	if err != nil {
		t.Fatal(err)
	}
	var x test
	err = a.Cast(&x)
	if err != nil {
		t.Fatal(err)
	}
	if *x.A != 1 {
		t.Fatal("not 1")
	}
}

// BenchmarkUnpack is used to benchmark unpacking.
func BenchmarkUnpack(b *testing.B) {
	type test struct {
		A *int `erlpack:"a"`
	}
	var x test
	_ = Unpack([]byte("\x83t\x00\x00\x00\x01m\x00\x00\x00\x01aa\x01"), &x)
}

// BenchmarkLargeUnpack is used to benchmark a huge item being unpacked.
func BenchmarkLargerUnpack(b *testing.B) {
	m := map[int]int{}
	for i := 0; i < 10000; i++ {
		m[i] = 1024
	}
	data, err := Pack(&m)
	if err != nil {
		b.Fatal(err)
	}
	b.ResetTimer()
	m = nil
	err = Unpack(data, &m)
	if err != nil {
		b.Fatal(err)
	}
}
