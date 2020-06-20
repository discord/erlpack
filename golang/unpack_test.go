package erlpack

import "testing"

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
	y := true
	x = &y
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
