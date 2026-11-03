# SPDX-License-Identifier: MIT
"""Package the selected, manifest-pinned source closure into a fresh reproducible ZIP.

The manifest is prepared deliberately by the maintainer. This command verifies
existing identities; it does not discover private files or silently refresh pins.
"""
from pathlib import Path,PurePosixPath
import argparse,json,zipfile
from common import ROOT,digest
from installation_safety import no_reparse


def package(output):
    manifest=ROOT/'PUBLIC_FILE_MANIFEST.json'
    rows=json.loads(manifest.read_text(encoding='utf-8'))['files']
    files={};folded=set()
    for name,row in sorted(rows.items()):
        p=PurePosixPath(name)
        if p.is_absolute() or '..' in p.parts or '\\' in name or ':' in name:
            raise ValueError('Unsafe source member')
        if name.casefold() in folded:raise ValueError('Duplicate source member')
        folded.add(name.casefold());path=ROOT/name;no_reparse(path)
        raw=path.read_bytes()
        if len(raw)!=row['bytes'] or digest(path)!=row['sha256']:
            raise ValueError('Source differs from pinned manifest: '+name)
        files[name]=raw
    files['PUBLIC_FILE_MANIFEST.json']=manifest.read_bytes()
    files['SHA256SUMS']=(ROOT/'SHA256SUMS').read_bytes()
    expected=''.join(row['sha256']+'  '+name+'\n' for name,row in sorted(rows.items()))+digest(manifest)+'  PUBLIC_FILE_MANIFEST.json\n'
    if files['SHA256SUMS'].decode('utf-8')!=expected:raise ValueError('Source checksums mismatch')
    output=Path(output);no_reparse(output)
    with output.open('xb') as f:
        with zipfile.ZipFile(f,'w',zipfile.ZIP_DEFLATED) as z:
            for name,raw in sorted(files.items()):
                info=zipfile.ZipInfo(name,(2026,1,1,0,0,0));info.compress_type=zipfile.ZIP_DEFLATED
                info.external_attr=0o100644<<16;z.writestr(info,raw)
    with zipfile.ZipFile(output) as z:
        if z.testzip() is not None or {n:z.read(n) for n in z.namelist()}!=files:
            raise ValueError('Source archive verification failed')
    return {'output':str(output),'files':len(files),'bytes':output.stat().st_size,'sha256':digest(output)}


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',required=True,type=Path)
    print(json.dumps(package(p.parse_args().output),indent=2))


if __name__=='__main__':main()
