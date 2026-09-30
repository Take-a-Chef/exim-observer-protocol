package protocol

import (
	"bytes"
	"encoding/binary"
	"unicode/utf8"
)

type (
	fieldRule   struct{ max, kind int }
	messageRule struct{ required, allowed uint32 }
)

func validRange(r Range) bool { return r.Min.Major == r.Max.Major && r.Min.Minor <= r.Max.Minor }
func validField(v []byte, r fieldRule) bool {
	if len(v) > r.max {
		return false
	}
	switch r.kind {
	case 0, 1:
		return len(v) == r.max
	case 2:
		return len(v) == 8 && binary.BigEndian.Uint64(v) != 0
	case 3:
		if len(v) == 0 {
			return false
		}
		for _, c := range v {
			if !(c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z' || c >= '0' && c <= '9' || bytes.IndexByte([]byte("._:@+-"), c) >= 0) { //nolint:staticcheck // Positive allowed-character groups mirror the C validator.
				return false
			}
		}
		return true
	case 4:
		return utf8.Valid(v) && bytes.IndexByte(v, 0) < 0
	case 5:
		return len(v) == 2 && binary.BigEndian.Uint16(v) > uint16(ErrorOK) &&
			binary.BigEndian.Uint16(v) <= uint16(ErrorTemporaryError)
	}
	return false
}

// Validate enforces framing bounds, canonical ordering and message schemas.
func Validate(f Frame) error {
	if f.Version.Major != Major || f.Version.Minor < Minor {
		return ErrorUnsupportedVersion
	}
	if f.Flags != 0 || len(f.Fields) > MaxFields {
		return ErrorInvalidFrame
	}
	rule, ok := messageRules[f.Type]
	if !ok {
		return ErrorUnsupportedMessage
	}
	if (f.Type == MsgMessageAccepted) != (f.Sequence != 0) {
		return ErrorInvalidRequest
	}
	var seen uint32
	var previous uint16
	size := 0
	for _, v := range f.Fields {
		if v.ID == 0 || v.ID <= previous || len(v.Value) > 65535 {
			return ErrorInvalidFrame
		}
		previous = v.ID
		if len(v.Value)+4 > MaxPayload-size {
			return ErrorInvalidFrame
		}
		size += len(v.Value) + 4
		r, known := fieldRules[v.ID]
		if !known {
			if v.ID&0x8000 != 0 {
				return ErrorInvalidRequest
			}
			continue
		}
		bit := uint32(1) << v.ID
		if rule.allowed&bit == 0 || !validField(v.Value, r) {
			return ErrorInvalidRequest
		}
		seen |= bit
	}
	if seen&rule.required != rule.required {
		return ErrorInvalidRequest
	}
	if f.Type == MsgHello {
		a, b := f.Find(FieldMinVersion), f.Find(FieldMaxVersion)
		if len(a) != 2 || len(b) != 2 || !validRange(Range{Version{a[0], a[1]}, Version{b[0], b[1]}}) {
			return ErrorInvalidRequest
		}
	}
	return nil
}

// Negotiate chooses the highest common version and intersects capability bits.
// An error returns a zero version and zero capabilities.
func Negotiate(a, b Range, aCaps, bCaps uint64) (Version, uint64, error) {
	if !validRange(a) || !validRange(b) || a.Min.Major != b.Min.Major {
		return Version{}, 0, ErrorUnsupportedVersion
	}
	lo := max(a.Min.Minor, b.Min.Minor)
	hi := min(a.Max.Minor, b.Max.Minor)
	if lo > hi {
		return Version{}, 0, ErrorUnsupportedVersion
	}
	return Version{a.Min.Major, hi}, aCaps & bCaps, nil
}
