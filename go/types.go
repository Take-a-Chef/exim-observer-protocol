// Package protocol implements the experimental Exim Observer wire protocol.
// It contains no transport, Exim, or agent implementation.
package protocol

import "strconv"

// Version identifies a wire version, independently of software versions.
type Version struct{ Major, Minor uint8 }

// Range is an inclusive range within one major version.
type Range struct{ Min, Max Version }

// Field is an ordered TLV. Values returned by Decode borrow its input.
type Field struct {
	ID    uint16
	Value []byte
}

// Frame is one complete message. Fields must be strictly ordered by ID.
type Frame struct {
	Version  Version
	Type     uint16
	Flags    uint32
	Sequence uint64
	Fields   []Field
}

// Code is a stable machine-readable protocol error; compare with errors.Is.
type Code uint16

func (c Code) Error() string { return "exob protocol error " + strconv.Itoa(int(c)) }

// Find returns a borrowed field value, or nil if absent. Validate first when
// interpreting required fields. An optional empty field may also return nil.
func (f Frame) Find(id uint16) []byte {
	for _, v := range f.Fields {
		if v.ID == id {
			return v.Value
		}
	}
	return nil
}
