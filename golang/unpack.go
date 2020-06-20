package erlpack

import (
	"bytes"
	"encoding/binary"
	"errors"
	"github.com/jakemakesstuff/structs"
	"reflect"
	"unsafe"
)

// Atom is used to define an atom within the codebase.
type Atom string

// Used to cast the item.
func handleItemCasting(Item, Ptr interface{}) error {
	// Get the reflect value.
	r := reflect.ValueOf(Ptr)

	// Handle double pointers.
	if r.Elem().Kind() == reflect.Ptr {
		// Create the inner pointer.
		ptr := reflect.New(r.Elem().Type().Elem())

		// Call this function.
		err := handleItemCasting(Item, ptr.Interface())
		if err != nil {
			return err
		}

		// Set the element.
		r.Elem().Set(ptr)
		return nil
	}

	// Handle a interface.
	switch x := Ptr.(type) {
	case *interface{}:
		*x = Item
		return nil
	}

	// Handle specific type casting.
	switch x := Item.(type) {
	case Atom:
		switch y := Ptr.(type) {
		case *Atom:
			*y = x
			return nil
		}
	case int64:
		switch p := Ptr.(type) {
		case *int:
			*p = int(x)
		case *int64:
			*p = x
		default:
			return errors.New("could not de-serialize into int")
		}
		return nil
	case int32:
		switch p := Ptr.(type) {
		case *int:
			*p = int(x)
		case *int32:
			*p = x
		default:
			return errors.New("could not de-serialize into int")
		}
		return nil
	case float64:
		switch p := Ptr.(type) {
		case *float64:
			*p = x
		default:
			return errors.New("could not de-serialize into float64")
		}
		return nil
	case uint8:
		switch p := Ptr.(type) {
		case *uint:
			*p = uint(x)
		case *uint8:
			*p = x
		case *int:
			*p = int(x)
		default:
			return errors.New("could not de-serialize into uint8")
		}
		return nil
	case string:
		// Map key.
		switch p := Ptr.(type) {
		case *string:
			*p = x
		default:
			return errors.New("could not de-serialize into string")
		}
		return nil
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
		default:
			// Zero the pointer which is provided.
			// TODO: figure out this
		}
		return nil
	case []interface{}:
		// We should handle this array.
		switch p := Ptr.(type) {
		case *[]interface{}:
			// This is simple.
			*p = x
			return nil
		default:
			// Create the new array.
			a := reflect.MakeSlice(r.Elem().Type(), len(x), len(x))

			// Set all the items.
			for i, v := range x {
				indexItem := a.Index(i)
				x := reflect.New(indexItem.Type())
				t := x.Interface()
				err := handleItemCasting(v, t)
				if err != nil {
					return err
				}
				indexItem.Set(x.Elem())
			}

			// Set the pointer to the item.
			e := reflect.ValueOf(p).Elem()
			e.Set(a)

			// Return no errors.
			return nil
		}
	case map[interface{}]interface{}:
		// Maps are complicated since they can serialize into a lot of different types.

		switch p := Ptr.(type) {
		case *map[interface{}]interface{}:
			// This is the first thing we check for since it is by far the best situation.
			*p = x
			return nil
		}

		// Check the type of the pointer.
		switch r.Elem().Kind() {
		case reflect.Struct:
			// Make the new struct.
			i := reflect.New(r.Elem().Type())

			// Get the struct object.
			s := structs.New(i.Interface())
			s.TagName = "erlpack"

			// Set tag > field.
			tag2field := map[string]string{}
			for _, field := range s.Fields() {
				t := field.Tag("erlpack")
				if t != "" && t != "-" {
					tag2field[t] = field.Name()
				}
			}

			// Iterate through the map.
			for k, v := range x {
				switch str := k.(type) {
				case string:
					fieldName, ok := tag2field[str]
					if !ok {
						continue
					}
					field, ok := s.FieldOk(fieldName)
					if !ok {
						return errors.New("failed to get field")
					}
					r := reflect.New(field.Type())
					x := r.Interface()
					err := handleItemCasting(v, x)
					if err != nil {
						return err
					}
					err = field.Set(r.Elem().Interface())
					if err != nil {
						return err
					}
				default:
					return errors.New("key must be string")
				}
			}

			// Set to the interface.
			r.Elem().Set(i.Elem())

			// Return no errors.
			return nil
		case reflect.Map:
			// Make the new map.
			m := reflect.MakeMap(r.Elem().Type())

			// Get the key type.
			keyType := m.Type().Key()

			// Get the value type.
			valueType := m.Type().Elem()

			// Iterate through the map.
			for k, v := range x {
				// Create a new version of the key with the reflect type.
				reflectKey := reflect.New(keyType)
				iface := reflectKey.Interface()

				// Handle the item casting for the key.
				err := handleItemCasting(k, iface)
				if err != nil {
					return err
				}

				// Create a new version of the value with the reflect type.
				reflectValue := reflect.New(valueType)
				iface = reflectValue.Interface()

				// Handle the item casting for the value.
				err = handleItemCasting(v, iface)
				if err != nil {
					return err
				}

				// Set the item.
				m.SetMapIndex(reflectKey.Elem(), reflectValue.Elem())
			}

			// Set the pointer to this map.
			r.Elem().Set(m)

			// Return no errors.
			return nil
		}
	}

	// Return unknown type error.
	return errors.New("unable to unpack to pointer specified")
}

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
	case 'a': // Small int.
		i, err := r.ReadByte()
		if err != nil {
			return errors.New("failed to read small int")
		}
		Item = i
	case 'b': // int32
		b := make([]byte, 4)
		_, err := r.Read(b)
		if err != nil {
			return errors.New("not enough bytes for int32")
		}
		l := binary.BigEndian.Uint32(b)
		Item = *(*int32)(unsafe.Pointer(&l))
	case 'n': // int64
		// Get the number of encoded bytes.
		encodedBytes, err := r.ReadByte()
		if err != nil {
			return errors.New("unable to read int64 byte count")
		}

		// Get the signature.
		signatureChar, err := r.ReadByte()
		if err != nil {
			return errors.New("unable to read int64 signature")
		}
		negative := signatureChar == 1

		// Create the uint64.
		u := uint64(0)

		// Decode the int64.
		x := uint64(0)
		for i := 0; i < int(encodedBytes); i++ {
			// Read the next byte.
			b, err := r.ReadByte()
			if err != nil {
				return errors.New("int64 length greater than array")
			}

			// Add the byte.
			u += uint64(b) * x
			x <<= 8
		}

		// Turn the uint64 into a int64.
		if negative {
			Item = int64(u) * -1
		} else {
			Item = int64(u)
		}
	case 'F': // float
		// Get the next 8 bytes.
		encodedBytes := make([]byte, 8)

		// Read said encoded bytes.
		_, err := r.Read(encodedBytes)
		if err != nil {
			return errors.New("not enough bytes to decode")
		}

		// Get the item as a uint64.
		i := binary.BigEndian.Uint64(encodedBytes)

		// Turn it into a float64.
		Item = *(*float64)(unsafe.Pointer(&i))
	case 't': // map
		// Get the length.
		b := make([]byte, 4)
		_, err := r.Read(b)
		if err != nil {
			return errors.New("not enough bytes for int32")
		}
		l := binary.BigEndian.Uint32(b)

		// Create the map.
		m := make(map[interface{}]interface{}, l)

		// Get each item from the map.
		for i := uint32(0); i < l; i++ {
			// Get the key.
			var Key interface{}
			err := processItem(&Key, r)
			if err != nil {
				return err
			}
			switch x := Key.(type) {
			case []byte:
				// bytes should be stored as strings for maps
				Key = string(x)
			}

			// Get the value.
			var Value interface{}
			err = processItem(&Value, r)
			if err != nil {
				return err
			}

			// Set the key to the value specified.
			m[Key] = Value
		}

		// Set the item to the map.
		Item = m
	default: // Don't know this data type.
		return errors.New("unknown data type")
	}

	// Handle the item casting.
	return handleItemCasting(Item, Ptr)
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
