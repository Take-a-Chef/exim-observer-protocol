#!/usr/bin/env python3
"""Explicit golden update only: independently serialize fixed semantic objects."""
import json
import pathlib
import struct

ROOT = pathlib.Path(__file__).resolve().parent.parent


def u64(n): return struct.pack('!Q', n).hex()
def txt(s): return s.encode().hex()
def frame(kind, fields, sequence=0, minor=1):
    return {'major': 0, 'minor': minor, 'type': kind, 'flags': 0, 'sequence': sequence,
            'fields': [{'id': i, 'hex': h} for i, h in fields]}

def encode(f):
    payload = b''.join(struct.pack('!HH', x['id'], len(bytes.fromhex(x['hex']))) + bytes.fromhex(x['hex']) for x in f['fields'])
    return struct.pack('!4sBBHIIQ', b'EXOB', f['major'], f['minor'], f['type'], f['flags'], len(payload), f['sequence']) + payload


def save(name, f, code=0, wire=None):
    kind = 'invalid' if code else 'valid'
    wire = encode(f) if wire is None else wire
    path = ROOT / 'testdata' / kind / ('draft01-' + name)
    data = {'name': name, 'protocol_version': '0.1', 'semantic': f, 'expected_decoded': f if not code else None,
            'expected_error': code, 'binary_hex': wire.hex()}
    path.with_suffix('.json').write_text(json.dumps(data, indent=2)+'\n')
    path.with_suffix('.bin').write_bytes(wire)


hello = frame(1, [(1,'0001'), (2,'0001'), (4,u64(16387))])
accepted = frame(0x102, [(6,txt('mx-1')), (7,txt('track-1')), (8,txt('1abc-000001-XY')), (10,txt('event-42')), (11,u64(1700000000000000)), (15,txt('sender@example.org'))], 42)
error = frame(3,[(5,u64(7)),(13,'0002'),(14,txt('No overlap: café'))])
for n,f in [('hello',hello),('hello-ack',frame(2,[(3,'0001'),(4,u64(1))])),('hello-incompatible-range',frame(1,[(1,'0100'),(2,'0102'),(4,u64(1))])),('message-accepted',accepted),('queue-count',frame(0x202,[(5,u64(7))])),('queue-count-response',frame(0x402,[(5,u64(7)),(12,u64(19))])),('error',error),('optional-extension',frame(1,[(1,'0001'),(2,'0001'),(4,u64(1)),(100,'abcd')],minor=2)),('empty-optional-text',frame(3,[(13,'0001'),(14,'')]))]: save(n,f)
raw=encode(hello)
for n,b in [('empty',b''),('wrong-magic',b'NOPE'+raw[4:]),('truncated-header',raw[:23]),('truncated-payload',raw[:-1]),('trailing-byte',raw+b'\x00'),('oversized-payload',raw[:12]+struct.pack('!I',65537)+raw[16:]),('integer-overflow',raw[:12]+b'\xff'*4+raw[16:]),('truncated-tlv',raw[:12]+struct.pack('!I',1)+raw[16:24]+b'\x00'),('tlv-overrun',raw[:26]+b'\xff\xff'+raw[28:])]: save(n,hello,1,b)
for n,f,code in [
 ('unknown-message',frame(0x999,[]),3),('reserved-event',frame(0x101,[]),3),
 ('unknown-required',frame(1,[(1,'0001'),(2,'0001'),(4,u64(1)),(0x8001,'')]),5),
 ('duplicate-fields',frame(1,[(1,'0001'),(1,'0001'),(2,'0001'),(4,u64(1))]),1),
 ('unordered-fields',frame(1,[(2,'0001'),(1,'0001'),(4,u64(1))]),1),
 ('missing-field',frame(1,[(1,'0001'),(2,'0001')]),5),
 ('invalid-range',frame(1,[(1,'0002'),(2,'0001'),(4,u64(1))]),5),
 ('zero-request-id',frame(0x202,[(5,u64(0))]),5),
 ('wrong-integer-width',frame(0x202,[(5,'01')]),5),
 ('invalid-enum',frame(3,[(13,'ffff')]),5),
 ('ok-in-error',frame(3,[(13,'0000')]),5),
 ('invalid-utf8',frame(3,[(13,'0001'),(14,'c080')]),5),
 ('surrogate-utf8',frame(3,[(13,'0001'),(14,'eda080')]),5),
 ('unicode-overflow',frame(3,[(13,'0001'),(14,'f4908080')]),5),
 ('nul-text',frame(3,[(13,'0001'),(14,'610062')]),5),
 ('oversized-error-text',frame(3,[(13,'0001'),(14,'61'*1025)]),5),
 ('too-many-fields',frame(1,[(1,'0001'),(2,'0001'),(4,u64(1))]+[(i,'') for i in range(100,130)]),1),
 ('forbidden-known-field',frame(0x202,[(5,u64(1)),(6,txt('mx'))]),5),
 ('event-zero-sequence',frame(0x102,[(6,'61'),(8,'62'),(10,'63'),(11,u64(0))]),5),
 ('empty-identifier',frame(0x102,[(6,''),(8,'62'),(10,'63'),(11,u64(0))],1),5),
 ('oversized-tracking-id',frame(0x102,[(6,'61'),(7,'61'*129),(8,'62'),(10,'63'),(11,u64(0))],1),5),
 ('invalid-identifier',frame(0x102,[(6,'612062'),(8,'62'),(10,'63'),(11,u64(0))],1),5)]: save(n,f,code)
