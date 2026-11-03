# SPDX-License-Identifier: MIT
"""Run offline remediation assertions; known defects return failure, never XFAIL/pass."""
import argparse
import json
import subprocess
import sys
from common import ROOT, compiler, digest, environment, flags, fresh, run


def execute(argv, cwd, env, timeout):
    argv = list(map(str, argv))
    child = subprocess.Popen(argv, cwd=cwd, env=env, stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE, text=True,
                             creationflags=getattr(subprocess, 'CREATE_NO_WINDOW', 0))
    timed_out = False
    try:
        stdout, stderr = child.communicate(timeout=timeout)
    except subprocess.TimeoutExpired:
        timed_out = True
        child.kill()  # Only the exact owned fixture process; communicate reaps it.
        stdout, stderr = child.communicate()
    return dict(argv=argv, cwd=str(cwd), pid=child.pid, returncode=child.returncode,
                stdout=stdout, stderr=stderr, timed_out=timed_out,
                timeout_seconds=timeout, owned_child_reaped=child.poll() is not None)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--zig', required=True)
    parser.add_argument('--ubsan', action='store_true')
    parser.add_argument('--suite', action='append', choices=[
        'music_control_tests', 'music_control_recovery_tests', 'adapter_release_tests',
        'native_recovery_wrapper_tests', 'dependency_policy_tests',
        'required_audio_policy_tests', 'audio_dependency_preflight_tests'])
    args = parser.parse_args()
    cc = compiler(args.zig)
    work = fresh('remediation-tests')
    print('Disposable test output: '+str(work), flush=True)
    env = environment(work)
    source = ROOT/'source'
    opts = flags(source)+(['-fsanitize=undefined', '-fno-sanitize-recover=all'] if args.ubsan else [])
    meter = work/'ebur128.o'
    stream = [source/'playlist'/n for n in ['stream_engine.cpp', 'notifications.cpp',
              'wav_pipeline.cpp', 'mp3_reader.cpp', 'normalization.cpp']]
    suites = {
        'music_control_tests': [],
        'music_control_recovery_tests': [],
        'adapter_release_tests': ['-DRECOVERY_SELFTEST_BUILD', *stream, meter,
                                  source/'recovery_native.cpp', source/'native_bridges.S'],
        'native_recovery_wrapper_tests': ['-DRECOVERY_SELFTEST_BUILD', source/'recovery_native.cpp'],
        'dependency_policy_tests': [],
        'required_audio_policy_tests': [],
        'audio_dependency_preflight_tests': [],
    }
    if args.suite:
        suites = {n:extra for n,extra in suites.items() if n in args.suite}
    inputs = [p for d in ['source', 'tests'] for p in (ROOT/d).rglob('*') if p.is_file()]
    metadata = dict(ubsan=args.ubsan,compiler=str(cc),compiler_sha256=digest(cc),
                    source_root=str(ROOT),input_sha256={p.relative_to(ROOT).as_posix():digest(p) for p in inputs},
                    scope='Owned synthetic inputs only. SDK stub success models submission, not audibility. '
                          'Adapter uses actual source/Core/Engine/bridges; activation and native game bodies never run. '
                          'C meter and assembly are not UBSan instrumented.')
    (work/'inputs.json').write_text(json.dumps(metadata, indent=2)+'\n', encoding='utf-8')
    if 'adapter_release_tests' in suites:
        vendor = source/'vendor/libebur128/ebur128'
        run([cc,'cc','-std=gnu99','-O2','-g0','-I'+str(vendor),'-I'+str(vendor/'queue'),
             ROOT/'ebur128_peak.c','-c','-o',meter], 'compile_meter',work,env)
    rows = []
    for name, extra in suites.items():
        exe = work/(name+'.exe')
        run([cc,'c++',*opts,'-ffunction-sections','-fdata-sections',ROOT/'tests'/(name+'.cpp'),
             *extra,'-Wl,--gc-sections','-lbcrypt','-luser32','-o',exe], 'compile_'+name,work,env)
        listing = execute([exe,'--list'],work,env,10)
        (work/(name+'-list.json')).write_text(json.dumps(listing, indent=2)+'\n', encoding='utf-8')
        if listing['returncode'] or listing['timed_out']:
            raise RuntimeError('Case listing failed: '+name)
        cases = listing['stdout'].splitlines()
        if not cases or len(cases)!=len(set(cases)):
            raise RuntimeError('Empty/duplicate case list: '+name)
        for case in cases:
            cwd = work/(name+'--'+case)
            cwd.mkdir()
            timeout = 3 if case=='same_invocation_cleanup_liveness' else 15
            result = execute([exe,case],cwd,env,timeout)
            result.update(suite=name,case=case)
            # An H1 timeout is a failed liveness assertion with retained evidence,
            # NOT a passing characterization. No expected-failure whitelist.
            result['status'] = ('TIMEOUT' if result['timed_out'] else
                                'PASS' if result['returncode']==0 else
                                'FAIL' if result['returncode']==1 else 'ERROR')
            receipt = name+'--'+case+'.json'
            (work/receipt).write_text(json.dumps(result,indent=2)+'\n', encoding='utf-8')
            rows.append(dict(suite=name,case=case,status=result['status'],receipt=receipt))
            print(result['status']+' '+name+'/'+case, flush=True)
    counts = {s:sum(r['status']==s for r in rows) for s in ['PASS','FAIL','TIMEOUT','ERROR']}
    summary = dict(result='PASS' if counts['PASS']==len(rows) else 'FAIL',counts=counts,cases=rows,
                   output=str(work),ubsan=args.ubsan)
    (work/'SUMMARY.json').write_text(json.dumps(summary,indent=2)+'\n', encoding='utf-8')
    print(json.dumps({k:v for k,v in summary.items() if k!='cases'}),flush=True)
    return 0 if summary['result']=='PASS' else 1


if __name__=='__main__':
    sys.exit(main())
