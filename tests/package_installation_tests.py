# SPDX-License-Identifier: MIT
"""Developer-only packaging regression: test the actual emitted/extracted ZIP.

The supplied DLLs are treated only as bytes. This does not install or load them.
"""
from pathlib import Path, PurePosixPath
import argparse, hashlib, json, re, sys, zipfile
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
import package_installation as packaging
from install_supermoon import ROOT, identity, load_json


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--dll-dir', required=True, type=Path)
    p.add_argument('--work', required=True, type=Path)
    a = p.parse_args()
    a.work.mkdir(exist_ok=False)
    recipe = load_json(ROOT / 'build/recipe.json')
    receipts = []
    for name in ['Supermoon-'+recipe['version']+'-windows-x64.zip', 'reproduction.zip']:
        receipts.append(packaging.package(a.dll_dir, a.work / name, recipe))
    assert receipts[0]['identity'] == receipts[1]['identity'], 'ZIP is not reproducible'
    archive = Path(receipts[0]['output'])
    extracted = a.work / 'extracted'
    with zipfile.ZipFile(archive) as z:
        names = z.namelist()
        assert len(names) == len(set(n.casefold() for n in names))
        assert all(not PurePosixPath(n).is_absolute() and '..' not in PurePosixPath(n).parts
                   and '\\' not in n and ':' not in n for n in names)
        assert z.testzip() is None
        z.extractall(extracted)
    needed = ['Setup.cmd', 'SETUP_README.txt', 'SETUP_SECURITY.txt',
              'winmm.dll', 'bytebeat-playlist-experimental.enabled',
              'Supermoon/supermoon_playlist.dll', 'Supermoon/tools/Setup.ps1',
              'Supermoon/tools/setup/Supermoon.Setup.psm1', 'Supermoon/tools/setup/Native.cs',
              'Supermoon/tools/audio_patch.json', 'Supermoon/build/recipe.json',
              'Supermoon/INSTALL.md', 'Supermoon/THIRD_PARTY_NOTICES.md',
              'Supermoon/ASSET_NOTICE.md', 'Supermoon/docs/NMS26540391_MIGRATION.md',
              'Supermoon/docs/CONTROLLED_LIVE_TEST.md']
    assert all((extracted / n).is_file() for n in needed)
    forbidden = {'.py', '.pyc', '.exe', '.bnk', '.pak', '.wav', '.mp3', '.pdb'}
    assert not any(Path(n).suffix.casefold() in forbidden for n in names)
    assert not any(Path(n).name.casefold() == 'playlist.ini' for n in names)
    assert {n for n in names if n.endswith('/')} == {'Supermoon/Music/'}
    assert (extracted / 'Supermoon/Music').is_dir() and not list((extracted / 'Supermoon/Music').iterdir())
    assert not any(set(PurePosixPath(n).parts) & {'Music', 'Cache', 'State', '.Supermoon-Recovery'} for n in names if not n.endswith('/'))
    assert {n for n in names if n.endswith('.dll')} == {'winmm.dll', 'Supermoon/supermoon_playlist.dll'}
    manifest = json.loads((extracted / 'PUBLIC_FILE_MANIFEST.json').read_text())
    assert set(names) == set(manifest['files']) | {'PUBLIC_FILE_MANIFEST.json', 'SHA256SUMS', 'Supermoon/Music/'}
    assert 'retained Phase 6' not in manifest['purpose']
    for name, expected in manifest['files'].items():
        assert identity((extracted / name).read_bytes()) == expected, name
    sums = (extracted / 'SHA256SUMS').read_text().splitlines()
    assert len(sums) == len(names) - 2
    for line in sums:
        h, name = line.split('  ', 1)
        assert hashlib.sha256((extracted / name).read_bytes()).hexdigest() == h
    for f in (ROOT / 'licenses').iterdir():
        assert (extracted / 'Supermoon/licenses' / f.name).read_bytes() == f.read_bytes()
    assert (extracted / 'Supermoon/LICENSE').read_bytes() == (ROOT / 'LICENSE').read_bytes()
    assert (extracted / 'Supermoon/COPYING').read_bytes() == (ROOT / 'COPYING').read_bytes()
    for rel in ['Setup.cmd', 'SETUP_README.txt', 'SETUP_SECURITY.txt']:
        assert (extracted / rel).read_bytes() == (ROOT / rel).read_bytes()
    for rel in ['tools/Setup.ps1', 'tools/setup/Supermoon.Setup.psm1', 'tools/setup/Native.cs', 'tools/audio_patch.json']:
        assert (extracted / 'Supermoon' / rel).read_bytes() == (ROOT / rel).read_bytes()
    # Every Markdown link in current packaged material must resolve locally or be external.
    for f in extracted.rglob('*.md'):
        for target in re.findall(r'\[[^\]]*\]\(([^)]+)\)', f.read_text(encoding='utf-8')):
            if ':' in target or target.startswith('#'):
                continue
            assert (f.parent / target.split('#')[0]).is_file(), (f, target)
    assert b'Python 3.10 or newer is required' not in (extracted / 'Supermoon/INSTALL.md').read_bytes()
    # Exclusive output creation must not replace an existing archive.
    try:
        packaging.package(a.dll_dir, archive, recipe)
    except FileExistsError:
        pass
    else:
        raise AssertionError('Existing archive overwritten')
    # A same-size DLL mutation must not produce a package.
    bad = a.work / 'bad-dlls'
    bad.mkdir()
    for name in recipe['DLLs']:
        b = bytearray((a.dll_dir / name).read_bytes())
        if name == 'winmm.dll':
            b[-1] ^= 1
        (bad / name).write_bytes(b)
    try:
        packaging.package(bad, a.work / 'must-not-exist.zip', recipe)
    except ValueError:
        pass
    else:
        raise AssertionError('Mutated DLL accepted')
    assert not (a.work / 'must-not-exist.zip').exists()
    result = dict(result='PASS', archive=receipts[0], reproduction=receipts[1],
                  extracted=str(extracted), members=names, manifest=identity((extracted / 'PUBLIC_FILE_MANIFEST.json').read_bytes()),
                  checks=['exact required Python-free members', 'no Python/runtime tools/game assets/personal files',
                          'all license files exact', 'manifest/checksums/paths/case/CRC', 'current Markdown links',
                          'byte-identical repeated ZIP', 'exclusive existing output', 'mutated DLL refusal'],
                  end_user_execution='Separate extracted-ZIP PowerShell/CMD qualification required')
    (a.work / 'PACKAGE_TEST_RESULTS.json').write_text(json.dumps(result, indent=2)+'\n')
    print(json.dumps({k: result[k] for k in ['result', 'archive', 'extracted', 'checks']}, indent=2))


if __name__ == '__main__':
    main()
