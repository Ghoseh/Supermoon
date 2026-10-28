# SPDX-FileCopyrightText: 2026 Supermoon contributors
# SPDX-License-Identifier: MIT
"""Reproduce the pinned DLL pair in fresh disposable outputs, never install it."""
import argparse,json,shutil,re
from common import ROOT,compiler,digest,environment,flags,fresh,recipe,run
from normalize_pe import normalize
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--zig',required=True,help='Path to pinned Windows x64 zig.exe')
    parser.add_argument('--dry-run',action='store_true',help='Validate closure and print commands; do not compile')
    args=parser.parse_args();cc=compiler(args.zig);data=recipe();work=fresh('build')
    print('Disposable build output: '+str(work),flush=True)
    shutil.copytree(ROOT/'source',work/'source',copy_function=shutil.copyfile)
    for name in ['ebur128_peak.c','winmm.def']:shutil.copyfile(ROOT/name,work/name)
    source=work/'source';vendor=source/'vendor/libebur128/ebur128';options=flags(source)
    for name in data['payload_sources']+data['proxy_sources']:
        if not (source/name).is_file():raise FileNotFoundError('Missing production unit: '+name)
    if len(data['payload_sources'])!=19 or len(data['proxy_sources'])!=4:raise ValueError('Production unit list mismatch')
    meter=work/'ebur128.o';payload=work/'supermoon_playlist.dll';proxy=work/'winmm.dll'
    commands={
      'meter':[cc,'cc','-std=gnu99','-O2','-g0','-I'+str(vendor),'-I'+str(vendor/'queue'),work/'ebur128_peak.c','-c','-o',meter],
      'payload':[cc,'c++',*options,'-shared',*[source/n for n in data['payload_sources']],meter,'-lbcrypt','-luser32','-o',payload],
      'proxy':[cc,'c++',*options,'-shared',*[source/n for n in data['proxy_sources']],work/'winmm.def','-lbcrypt','-luser32','-o',proxy]}
    if args.dry_run:
        print(json.dumps({k:list(map(str,v)) for k,v in commands.items()},indent=2));return
    env=environment(work);run(commands['meter'],'meter',work,env);run(commands['payload'],'payload',work,env)
    normalize(payload)
    expected=data['DLLs']['supermoon_playlist.dll']
    if digest(payload)!=expected['sha256'] or payload.stat().st_size!=expected['bytes']:
        raise ValueError('Payload is not the qualified binary; no bootstrap pin was rewritten')
    text=(source/'winmm_bootstrap_config.hpp').read_text(encoding='utf-8')
    size=int(re.search(r'constexpr uint64_t adapter_bytes=(\d+);',text)[1])
    pin=bytes(int(v,16) for v in re.findall(r'0x([0-9a-fA-F]{2})',re.search(r'adapter_sha256\[\]=\{([^}]+)\}',text)[1])).hex()
    if pin!=digest(payload) or size!=payload.stat().st_size:raise ValueError('Existing bootstrap payload pin mismatch')
    run(commands['proxy'],'proxy',work,env);normalize(proxy)
    for path in [payload,proxy]:
        if digest(path)!=data['DLLs'][path.name]['sha256']:raise ValueError('Reproducibility mismatch: '+path.name)
    print('PASS: both normalized DLL hashes match the pinned pair. See qualification documentation. Nothing installed.')
if __name__=='__main__':main()
