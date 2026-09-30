#!/usr/bin/env python3
"""Check fixture pairs and emit C test-only objects from shared semantic JSON."""
import json
from pathlib import Path
ROOT = Path(__file__).resolve().parent.parent


def byte_array(data): return ','.join(str(x) for x in data) or '0'

def main():
    lines = ['/* Test adapter generated from shared testdata, not codec output. */']
    entries=[]
    paths=sorted((ROOT/'testdata').glob('*/*.json'))
    for path in paths:
        if path.parent.name=='compatibility': continue
        v=json.loads(path.read_text())
        raw=bytes.fromhex(v['binary_hex'])
        assert path.with_suffix('.bin').read_bytes()==raw, path
        if v['expected_error']==0: assert v['semantic']==v['expected_decoded'], path
        index=len(entries)
        lines.append(f'static const uint8_t wire_{index}[] = {{{byte_array(raw)}}};')
        fields=[]
        for j,f in enumerate(v['semantic']['fields']):
            value=bytes.fromhex(f['hex'])
            lines.append(f'static const uint8_t val_{index}_{j}[] = {{{byte_array(value)}}};')
            fields.append(f'{{{f["id"]}, {len(value)}, val_{index}_{j}}}')
        s=v['semantic']
        # Invalid vectors only need their wire form; some cannot fit the public object.
        obj='{{0,0},0,0,0,0,{{0,0,NULL}}}'
        if v['expected_error']==0:
            obj=f'{{{{{s["major"]},{s["minor"]}}},{s["type"]},{s["flags"]},UINT64_C({s["sequence"]}),{len(fields)},{{'+','.join(fields)+'}}'
        entries.append(f'{{"{v["name"]}",wire_{index},{len(raw)},{v["expected_error"]},{obj}}}')
    # Avoid unused invalid semantic buffers in strict C builds.
    used='\n'.join(entries)
    lines=[line for line in lines if not line.startswith('static const uint8_t val_') or line.split('[]')[0].split()[-1] in used]
    lines.append('static const struct vector vectors[] = {\n'+',\n'.join(entries)+'\n};')
    cases=json.loads((ROOT/'testdata/compatibility/negotiation.json').read_text())
    rows=[]
    for c in cases:
        ranges=[]
        for key in ('a','b'):
            a,b,d,e=c[key]; ranges.append(f'{{{{{a},{b}}},{{{d},{e}}}}}')
        rows.append(f'{{.a={ranges[0]},.b={ranges[1]},.a_caps=UINT64_C({c["a_caps"]}),.b_caps=UINT64_C({c["b_caps"]}),.selected={{{c["selected"][0]},{c["selected"][1]}}},.caps=UINT64_C({c["caps"]}),.error={c["error"]}}}')
    lines.append('static const struct negotiation negotiations[] = {\n'+',\n'.join(rows)+'\n};')
    assert len(list((ROOT/'testdata/valid').glob('*.bin'))) + len(list((ROOT/'testdata/invalid').glob('*.bin'))) == len(entries), 'orphan binary'
    (ROOT/'build').mkdir(exist_ok=True)
    target = ROOT / 'build/vectors.inc'
    content = '\n'.join(lines) + '\n'
    if not target.exists() or target.read_text() != content:
        target.write_text(content)
    print(f'Checked {len(entries)} binary/semantic pairs and {len(cases)} negotiation cases')


if __name__=='__main__': main()
