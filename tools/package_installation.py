# SPDX-License-Identifier: MIT
"""Assemble a reproducible installation ZIP from an exact pinned DLL pair; never install/publish.

The archive has an example INI only. The supported installation path is the
four-file installer, not extraction onto an existing game directory.
"""
from pathlib import Path
import argparse,json,zipfile
from install_supermoon import ROOT,MARKER,identity,load_json,read_file
from installation_safety import no_reparse


def members(dll_dir,recipe):
    dll_dir=Path(dll_dir);no_reparse(dll_dir)
    files={}
    for name,rel in [('winmm.dll','winmm.dll'),('supermoon_playlist.dll','Supermoon/supermoon_playlist.dll')]:
        raw=read_file(dll_dir/name)
        if raw is None or identity(raw)!=recipe['DLLs'][name]:raise ValueError('Unqualified DLL input: '+name)
        files[rel]=raw
    files['bytebeat-playlist-experimental.enabled']=MARKER
    # Ordinary installation/recovery uses only Windows PowerShell 5.1. Python
    # development/oracle and version-specific legacy tools stay in source.
    for name in ['Setup.cmd','SETUP_README.txt','SETUP_SECURITY.txt']:
        files[name]=(ROOT/name).read_bytes()
    for name in ['Setup.ps1','setup/Supermoon.Setup.psm1','setup/Native.cs','audio_patch.json']:
        files['Supermoon/tools/'+name]=(ROOT/'tools'/name).read_bytes()
    files['Supermoon/build/recipe.json']=(json.dumps(recipe,indent=2)+'\n').encode()
    files['Supermoon/playlist.example.ini']=(ROOT/'source/playlist/playlist.example.ini').read_bytes()
    for name in ['INSTALL.md','README.md','CHANGELOG.md','COPYING','LICENSING.md','ASSET_NOTICE.md','THIRD_PARTY_NOTICES.md']:
        files['Supermoon/'+name]=(ROOT/name).read_bytes()
    # Include the documentation linked by README/INSTALL in the end-user package.
    # Current user guides only. Historical release/qualification prose stays in
    # source rather than being presented as this rebuilt pair's qualification.
    for name in ['BUILD_AND_PROVENANCE.md','DEVELOPER_GUIDE.md','QUALIFICATION.md','VERSION_TRANSITION.md','CONFIGURATION_AND_LAYOUT.md',
                 'DEPENDENCIES.md','DEPENDENCIES.json','SUPERMOON_V1_KNOWN_LIMITATIONS.md',
                 'SUPPORTED_FORMATS.md','TROUBLESHOOTING.md','RELEASE_NOTES.md',
                 'NMS26540391_MIGRATION.md','CONTROLLED_LIVE_TEST.md']:
        files['Supermoon/docs/'+name]=(ROOT/'docs'/name).read_bytes()
    files['Supermoon/tests/README.md']=(ROOT/'tests/README.md').read_bytes()
    files['Supermoon/LICENSE']=(ROOT/'LICENSE').read_bytes()
    for name in ['Zig-MIT.txt','toolchain-only-winpthreads-MIT.txt','musl-derived-math-per-file-NOTICES.txt',
                 'musl-COPYRIGHT.txt','MinGW-COPYING-ZPL-2.1.txt','Lucent-gdtoa-per-file-NOTICES.txt',
                 'LLVM-libunwind-LICENSE.txt','LLVM-libcxxabi-LICENSE.txt','LLVM-libcxx-LICENSE.txt',
                 'libebur128-COPYING.txt','dr_mp3-repository-LICENSE.txt','dr_mp3-header-and-minimp3-notices.txt',
                 'Berkeley-queue-BSD-3-Clause.txt']:
        files['Supermoon/licenses/'+name]=(ROOT/'licenses'/name).read_bytes()
    assert not any(Path(n).name.casefold()=='playlist.ini' for n in files)
    manifest=(json.dumps({'purpose':'Supermoon '+recipe['version']+' Windows x64 Python-free installation',
                         'qualification':recipe.get('qualification','Unspecified; no live qualification implied'),
                         'native_checkpoint':'5d4671694eb0f020f10a63b2a600dbc820f05534',
                         'end_user_setup':'Windows PowerShell Desktop 5.1 via Setup.cmd; no Python or external build tools',
                         'directories':['Supermoon/Music/'],
                         'files':{n:identity(b) for n,b in sorted(files.items())}},indent=2)+'\n').encode()
    files['PUBLIC_FILE_MANIFEST.json']=manifest
    files['SHA256SUMS']=(''.join(identity(b)['sha256']+'  '+n+'\n' for n,b in sorted(files.items()) if n!='PUBLIC_FILE_MANIFEST.json')+identity(manifest)['sha256']+'  PUBLIC_FILE_MANIFEST.json\n').encode()
    return files


def package(dll_dir,output,recipe):
    files=members(dll_dir,recipe);output=Path(output);no_reparse(output)
    # Exclusive creation: never rewrite a release/archive supplied by the caller.
    with output.open('xb') as f:
        with zipfile.ZipFile(f,'w',zipfile.ZIP_DEFLATED) as z:
            info=zipfile.ZipInfo('Supermoon/Music/',(2026,1,1,0,0,0))
            info.external_attr=(0o40755<<16)|0x10;z.writestr(info,b'')
            for name,data in sorted(files.items()):
                info=zipfile.ZipInfo(name,(2026,1,1,0,0,0));info.compress_type=zipfile.ZIP_DEFLATED
                info.external_attr=0o100644<<16;z.writestr(info,data)
    with zipfile.ZipFile(output) as z:
        if z.testzip() is not None or {n:z.read(n) for n in z.namelist() if not n.endswith('/')}!=files or {n for n in z.namelist() if n.endswith('/')}!={'Supermoon/Music/'}:raise ValueError('Archive verification failed')
    return {'files':len(files),'output':str(output),'identity':identity(output.read_bytes())}


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--dll-dir',required=True,type=Path);p.add_argument('--output',required=True,type=Path)
    a=p.parse_args();print(json.dumps(package(a.dll_dir,a.output,load_json(ROOT/'build/recipe.json')),indent=2))


if __name__=='__main__':main()
