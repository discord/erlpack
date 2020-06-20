package erlpack

import (
	"bytes"
	"encoding/binary"
	"errors"
	"reflect"
)

// Atom is used to define an atom within the codebase.
type Atom string

// Used to process an atom during unpacking.
func processAtom(Data []byte) interface{} {
	matchRest := func(d []byte) bool {
		if len(d) > len(Data)-1 {
			return false
		}
		for i := 0; i < len(d); i++ {
			if Data[i+1] != d[i] {
				return false
			}
		}
		return true
	}
	switch Data[0] {
	case 't':
		matched := matchRest([]byte("rue"))
		if !matched {
			return Atom(Data)
		}
		return true
	case 'f':
		matched := matchRest([]byte("alse"))
		if !matched {
			return Atom(Data)
		}
		return false
	case 'n':
		matched := matchRest([]byte("il"))
		if !matched {
			return Atom(Data)
		}
		return nil
	default:
		return Atom(Data)
	}
}

// Processes a item.
func processItem(Ptr interface{}, r *bytes.Reader) error {
	// Gets the type of data.
	DataType, err := r.ReadByte()
	if err != nil {
		return errors.New("not long enough to include data type")
	}
	var Item interface{}
	switch DataType {
	case 's': // This is an atom.
		// Get the atom information.
		if r.Len() == 0 {
			// Byte slice is too small.
			return errors.New("atom information missing")
		}
		b, _ := r.ReadByte()
		Len := int(b)
		Data := make([]byte, Len)
		Total := 0
		for {
			if Total == Len {
				// We have all the information we need.
				break
			}
			b, err := r.ReadByte()
			if err != nil {
				return errors.New("atom size larger than remainder of array")
			}
			Data[Total] = b
			Total++
		}
		Item = processAtom(Data)
	case 'j': // Blank list.
		Item = []interface{}{}
	case 'l': // List.
		// Get the length of the list.
		lengthBytes := make([]byte, 4)
		_, err := r.Read(lengthBytes)
		if err != nil {
			return errors.New("not enough bytes for list length")
		}
		l := binary.BigEndian.Uint32(lengthBytes)

		// Try and get each item from the list.
		Item = make([]interface{}, l)
		for i := 0; i < int(l); i++ {
			var x interface{}
			err := processItem(&x, r)
			if err != nil {
				return err
			}
			Item.([]interface{})[i] = x
		}
	case 'm': // String.
		// Get the length of the string.
		lengthBytes := make([]byte, 4)
		_, err := r.Read(lengthBytes)
		if err != nil {
			return errors.New("not enough bytes for list length")
		}
		l := binary.BigEndian.Uint32(lengthBytes)

		// Make an array of the specified length.
		Item = make([]byte, l)

		// Write into it if we can.
		_, err = r.Read(Item.([]byte))
		if err != nil {
			return errors.New("string length is longer than remainder of array")
		}
	//case 'a': // Small int.
	//	i, err := r.ReadByte()
	//	if err != nil {
	//		return errors.New("failed to read small int")
	//	}
	//	Item = int(i)
	//case 'b': // int32
	//	b := make([]byte, 4)
	//	_, err := r.Read(b)
	//	if err != nil {
	//		return errors.New("not enough bytes for int32")
	//	}
	//	l := binary.BigEndian.Uint32(b)
	//	Item = int(l)
	//case 'n': // int64
	//	b := make([]byte, 11)
	//	_, err := r.Read(b)
	//	if err != nil {
	//		return errors.New("not enough bytes for int64")
	//	}
	//	l := binary.BigEndian.Uint32(b)
	//	Item = int(l)
	// TODO: Fix ints.
	default: // Don't know this data type.
		return errors.New("unknown data type")
	}

	// Handle the item casting.
	switch x := Item.(type) {
	case []interface{}:
		// We should handle this array.
		switch p := Ptr.(type) {
		case *[]interface{}:
			// This is simple.
			*p = x
		default:
			// Get the reflect value.
			v := reflect.ValueOf(p)

			// Create the new array.
			a := reflect.New(v.Elem().Type())
			a.SetLen(len(x))

			// Set all the items.
			for i, v := range x {
				a.Index(i).Set(reflect.ValueOf(v))
			}

			// Set the pointer to the item.
			e := reflect.ValueOf(p).Elem()
			e.Set(a)

			// Return no errors.
			return nil
		}
	case []byte:
		// We should try and string-ify this if possible.
		switch p := Ptr.(type) {
		case *string:
			*p = string(x)
		case *[]byte:
			*p = x
		default:
			return errors.New("could not de-serialize into string")
		}
		return nil
	case bool:
		// This should cast into either a string or a boolean.
		switch p := Ptr.(type) {
		case *Atom:
			// Set it to a string representation of the value.
			if x {
				*p = "true"
			} else {
				*p = "false"
			}
			return nil
		case *bool:
			// Set it to the raw value.
			*p = x
			return nil
		}
	case nil:
		// This should zero any data types other than atoms.
		switch p := Ptr.(type) {
		case *Atom:
			// We should set this to "nil".
			*p = "nil"
			return nil
		default:
			// Zero the pointer which is provided.
			e := reflect.ValueOf(p).Elem()
			e.Set(reflect.Zero(e.Type()))
			return nil
		}
	}

	// Return unknown type error.
	return errors.New("unable to unpack to pointer specified")
}

// Unpack is used to unpack a value to a pointer.
func Unpack(Data []byte, Ptr interface{}) error {
	// Check if the ptr is actually a pointer.
	if reflect.ValueOf(Ptr).Kind() != reflect.Ptr {
		return errors.New("invalid pointer")
	}

	// The invalid erlpack handler.
	err := func() error {
		return errors.New("invalid erlpack bytes")
	}

	// Create a bytes reader.
	r := bytes.NewReader(Data)

	// Check the length.
	l := len(Data)
	if 2 > l {
		return err()
	}

	// Check the version.
	Version, _ := r.ReadByte()
	if Version != 131 {
		return err()
	}

	// Return the data unpacking.
	return processItem(Ptr, r)
}
