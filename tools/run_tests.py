# SPDX-FileCopyrightText: 2026 Supermoon contributors
# SPDX-License-Identifier: MIT
"""Build/run a synthetic-input public regression subset in fresh disposable output."""
import argparse,sys,json,shutil
from pathlib import Path
from common import ROOT,compiler,environment,flags,fresh,run
sys.path.insert(0,str(ROOT/'tests'))
from parser_cases import corpus
def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--zig',required=True)
    parser.add_argument('--dry-run',action='store_true');parser.add_argument('--ubsan',action='store_true')
    parser.add_argument('--only-startup',action='store_true',help='Startup policy plus final F1 guard-page corpus only')
    parser.add_argument('--suite',action='append',help='Select named suite(s), plus the final F1 corpus')
    parser.add_argument('--proxy',help='Optional already-built qualified winmm.dll for the separate wrong-host load test')
    args=parser.parse_args();cc=compiler(args.zig);work=fresh('tests');print('Disposable test output: '+str(work),flush=True)
    env=environment(work);source=ROOT/'source';vendor=source/'vendor/libebur128/ebur128'
    options=flags(source)+(['-fsanitize=undefined','-fno-sanitize-recover=all'] if args.ubsan else [])
    stream=[source/'playlist/stream_engine.cpp',source/'playlist/notifications.cpp']
    wav=[source/'playlist/wav_pipeline.cpp',source/'playlist/mp3_reader.cpp',source/'playlist/normalization.cpp',work/'ebur128.o']
    cache=[source/'playlist/persistent_cache.cpp',source/'playlist/library.cpp']
    keys=[source/'playlist/keyboard_settings.cpp',source/'playlist/windows_keyboard.cpp']
    groups={'startup_policy_tests':[],
      'wav_pipeline_tests':wav,'cache_tests':[*stream,*wav,*cache],
      'notification_tests':[*stream,*wav],'notification_peak_tests':[*stream,*wav],
      'stream_engine_tests':[*stream,*wav,source/'playlist/windows_controls.cpp'],
      'endurance_tests':[],'keyboard_tests':[*stream,*wav,*keys],
      'forwarding_tests':['-DWINMM_FORWARD_SELFTEST',source/'playlist/winmm_forwarding.cpp',source/'winmm_forwarders.S'],
      'compatibility_tests':[source/'playlist/compatibility.cpp'],
      'recovery_tests':['-DCOMPATIBILITY_SELFTEST',source/'playlist/compatibility.cpp']}
    if args.only_startup:groups={'startup_policy_tests':[]}
    if args.suite:
        unknown=set(args.suite)-set(groups)
        if unknown:raise ValueError('Unknown suite: '+str(sorted(unknown)))
        groups={n:v for n,v in groups.items() if n in args.suite}
    if args.proxy:
        proxy=Path(args.proxy).resolve(strict=True)
        from common import digest,recipe
        if digest(proxy)!=recipe()['DLLs']['winmm.dll']['sha256']:raise ValueError('Proxy identity mismatch')
        shutil.copyfile(proxy,work/'winmm.dll')
    commands=[]
    if any(work/'ebur128.o' in extra for extra in groups.values()):
        commands.append(('compile_meter',[cc,'cc','-std=gnu99','-O2','-g0','-I'+str(vendor),'-I'+str(vendor/'queue'),ROOT/'ebur128_peak.c','-c','-o',work/'ebur128.o']))
    for name,extra in groups.items():
        src=ROOT/'tests'/(name+'.cpp');unicode=['-municode'] if 'int wmain(' in src.read_text(encoding='utf-8') else []
        commands.append(('compile_'+name,[cc,'c++',*options,*unicode,src,*extra,'-lbcrypt','-luser32','-o',work/(name+'.exe')]))
        commands.append((name,[work/(name+'.exe'),*(['--without-proxy'] if name=='forwarding_tests' and not args.proxy else [])]))
    f1=work/'f1_native.exe';manifest=corpus(work)
    commands.append(('compile_f1_guard',[cc,'c++',*options,'-DEXPECT_F1_CORRECTED=1',ROOT/'tests/f1_native.cpp','-o',f1]))
    commands.append(('f1_guard',[f1,manifest]))
    for label,cmd in commands:
        if args.dry_run:print(json.dumps({'step':label,'argv':list(map(str,cmd))}))
        else:run(cmd,label,work,env)
    print('Dry-run closure checked; no compilation.' if args.dry_run else 'PASS public regression selection; no release DLL rebuilt or game launched.')
if __name__=='__main__':main()
