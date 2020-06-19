package erlpack

import "testing"

func TestScratchpad(t *testing.T) {
	pad := newScratchpad(5)
	pad.startAppend('h', 'e', 'l', 'l', 'o')
	if len(pad.alloc) != 5 {
		t.Fatal("pad reallocated")
	}
	s := string(pad.bytes())
	if s != "hello" {
		t.Fatal("pad says ", s)
	}
	pad.endAppend(' ', 'w', 'o', 'r', 'l', 'd')
	if len(pad.alloc) != 16 {
		t.Fatal("expected 16 byte pad realloc, got", len(pad.alloc), "byte realloc")
	}
	s = string(pad.bytes())
	if s != "hello world" {
		t.Fatal("pad says ", s)
	}
	pad.startAppend(':', ')', ' ')
	s = string(pad.bytes())
	if s != ":) hello world" {
		t.Fatal("pad says ", s)
	}
}
