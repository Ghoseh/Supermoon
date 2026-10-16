# SPDX-FileCopyrightText: 2026 Supermoon contributors
# SPDX-License-Identifier: MIT
"""Portable tooling; every output uses a fresh disposable directory."""
from pathlib import Path
import hashlib,json,os,shutil,subprocess,tempfile
ROOT=Path(__file__).resolve().parents[1]
def digest(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def recipe():return json.loads((ROOT/'build/recipe.json').read_text(encoding='utf-8'))
def fresh(label):
    parent=Path(tempfile.gettempdir()).resolve(strict=True)
    if parent==ROOT or parent.is_relative_to(ROOT) or ROOT.is_relative_to(parent):
        raise ValueError('Temporary output must be outside and distinct from the source tree')
    for p in (parent,*parent.parents):
        if p.is_symlink() or getattr(p.stat(),'st_file_attributes',0)&0x400:
            raise ValueError('Temporary ancestors may not be reparse points')
        if (p/'NMS.exe').exists() or (p/'supermoon_playlist.dll').exists() or p.name.lower() in {'baselines','review'}:
            raise ValueError('Temporary output may not be in a game/protected tree')
    work=Path(tempfile.mkdtemp(prefix='Supermoon-'+label+'-',dir=parent))
    (work/'OWNED_DISPOSABLE_OUTPUT.txt').write_text('Outputs created by Supermoon public tooling. No historical fixture inputs.\n',encoding='utf-8')
    return work
def environment(work):
    env=os.environ.copy()
    for name in ['TEMP','TMP','APPDATA','LOCALAPPDATA','ZIG_LOCAL_CACHE_DIR','ZIG_GLOBAL_CACHE_DIR']:
        path=work/('isolated-'+name.lower());path.mkdir();env[name]=str(path)
    env.pop('ZIG_LIB_DIR',None)
    return env
def compiler(value):
    path=Path(value).resolve(strict=True)
    if digest(path)!=recipe()['compiler']['zig_exe_sha256']:
        raise ValueError('Compiler identity mismatch: use the pinned Windows x64 Zig 0.15.2 archive')
    return path
def run(args,label,work,env,timeout=300):
    args=list(map(str,args))
    result=subprocess.run(args,cwd=work,env=env,capture_output=True,text=True,timeout=timeout,
                          creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
    record={'argv':args,'cwd':str(work),'returncode':result.returncode,'stdout':result.stdout,'stderr':result.stderr}
    with (work/(label+'.json')).open('x',encoding='utf-8') as f:json.dump(record,f,indent=2)
    if result.returncode:raise RuntimeError(label+' failed; private output receipt: '+str(work/(label+'.json')))
    print(label+': PASS',flush=True)
    return result
def flags(source):
    return ['-std=c++17','-fms-extensions','-DADAPTER_RECOVERY_BUILD','-O2','-g0','-Wall','-Wextra','-Werror',
            '-I'+str(source),'-I'+str(source/'vendor/libebur128/ebur128'),'-ffile-prefix-map='+source.as_posix()+'/=./']
