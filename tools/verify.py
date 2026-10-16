# SPDX-FileCopyrightText: 2026 Supermoon contributors
# SPDX-License-Identifier: MIT
"""Read-only manifest verification for source or an installation directory/ZIP."""
from pathlib import Path,PurePosixPath
import argparse,hashlib,json,zipfile
def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('target',nargs='?',default=str(Path(__file__).resolve().parents[1]))
    parser.add_argument('--manifest-sha256',help='Optional trusted manifest pin from the release receipt')
    args=parser.parse_args();root=Path(args.target)
    if root.is_file():
        z=zipfile.ZipFile(root);names=z.namelist()
        if len(names)!=len(set(names)):raise ValueError('Duplicate archive members')
        for n in names:
            p=PurePosixPath(n)
            if p.is_absolute() or '..' in p.parts or '\\' in n or ':' in n:raise ValueError('Unsafe member name')
        actual={n for n in names if not n.endswith('/')};read=z.read
    else:
        paths=list(root.rglob('*'))
        if any(p.is_symlink() or getattr(p.lstat(),'st_file_attributes',0)&0x400 for p in paths):raise ValueError('Reparse point in tree')
        actual={p.relative_to(root).as_posix() for p in paths if p.is_file()};read=lambda n:(root/n).read_bytes()
    raw=read('PUBLIC_FILE_MANIFEST.json');pin=hashlib.sha256(raw).hexdigest()
    if args.manifest_sha256 and pin!=args.manifest_sha256:raise ValueError('Manifest pin mismatch')
    rows=json.loads(raw)['files']
    if actual!=set(rows)|{'PUBLIC_FILE_MANIFEST.json','SHA256SUMS'}:raise ValueError('File set differs from manifest')
    for n,e in rows.items():
        p=PurePosixPath(n)
        if p.is_absolute() or '..' in p.parts or '\\' in n or ':' in n:raise ValueError('Unsafe manifest path')
        b=read(n)
        if len(b)!=e['bytes'] or hashlib.sha256(b).hexdigest()!=e['sha256']:raise ValueError('Hash mismatch: '+n)
    expected=''.join(e['sha256']+'  '+n+'\n' for n,e in sorted(rows.items()))+pin+'  PUBLIC_FILE_MANIFEST.json\n'
    if read('SHA256SUMS').decode('utf-8')!=expected:raise ValueError('SHA256SUMS mismatch')
    print(json.dumps({'result':'PASS','files':len(actual),'manifest_sha256':pin},indent=2))
if __name__=='__main__':main()
