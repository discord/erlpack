package erlpack

// #include "../cpp/encoder.h"
import "C"
import (
	"errors"
	"fmt"
	"reflect"
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

// packInt64 is used to pack a 64-bit integer.
func packInt64(Data int64, Buffer *C.erlpack_buffer) {
	C.erlpack_append_long_long(Buffer, C.longlong(Data))
}

// packInt is used to pack a int.
func packInt(Data int, Buffer *C.erlpack_buffer) {
	if Data < 256 {
		// We can pack as a small int.
		C.erlpack_append_small_integer(Buffer, C.uchar(Data))
	} else if Data < 2147483647 {
		// We should pack as a standard int.
		C.erlpack_append_integer(Buffer, C.int(Data))
	} else {
		// Call packInt64 (this will only ever get here on a 64-bit system).
		packInt64(int64(Data), Buffer)
	}
}

// packFloat64 is used to pack a 64-bit floating point number.
func packFloat64(Data float64, Buffer *C.erlpack_buffer) {
	C.erlpack_append_double(Buffer, C.double(Data))
}

// packBool is used to pack a boolean.
func packBool(Data bool, Buffer *C.erlpack_buffer) {
	if Data {
		C.erlpack_append_true(Buffer)
	} else {
		C.erlpack_append_false(Buffer)
	}
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
			// Pack the nil bytes and return nil.
			packNil(buffer)
			return nil
		case string:
			// Pack the string and return nil.
			packString(i.(string), buffer)
			return nil
		case bool:
			// Pack the boolean and return nil.
			packBool(i.(bool), buffer)
			return nil
		case int:
			// Pack the integer and return nil.
			packInt(i.(int), buffer)
			return nil
		case int64:
			// Pack the int64 and return nil.
			packInt64(i.(int64), buffer)
			return nil
		case float64:
			// Pack the float64 and return nil.
			packFloat64(i.(float64), buffer)
			return nil
		default:
			rt := reflect.ValueOf(i)
			switch rt.Kind() {
			case reflect.Ptr:
				// Check if it's a null pointer.
				if rt.IsNil() {
					// It is. Add a null.
					packNil(buffer)
				} else {
					// No it isn't. Pack the value.
					err := handler(rt.Elem().Interface())
					if err != nil {
						return err
					}
				}

				// Return nil (there's been no errors).
				return nil
			case reflect.Slice, reflect.Array:
				// Get the length.
				l := rt.Len()

				// Process the length.
				if l == 0 {
					// Apply the null length bytes.
					C.erlpack_append_nil_ext(buffer)
				} else {
					// Iterate through the array.
					C.erlpack_append_list_header(buffer, C.ulong(l))
					for i := 0; i < l; i++ {
						item := rt.Index(i).Interface()
						err := handler(item)
						if err != nil {
							return err
						}
					}
					C.erlpack_append_nil_ext(buffer)
				}

				// Return nil (there were no errors).
				return nil
			case reflect.Map:
				// Create the map header.
				keys := rt.MapKeys()
				C.erlpack_append_map_header(buffer, C.ulong(len(keys)))

				// Iterate the map.
				for _, e := range keys {
					v := rt.MapIndex(e)
					err := handler(v.Interface())
					if err != nil {
						return err
					}
					Value := v.Elem().Interface()
					err = handler(Value)
					if err != nil {
						return err
					}
				}

				// Return nil (there were no errors).
				return nil
			default:
				// Send a unknown type error.
				return errors.New(fmt.Sprintf("unknown type: %T", i))
			}
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
