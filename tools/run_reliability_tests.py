# SPDX-License-Identifier: MIT
"""Phase 4 deterministic owned fixtures; no native NMS instructions execute."""
import argparse,json,sys
from common import ROOT,compiler,digest,environment,flags,fresh,run
from run_remediation_tests import execute
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--zig',required=True);p.add_argument('--ubsan',action='store_true');a=p.parse_args()
    cc=compiler(a.zig);work=fresh('reliability-tests');env=environment(work);print('Disposable test output: '+str(work),flush=True)
    src=ROOT/'source';options=flags(src)+(['-fsanitize=undefined','-fno-sanitize-recover=all'] if a.ubsan else []);meter=work/'ebur128.o';vendor=src/'vendor/libebur128/ebur128'
    run([cc,'cc','-std=gnu99','-O2','-g0','-I'+str(vendor),'-I'+str(vendor/'queue'),ROOT/'ebur128_peak.c','-c','-o',meter],'compile_meter',work,env)
    wav=[src/'playlist'/n for n in ['wav_pipeline.cpp','mp3_reader.cpp','normalization.cpp']]
    groups={'discovery_reliability_tests':[*wav,meter,src/'playlist/persistent_cache.cpp',src/'playlist/library.cpp'],
      'reliability_recovery_tests':['-DRECOVERY_SELFTEST_BUILD',src/'recovery_native.cpp'],
      'activation_status_tests':['-DRECOVERY_SELFTEST_BUILD',*wav,meter,src/'playlist/stream_engine.cpp',src/'playlist/notifications.cpp',src/'recovery_native.cpp',src/'native_bridges.S']}
    rows=[]
    inputs={p.relative_to(ROOT).as_posix():digest(p) for d in ['source','tests','tools'] for p in (ROOT/d).rglob('*') if p.is_file() and '__pycache__' not in p.parts}
    (work/'inputs.json').write_text(json.dumps(dict(ubsan=a.ubsan,compiler_sha256=digest(cc),inputs=inputs),indent=2)+'\n')
    for name,extra in groups.items():
        exe=work/(name+'.exe');run([cc,'c++',*options,'-ffunction-sections','-fdata-sections',ROOT/'tests'/(name+'.cpp'),*extra,'-Wl,--gc-sections','-lbcrypt','-luser32','-o',exe],'compile_'+name,work,env)
        listing=execute([exe,'--list'],work,env,10);assert listing['returncode']==0
        for case in listing['stdout'].splitlines():
            cwd=work/(name+'--'+case);cwd.mkdir();r=execute([exe,case],cwd,env,20)
            r.update(suite=name,case=case,status='TIMEOUT' if r['timed_out'] else 'PASS' if r['returncode']==0 else 'FAIL')
            (work/(name+'--'+case+'.json')).write_text(json.dumps(r,indent=2)+'\n');rows.append(r);print(r['status']+' '+name+'/'+case,flush=True)
    env['SUPERMOON_RELIABILITY_ROOT']=str(work);run([sys.executable,'-B',ROOT/'tests/tool_output_tests.py'],'tool_output_tests',work,env)
    summary=dict(ubsan=a.ubsan,counts={s:sum(r['status']==s for r in rows) for s in ['PASS','FAIL','TIMEOUT']},python_tests=6,output=str(work),scope='Owned fixtures. Activation-stage injection exercises the real exported reader/formatter; native publication is not executed.')
    (work/'SUMMARY.json').write_text(json.dumps(summary,indent=2)+'\n');print(json.dumps(summary),flush=True)
    return 0 if all(r['status']=='PASS' for r in rows) else 1
if __name__=='__main__':sys.exit(main())