save('maximum-fields',frame(1,[(1,'0001'),(2,'0001'),(4,u64(1))]+[(i,'') for i in range(100,129)]))
save('zero-tag',frame(1,[(0,''),(1,'0001'),(2,'0001'),(4,u64(1))]),1)
save('uint64-maximum',frame(0x402,[(5,u64(2**64-1)),(12,u64(2**64-1))]))
save('unicode-boundaries',frame(3,[(13,'0001'),(14,txt('\u0080\u07ff\u0800\ud7ff\ue000\U00010000\U0010ffff'))]))
f=dict(hello); f['major']=1; save('unsupported-major',f,2)
f=dict(hello); f['minor']=0; save('unsupported-minor',f,2)
f=dict(hello); f['flags']=1; save('reserved-flags',f,1)
f=dict(hello); f['sequence']=1; save('control-sequence',f,5)
# Maximum payload exactly, using unknown optional values (no application semantics).
f=frame(1,[(1,'0001'),(2,'0001'),(4,u64(1)),(100,'61'*(65536-28))]); save('maximum-payload',f)
f=frame(1,[(1,'0001'),(2,'0001'),(4,u64(1)),(100,'61'*(65537-28))]); save('one-over-maximum',f,1)
cases=[{'name':'overlap','a':[1,0,1,2],'b':[1,1,1,4],'a_caps':9,'b_caps':3,'selected':[1,2],'caps':1,'error':0}, {'name':'no-major-overlap','a':[1,0,1,1],'b':[2,0,2,1],'a_caps':1,'b_caps':1,'selected':[0,0],'caps':0,'error':2}, {'name':'no-minor-overlap','a':[1,0,1,1],'b':[1,2,1,4],'a_caps':1,'b_caps':1,'selected':[0,0],'caps':0,'error':2}, {'name':'reversed','a':[1,2,1,0],'b':[1,0,1,4],'a_caps':0,'b_caps':0,'selected':[0,0],'caps':0,'error':2}, {'name':'mixed-major-range','a':[0,1,1,0],'b':[1,0,1,4],'a_caps':0,'b_caps':0,'selected':[0,0],'caps':0,'error':2}, {'name':'no-capabilities','a':[0,1,0,1],'b':[0,1,0,1],'a_caps':1,'b_caps':2,'selected':[0,1],'caps':0,'error':0}]
(ROOT/'testdata/compatibility/negotiation.json').write_text(json.dumps(cases,indent=2)+'\n')
