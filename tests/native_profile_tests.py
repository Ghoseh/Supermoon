# SPDX-License-Identifier: MIT
"""Read-only native profile/recipe contract checks. Developer Python only.

Input game bytes are supplied locally, never executed or embedded in test source.
This verifies static identities, not game reachability or native lifetime safety.
"""
from pathlib import Path
import argparse,hashlib,json,re,struct,sys
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
from pe import PE

def digest(data):return hashlib.sha256(data).hexdigest()
def array(text,name):return [int(v,0) for v in re.search(name+r'\[\]\s*=\{([^}]+)\}',text)[1].split(',')]
def verify(image):
    image=Path(image);pe=PE(image);policy=json.loads((ROOT/'build/recipe.json').read_text())
    assert len(pe.data)==policy['game_executable_bytes'] and digest(pe.data)==policy['game_executable_sha256'],'Whole executable identity mismatch'
    def raw(rv,n):
        assert n>0
        for va,size,offset in pe.sections:
            if va<=rv and rv+n<=va+size:return pe.data[offset+rv-va:offset+rv-va+n]
        raise ValueError('Unbacked profile range')
    pins=[]
    for name,count in [('native_layout.hpp',7),('recovery_layout.hpp',12),('ui_profile.hpp',4),('time_profile.hpp',10),('register_profile.hpp',1),('recovery_pins.hpp',8)]:
        text=(ROOT/'source'/name).read_text()
        if 'layout' in name:
            text=text.split('inline constexpr Function functions[]={',1)[1].split('};',1)[0]
            rows=[(int(a),int(n),bytes(int(t) for t in h.split(','))) for a,o,s,n,h in re.findall(r'\{(\d+),(\d+),(\d+),(\d+),\{([^}]+)\}\}',text)]
        else:rows=[(int(a),int(n),bytes(int(t,0) for t in h.split(','))) for a,n,h in re.findall(r'\{(\d+),(\d+),\{([^}]+)\}\}',text)]
        assert len(rows)==count,(name,'Pin count')
        for rv,n,expected in rows:
            assert hashlib.sha256(raw(rv,n)).digest()==expected,(name,hex(rv),'Code pin mismatch')
            pins.append((name,rv,n))
    adapter=(ROOT/'source/supermoon_adapter.cpp').read_text()
    floor=re.findall(r'\{const uint8_t expected\[\]=\{([^}]+)\};if\(!memory_digest\(base\+(\d+),(\d+),expected\)\)return false;\}',adapter)
    assert len(floor)==4
    for expected,rv,n in floor:assert hashlib.sha256(raw(int(rv),int(n))).digest()==bytes(int(t,0) for t in expected.split(',')),('Inline SDK pin',rv)
    config=(ROOT/'source/winmm_bootstrap_config.hpp').read_text()
    bootstrap=(ROOT/'source/winmm_bootstrap.cpp').read_text()
    readiness=int(re.search(r'image_read\(base,(0x[0-9a-f]+),bytes,sizeof\(bytes\)\)',bootstrap)[1],16)
    assert hashlib.sha256(raw(readiness,32)).digest()==bytes(array(config,'readiness_setter_sha256'))
    expected_DLL=policy['DLLs']['supermoon_playlist.dll']
    assert bytes(array(config,'adapter_sha256')).hex()==expected_DLL['sha256']
    assert int(re.search(r'adapter_bytes=(\d+)',config)[1])==expected_DLL['bytes']
    identity=(ROOT/'source/pinned_build.hpp').read_text()
    assert bytes(int(v,0) for v in re.search(r'expected\[32\]=\{([^}]+)',identity)[1].split(',')).hex()==policy['game_executable_sha256']
    assert int(re.search(r'total==(\d+)',identity)[1])==policy['game_executable_bytes']
    base=pe.u64(pe.opt+24);cells=[];runtime_only=[]
    for name,slot_name,target_name in [('winmm_bootstrap_config.hpp','dispatch_slots','dispatch_targets'),('recovery_native.cpp','cells','targets'),('time.cpp','cells','targets')]:
        text=(ROOT/'source'/name).read_text();slots=array(text,slot_name);targets=array(text,target_name);assert len(slots)==len(targets)
        for index,(slot,target) in enumerate(zip(slots,targets)):
            try:value=struct.unpack('<Q',raw(slot,8))[0]
            except ValueError:
                assert name=='recovery_native.cpp' and index in (6,7),'Unexpected unbacked pointer cell'
                assert 0<slot<pe.u32(pe.opt+56)-8;runtime_only.append((name,slot));continue
            assert value==base+target,(name,hex(slot),'Pointer target mismatch');cells.append((name,slot,target))
    assert len(runtime_only)==2
    for token in ['slot','library_slot']:
        profile=(ROOT/'source/ui_profile.hpp').read_text();slot=int(re.search(r'\b'+token+r'=(0x[0-9a-f]+)',profile)[1],16);target=int(re.search(r'formatter=(0x[0-9a-f]+)',profile)[1],16)
        assert struct.unpack('<Q',raw(slot,8))[0]==base+target;cells.append(('ui_profile.hpp',slot,target))
    slots=array(adapter,'slots');span=int(re.search(r'constexpr size_t span=(0x[0-9a-f]+)',adapter)[1],16)
    first=int(re.search(r'auto first=base\+(0x[0-9a-f]+)',adapter)[1],16)
    assert first==min(slots) and span==max(slots)+8-first,'Dispatch protection span'
    return dict(result='PASS_STATIC_IDENTITIES',executable_sha256=digest(pe.data),code_pins=len(pins),inline_SDK_pins=4,readiness_pin=True,file_backed_pointer_checks=len(cells),runtime_initialized_cells_not_live_verified=len(runtime_only),dispatch_span=span,scope='No instructions executed; no game or filesystem mutation.')

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--image',required=True);a=p.parse_args()
    try:print(json.dumps(verify(a.image),indent=2))
    except (AssertionError,ValueError,OSError,IndexError) as e:print('REFUSED: '+str(e),file=sys.stderr);sys.exit(1)
