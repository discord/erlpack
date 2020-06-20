package erlpack

type scratchpad struct {
	// The allocation for the scratchpad.
	alloc []byte

	// Defines how many raw bytes are used out of the allocation.
	used uint

	// Defines rules on how the array should be constructed.
	rules []constructionRules

	// The initial allocation for the scratchpad.
	initAlloc uint
}

type constructionRules struct {
	// The start point of these bytes in the pad.
	start uint

	// The end point of these bytes in the pad.
	end uint
}

func newScratchpad(initAlloc uint) *scratchpad {
	return &scratchpad{
		alloc:     make([]byte, initAlloc),
		rules:     make([]constructionRules, 0),
		initAlloc: initAlloc,
		used:      0,
	}
}

// Add the raw bytes to memory.
func (s *scratchpad) addRaw(bytes ...byte) (start uint, end uint) {
	// Get the current position.
	CurrentPos := s.used

	// Get the length as a unsigned int.
	LengthUint := uint(len(bytes))

	// Add the number of bytes to the used marker.
	s.used += LengthUint

	// Is the amount that will be used greater than the length of the array.
	currentAlloc := uint(len(s.alloc))
	if s.used > currentAlloc {
		// It is. We should reallocate the array to handle this.
		realloc := make([]byte, currentAlloc+LengthUint+s.initAlloc)
		for i, v := range s.alloc {
			realloc[i] = v
		}
		s.alloc = realloc
	}

	// Add the bytes.
	for i := uint(0); i < LengthUint; i++ {
		s.alloc[CurrentPos+i] = bytes[i]
	}

	// Return the start and end.
	return CurrentPos, s.used
}

// Appends to the start of a scratchpad of bytes.
func (s *scratchpad) startAppend(bytes ...byte) {
	start, end := s.addRaw(bytes...)
	s.rules = append([]constructionRules{
		{
			start: start,
			end:   end,
		},
	}, s.rules...)
}

// Appends to the end of a scratchpad of bytes.
func (s *scratchpad) endAppend(bytes ...byte) {
	start, end := s.addRaw(bytes...)
	s.rules = append(s.rules, constructionRules{
		start: start,
		end:   end,
	})
}

// Turn the scratchpad into bytes.
func (s *scratchpad) bytes() []byte {
	arr := make([]byte, s.used)
	i := 0
	for _, rule := range s.rules {
		for x := rule.start; x < rule.end; x++ {
			arr[i] = s.alloc[x]
			i++
		}
	}
	return arr
}
