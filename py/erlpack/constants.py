FORMAT_VERSION = 131

NEW_FLOAT_EXT = 'F'      # 70  [Float64:IEEE float]
BIT_BINARY_EXT = 'M'     # 77  [UInt32:Len, UInt8:Bits, Len:Data]
SMALL_INTEGER_EXT = 'a'  # 97  [UInt8:Int]
INTEGER_EXT = 'b'        # 98  [Int32:Int]
FLOAT_EXT = 'c'          # 99  [31:Float String] Float in string format (formatted "%.20e", sscanf "%lf"). Superseded by NEW_FLOAT_EXT
ATOM_EXT = 'd'           # 100 [UInt16:Len, Len:AtomName] max Len is 255
REFERENCE_EXT = 'e'      # 101 [atom:Node, UInt32:ID, UInt8:Creation]
PORT_EXT = 'f'           # 102 [atom:Node, UInt32:ID, UInt8:Creation]
PID_EXT = 'g'            # 103 [atom:Node, UInt32:ID, UInt32:Serial, UInt8:Creation]
SMALL_TUPLE_EXT = 'h'    # 104 [UInt8:Arity, N:Elements]
LARGE_TUPLE_EXT = 'i'    # 105 [UInt32:Arity, N:Elements]
NIL_EXT = 'j'            # 106 empty list
STRING_EXT = 'k'         # 107 [UInt32:Len, Len:Characters]
LIST_EXT = 'l'           # 108 [UInt32:Len, Elements, Tail]
BINARY_EXT = 'm'         # 109 [UInt32:Len, Len:Data]
SMALL_BIG_EXT = 'n'      # 110 [UInt8:n, UInt8:Sign, n:nums]
LARGE_BIG_EXT = 'o'      # 111 [UInt32:n, UInt8:Sign, n:nums]
NEW_FUN_EXT = 'p'        # 112 [UInt32:Size, UInt8:Arity, 16*Uint6-MD5:Uniq, UInt32:Index, UInt32:NumFree, atom:Module, int:OldIndex, int:OldUniq, pid:Pid, NunFree*ext:FreeVars]
EXPORT_EXT = 'q'         # 113 [atom:Module, atom:Function, smallint:Arity]
NEW_REFERENCE_EXT = 'r'  # 114 [UInt16:Len, atom:Node, UInt8:Creation, Len*UInt32:ID]
SMALL_ATOM_EXT = 's'     # 115 [UInt8:Len, Len:AtomName]
MAP_EXT = 't'            # 116 [UInt32:Airty, N:Pairs]
FUN_EXT = 'u'            # 117 [UInt4:NumFree, pid:Pid, atom:Module, int:Index, int:Uniq, NumFree*ext:FreeVars]
COMPRESSED = 'P'         # 80  [UInt4:UncompressedSize, N:ZlibCompressedData]

