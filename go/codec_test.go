package protocol

import (
	"bytes"
	"encoding/hex"
	"encoding/json"
	"errors"
	"os"
	"path/filepath"
	"reflect"
	"testing"
)

type semantic struct {
	Major, Minor uint8
	Type         uint16
	Flags        uint32
	Sequence     uint64
	Fields       []struct {
		ID  uint16
		Hex string
	}
}

func (s semantic) frame(t *testing.T) Frame {
	t.Helper()
	f := Frame{Version: Version{s.Major, s.Minor}, Type: s.Type, Flags: s.Flags, Sequence: s.Sequence}
	for _, v := range s.Fields {
		b, err := hex.DecodeString(v.Hex)
		if err != nil {
			t.Fatal(err)
		}
		f.Fields = append(f.Fields, Field{v.ID, b})
	}
	return f
}
func TestSharedVectors(t *testing.T) {
	paths, err := filepath.Glob("../testdata/*/*.bin")
	if err != nil || len(paths) == 0 {
		t.Fatal("missing shared vectors", err)
	}
	for _, path := range paths {
		t.Run(filepath.Base(path), func(t *testing.T) {
			var v struct {
				Semantic        semantic
				ExpectedDecoded semantic `json:"expected_decoded"`
				ExpectedError   Code     `json:"expected_error"`
				BinaryHex       string   `json:"binary_hex"`
			}
			meta, err := os.ReadFile(path[:len(path)-4] + ".json")
			if err != nil {
				t.Fatal(err)
			}
			if err = json.Unmarshal(meta, &v); err != nil {
				t.Fatal(err)
			}
			wire, err := os.ReadFile(path)
			if err != nil {
				t.Fatal(err)
			}
			if hex.EncodeToString(wire) != v.BinaryHex {
				t.Fatal("binary differs from JSON")
			}
			got, err := Decode(wire)
			if v.ExpectedError != ErrorOK {
				if !errors.Is(err, v.ExpectedError) || !reflect.DeepEqual(got, Frame{}) {
					t.Fatalf("expected %v, got %+v %v", v.ExpectedError, got, err)
				}
				return
			}
			if err != nil {
				t.Fatal(err)
			}
			expected := v.ExpectedDecoded.frame(t)
			if !reflect.DeepEqual(got, expected) {
				t.Fatalf("decoded semantic mismatch: %+v", got)
			}
			for _, f := range []Frame{v.Semantic.frame(t), got} {
				encoded, err := Encode(f)
				if err != nil {
					t.Fatal(err)
				}
				if !bytes.Equal(encoded, wire) {
					t.Fatal("golden mismatch")
				}
			}
		})
	}
}
func TestNegotiation(t *testing.T) {
	var cases []struct {
		Name     string
		A, B     [4]uint8
		ACaps    uint64 `json:"a_caps"`
		BCaps    uint64 `json:"b_caps"`
		Selected [2]uint8
		Caps     uint64
		Error    Code
	}
	raw, err := os.ReadFile("../testdata/compatibility/negotiation.json")
	if err != nil {
		t.Fatal(err)
	}
	if err = json.Unmarshal(raw, &cases); err != nil {
		t.Fatal(err)
	}
	for _, c := range cases {
		t.Run(c.Name, func(t *testing.T) {
			r := func(x [4]uint8) Range { return Range{Version{x[0], x[1]}, Version{x[2], x[3]}} }
			v, caps, err := Negotiate(r(c.A), r(c.B), c.ACaps, c.BCaps)
			if v != (Version{c.Selected[0], c.Selected[1]}) || caps != c.Caps || (c.Error == 0 && err != nil) || (c.Error != 0 && !errors.Is(err, c.Error)) {
				t.Fatalf("%+v %d %v", v, caps, err)
			}
		})
	}
}
func TestEncodeBounds(t *testing.T) {
	raw, err := os.ReadFile("../testdata/valid/draft01-hello.bin")
	if err != nil {
		t.Fatal(err)
	}
	f, err := Decode(raw)
	if err != nil {
		t.Fatal(err)
	}
	f.Fields = append(f.Fields, Field{123, make([]byte, 65536)})
	if _, err := Encode(f); !errors.Is(err, ErrorInvalidFrame) {
		t.Fatal(err)
	}
	f.Fields = f.Fields[:3]
	f.Fields[1].ID = f.Fields[0].ID
	if _, err := Encode(f); !errors.Is(err, ErrorInvalidFrame) {
		t.Fatal(err)
	}
}
func FuzzDecode(f *testing.F) {
	paths, err := filepath.Glob("../testdata/*/*.bin")
	if err != nil || len(paths) == 0 {
		f.Fatal("missing seeds", err)
	}
	for _, p := range paths {
		raw, err := os.ReadFile(p)
		if err != nil {
			f.Fatal(err)
		}
		f.Add(raw)
	}
	f.Fuzz(func(t *testing.T, raw []byte) {
		frame, err := Decode(raw)
		if err != nil {
			return
		}
		encoded, err := Encode(frame)
		if err != nil || !bytes.Equal(raw, encoded) {
			t.Fatalf("noncanonical round trip: %v", err)
		}
	})
}
