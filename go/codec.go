package protocol

import "encoding/binary"

// Decode validates exactly one complete frame. Field values borrow input;
// callers must keep it unchanged until finished with the returned frame.
// On failure no partial frame is returned. Allocation is bounded independently
// of peer-supplied lengths.
func Decode(input []byte) (Frame, error) {
	if len(input) < HeaderSize || string(input[:4]) != "EXOB" {
		return Frame{}, ErrorInvalidFrame
	}
	payload := binary.BigEndian.Uint32(input[12:16])
	if payload > MaxPayload || uint64(payload) != uint64(len(input)-HeaderSize) {
		return Frame{}, ErrorInvalidFrame
	}
	f := Frame{Version: Version{input[4], input[5]}, Type: binary.BigEndian.Uint16(input[6:8]), Flags: binary.BigEndian.Uint32(input[8:12]), Sequence: binary.BigEndian.Uint64(input[16:24])}
	for pos := HeaderSize; pos < len(input); {
		if len(input)-pos < 4 || len(f.Fields) == MaxFields {
			return Frame{}, ErrorInvalidFrame
		}
		id := binary.BigEndian.Uint16(input[pos : pos+2])
		n := int(binary.BigEndian.Uint16(input[pos+2 : pos+4]))
		pos += 4
		if n > len(input)-pos {
			return Frame{}, ErrorInvalidFrame
		}
		f.Fields = append(f.Fields, Field{id, input[pos : pos+n]})
		pos += n
	}
	if err := Validate(f); err != nil {
		return Frame{}, err
	}
	return f, nil
}

// Encode validates a frame and returns newly allocated canonical bytes.
// It never sorts, truncates or silently removes fields.
func Encode(f Frame) ([]byte, error) {
	if err := Validate(f); err != nil {
		return nil, err
	}
	size := HeaderSize
	for _, v := range f.Fields {
		size += 4 + len(v.Value)
	}
	out := make([]byte, size)
	copy(out, "EXOB")
	out[4] = f.Version.Major
	out[5] = f.Version.Minor
	binary.BigEndian.PutUint16(out[6:8], f.Type)
	binary.BigEndian.PutUint32(out[8:12], f.Flags)
	binary.BigEndian.PutUint32(out[12:16], uint32(size-HeaderSize))
	binary.BigEndian.PutUint64(out[16:24], f.Sequence)
	pos := HeaderSize
	for _, v := range f.Fields {
		binary.BigEndian.PutUint16(out[pos:pos+2], v.ID)
		binary.BigEndian.PutUint16(out[pos+2:pos+4], uint16(len(v.Value)))
		copy(out[pos+4:], v.Value)
		pos += 4 + len(v.Value)
	}
	return out, nil
}
