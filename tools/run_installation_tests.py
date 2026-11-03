# SPDX-License-Identifier: MIT
"""Run owned installation/recovery fixtures outside the source/game trees."""
import argparse,json,sys
from common import ROOT,compiler,environment,flags,fresh,run


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--zig',required=True);p.add_argument('--ubsan',action='store_true')
    a=p.parse_args();cc=compiler(a.zig);work=fresh('installation-tests');env=environment(work)
    print('Disposable test output: '+str(work),flush=True)
    exe=work/'owned-process-fixture.exe'
    options=flags(ROOT/'source')+(['-fsanitize=undefined','-fno-sanitize-recover=all'] if a.ubsan else [])
    run([cc,'c++',*options,ROOT/'tests/installation_process_fixture.cpp','-o',exe],'compile_owned_process_fixture',work,env)
    env['SUPERMOON_INSTALL_TEST_ROOT']=str(work);env['SUPERMOON_INSTALL_PROCESS_FIXTURE']=str(exe)
    env['PYTHONDONTWRITEBYTECODE']='1'
    run([sys.executable,'-B',ROOT/'tests/installation_tests.py'],'installation_tests',work,env,timeout=180)
    print(json.dumps({'result':'PASS','ubsan_process_fixture':a.ubsan,'scope':'Python setup/recovery tests; only owned process helper uses UBSan. No game code runs.','output':str(work)}))


if __name__=='__main__':main()
