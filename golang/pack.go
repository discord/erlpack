package erlpack

// #include "../cpp/encoder.h"
import "C"
import (
	"errors"
	"fmt"
	"unsafe"
)

// INITIAL_ALLOC is the initial allocation.
var INITIAL_ALLOC = 1024 * 1024

// createErlpackBuffer is used to create a new erlpack buffer.
func createErlpackBuffer() *C.erlpack_buffer {
	size := C.ulong(INITIAL_ALLOC)
	buf := C.erlpack_buffer{}
	d := C.malloc(size)
	buf.buf = (*C.char)(d)
	buf.length = 0
	buf.allocated_size = size
	C.erlpack_append_version(&buf)
	return &buf
}

// packString is used to pack a string.
func packString(Data string, Buffer *C.erlpack_buffer) {
	cstr := C.CString(Data)
	C.erlpack_append_binary(Buffer, cstr, C.ulong(len(Data)))
	C.free(unsafe.Pointer(cstr))
}

// packNil is used to pack a nil.
func packNil(Buffer *C.erlpack_buffer) {
	C.erlpack_append_nil(Buffer)
}

// finaliseBuffer is used to finalise a erlpack buffer.
func finaliseBuffer(Buffer *C.erlpack_buffer) []byte {
	Data := C.GoBytes(unsafe.Pointer(Buffer.buf), C.int(Buffer.length))
	C.free(unsafe.Pointer(Buffer.buf))
	return Data
}

// Pack is used to pack a interface given to it.
func Pack(Interface interface{}) ([]byte, error) {
	// Create a erlpack buffer.
	buffer := createErlpackBuffer()

	// Add a switch for the type.
	var handler func(i interface{}) error
	handler = func(i interface{}) error {
		switch i.(type) {
		case nil:
			// Pack the nil byte.
			packNil(buffer)
			return nil
		case *string:
			// Pack a string or nil.
			s := i.(*string)
			if s == nil {
				packNil(buffer)
			} else {
				packString(*s, buffer)
			}
			return nil
		case string:
			// Pack the string and return nil.
			packString(i.(string), buffer)
			return nil
		default:
			// Send a unknown type error.
			return errors.New(fmt.Sprintf("unknown type: %T", i))
		}
	}

	// Runs the handler.
	err := handler(Interface)
	final := finaliseBuffer(buffer)
	if err != nil {
		return nil, err
	}
	return final, nil
}
