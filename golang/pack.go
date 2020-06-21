package erlpack

import (
	"encoding/binary"
	"errors"
	"fmt"
	"github.com/jakemakesstuff/structs"
	"reflect"
	"unsafe"
)

// INITIAL_ALLOC is the initial allocation.
var INITIAL_ALLOC = uint(1024 * 1024)

func ntohl32(i uint32, a []byte, offset int) {
	bytes := make([]byte, 4)
	binary.BigEndian.PutUint32(bytes, i)
	for i := 0; i < 4; i++ {
		a[i+offset] = bytes[i]
	}
}

func be64toh(i uint64, a []byte, offset int) {
	bytes := make([]byte, 8)
	binary.BigEndian.PutUint64(bytes, i)
	for i := 0; i < 8; i++ {
		a[i+offset] = bytes[i]
	}
}

// appendListHeader is used to append the list header.
func appendListHeader(pad *scratchpad, l uint32) {
	// Create the initial allocation and define the header.
	a := make([]byte, 5)
	a[0] = 'l'

	// Write the length.
	ntohl32(l, a, 1)

	// Append the header.
	pad.endAppend(a...)
}

// appendMapHeader is used to append the map header.
func appendMapHeader(pad *scratchpad, l uint32) {
	// Create the initial allocation and define the header.
	a := make([]byte, 5)
	a[0] = 't'

	// Write the length.
	ntohl32(l, a, 1)

	// Append the header.
	pad.endAppend(a...)
}

// packString is used to pack a string.
func packString(Data string, pad *scratchpad) {
	// Create the initial allocation and define the header.
	a := make([]byte, 5)
	a[0] = 'm'

	// Write the length.
	ntohl32(uint32(len(Data)), a, 1)

	// Append the header.
	pad.endAppend(a...)

	// Append the data.
	pad.endAppend([]byte(Data)...)
}

// packNil is used to pack a nil.
func packNil(pad *scratchpad) {
	pad.endAppend('s', 3, 'n', 'i', 'l')
}

// packInt64 is used to pack a 64-bit integer.
func packInt64(Data int64, pad *scratchpad) {
	// Create the initial allocation and define the header.
	a := make([]byte, 11)
	a[0] = 'n'

	// Define the int signature.
	if 0 > Data {
		a[2] = 1
	}

	// Create the unsigned int64.
	var ull uint64
	if 0 > Data {
		ull = uint64(Data * -1)
	} else {
		ull = uint64(Data)
	}

	// Defines how many bytes were encoded.
	BytesEnc := 0

	// Iterate through while ull is greater than 0.
	for ull > 0 {
		a[3+BytesEnc] = byte(ull)
		ull >>= 8
		BytesEnc++
	}

	// Add the length.
	a[1] = byte(BytesEnc)

	// Append the data.
	pad.endAppend(a...)
}

// packInt is used to pack a int.
func packInt(Data int, pad *scratchpad) {
	if Data < 256 && Data > 0 {
		// We can pack as a small int.
		pad.endAppend('a', byte(Data))
	} else if 2147483647 > Data && Data > -2147483647 {
		// We should pack as a standard int.
		a := make([]byte, 5)
		a[0] = 'b'
		i32 := int32(Data)
		ntohl32(*(*uint32)(unsafe.Pointer(&i32)), a, 1)
		pad.endAppend(a...)
	} else {
		// Call packInt64 (this will only ever get here on a 64-bit system).
		packInt64(int64(Data), pad)
	}
}

// packFloat64 is used to pack a 64-bit floating point number.
func packFloat64(Data float64, pad *scratchpad) {
	// Allocate the bytes.
	a := make([]byte, 9)

	// Set the header.
	a[0] = 'F'

	// Cast the memory to a uint64.
	i := *(*uint64)(unsafe.Pointer(&Data))

	// Write the integer.
	be64toh(i, a, 1)

	// Write to the pad.
	pad.endAppend(a...)
}

// packBool is used to pack a boolean.
func packBool(Data bool, pad *scratchpad) {
	if Data {
		pad.endAppend('s', 4, 't', 'r', 'u', 'e')
	} else {
		pad.endAppend('s', 5, 'f', 'a', 'l', 's', 'e')
	}
}

// Pack is used to pack a interface given to it.
func Pack(Interface interface{}) ([]byte, error) {
	// Create a scratchpad which will be used for creating this.
	pad := newScratchpad(INITIAL_ALLOC)
	pad.endAppend(131)

	// Add a switch for the type.
	var handler func(i interface{}) error
	handler = func(i interface{}) error {
		switch i.(type) {
		case nil:
			// Pack the nil bytes and return nil.
			packNil(pad)
			return nil
		case string:
			// Pack the string and return nil.
			packString(i.(string), pad)
			return nil
		case bool:
			// Pack the boolean and return nil.
			packBool(i.(bool), pad)
			return nil
		case int:
			// Pack the integer and return nil.
			packInt(i.(int), pad)
			return nil
		case int64:
			// Pack the int64 and return nil.
			packInt64(i.(int64), pad)
			return nil
		case float32:
			// Pack the float32 as a float64 and return nil.
			packFloat64(float64(i.(float32)), pad)
			return nil
		case Atom:
			// Pack a atom and return nil.
			pad.endAppend('s', byte(len(i.(Atom))))
			pad.endAppend([]byte(i.(Atom))...)
			return nil
		case UncastedResult:
			// Pack a uncasted result.
			return handler(i.(UncastedResult).item)
		case float64:
			// Pack the float64 and return nil.
			packFloat64(i.(float64), pad)
			return nil
		default:
			rt := reflect.ValueOf(i)
			switch rt.Kind() {
			case reflect.Ptr:
				// Check if it's a null pointer.
				if rt.IsNil() {
					// It is. Add a null.
					packNil(pad)
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
					pad.endAppend('j')
				} else {
					// Iterate through the array.
					appendListHeader(pad, uint32(l))
					for i := 0; i < l; i++ {
						item := rt.Index(i).Interface()
						err := handler(item)
						if err != nil {
							return err
						}
					}
					pad.endAppend('j')
				}

				// Return nil (there were no errors).
				return nil
			case reflect.Map:
				// Create the map header.
				keys := rt.MapKeys()
				appendMapHeader(pad, uint32(len(keys)))

				// Iterate the map.
				for _, e := range keys {
					v := rt.MapIndex(e)
					Key := e.Interface()
					err := handler(Key)
					if err != nil {
						return err
					}
					Value := v.Interface()
					err = handler(Value)
					if err != nil {
						return err
					}
				}

				// Return nil (there were no errors).
				return nil
			case reflect.Struct:
				// Create a struct parser.
				s := structs.New(i)
				s.TagName = "erlpack"

				// Call this back with the generated map.
				return handler(s.Map())
			default:
				// Send a unknown type error.
				return errors.New(fmt.Sprintf("unknown type: %T", i))
			}
		}
	}

	// Runs the handler.
	err := handler(Interface)
	if err != nil {
		return nil, err
	}
	return pad.bytes(), nil
}
