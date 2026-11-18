# SPDX-License-Identifier: MIT
"""Exact extracted-package tests in owned fixtures. No game or DLL execution.

Python is the development harness only; child setup PATH excludes Python.
"""
import argparse
import hashlib
import json
import os
import shutil
import subprocess
from pathlib import Path

PATHS = {
    'proxy': 'Binaries/winmm.dll',
    'payload': 'Binaries/Supermoon/supermoon_playlist.dll',
    'bank': 'GAMEDATA/MODS/Supermoon_MenuAudio_Test/audio/windows/init.bnk',
    'marker': 'Binaries/bytebeat-playlist-experimental.enabled',
}

def identity(path):
    raw = path.read_bytes()
    return {'bytes': len(raw), 'sha256': hashlib.sha256(raw).hexdigest()}

def tree(root):
    return {p.relative_to(root).as_posix(): identity(p) for p in root.rglob('*') if p.is_file()}

def quote(value):
    return "'" + str(value).replace("'", "''") + "'"

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ['package', 'inputs', 'work']:
        parser.add_argument('--' + name, type=Path, required=True)
    args = parser.parse_args()
    stage = args.package.resolve(strict=True)
    inputs = args.inputs.resolve(strict=True)
    work = args.work.resolve()
    assert not work.exists()
    assert not work.is_relative_to(stage) and not work.is_relative_to(inputs)
    for parent in [work, *work.parents]:
        assert not (parent/'Binaries/NMS.exe').exists(), 'Test work must not be inside a game'
    work.mkdir()
    (work/'OWNED_OFFLINE_FIXTURES.txt').write_text('Disposable copies; never executes game/DLLs.\n')
    logs = work/'logs'
    logs.mkdir()
    ps = Path(os.environ['SystemRoot'])/'System32/WindowsPowerShell/v1.0/powershell.exe'
    env = os.environ.copy()
    env['PATH'] = str(ps.parent)+';'+str(Path(os.environ['SystemRoot'])/'System32')+';'+os.environ['SystemRoot']
    for name in ['PYTHONPATH', 'PYTHONHOME', 'VIRTUAL_ENV', 'CONDA_PREFIX']:
        env.pop(name, None)
    for name in ['TEMP', 'TMP', 'APPDATA', 'LOCALAPPDATA']:
        p = work/('isolated-'+name)
        p.mkdir()
        env[name] = str(p)
    game = work/"game with spaces & apostrophe ' [1] ! % ^ (test)"
    game.mkdir()
    for rel in ['Binaries/NMS.exe', 'GAMEDATA/PCBANKS/NMSARC.audioBNK.pak']:
        dst = game/rel
        dst.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(inputs/rel, dst)
    sentinels = {
        'Binaries/Supermoon/Music/keep.wav': b'PERSONAL MUSIC',
        'Binaries/Supermoon/Cache/keep': b'PERSONAL CACHE',
        'Binaries/Supermoon/State/keep': b'PERSONAL STATE',
        'Binaries/Supermoon/diagnostics.log': b'PERSONAL LOG',
        'GAMEDATA/MODS/OtherMod/keep.txt': b'OTHER MOD',
    }
    def put(rel, raw):
        p = game/rel
        assert p.resolve().is_relative_to(work)
        p.parent.mkdir(parents=True, exist_ok=True)
        p.write_bytes(raw)
        return p
    for rel, raw in sentinels.items():
        put(rel, raw)
    stage_before, inputs_before = tree(stage), tree(inputs)
    rows, calls = [], 0
    def save():
        (work/'RESULTS.json').write_text(json.dumps({'complete': False, 'cases': rows}, indent=2)+'\n')
    def passed(name):
        rows.append({'case': name, 'result': 'PASS'})
        save()
        print('PASS', name, flush=True)
    def run(command, expected=0):
        nonlocal calls
        calls += 1
        result = subprocess.run([str(x) for x in command], capture_output=True, encoding='utf-8',
                                errors='replace', timeout=60, env=env, cwd=work)
        (logs/f'{calls:03}.json').write_text(json.dumps({'args': [str(x) for x in command],
            'exit': result.returncode, 'stdout': result.stdout, 'stderr': result.stderr}, indent=2)+'\n')
        assert result.returncode == expected, (result.stdout, result.stderr)
        return result
    script = stage/'Supermoon/tools/ManualSetup.ps1'
    def manual(action, output, expected=0, game_path=game, package=stage):
        command = [ps, '-NoProfile', '-NonInteractive', '-ExecutionPolicy', 'RemoteSigned', '-File', script,
                   '-Action', action, '-Game', game_path, '-Output', output, '-Package', package]
        return json.loads(run(command, expected).stdout.lstrip('\ufeff'))
    def automatic(action, expected=0):
        command = [ps, '-NoProfile', '-NonInteractive', '-ExecutionPolicy', 'RemoteSigned', '-File',
                   stage/'Supermoon/tools/Setup.ps1', '-Action', action, '-Game', game, '-Package', stage]
        if action not in ['status', 'generate']:
            command += ['-ConfirmAction', action.upper()]
        return json.loads(run(command, expected).stdout.lstrip('\ufeff'))
    def refused_prepare(label, mutate, undo):
        mutate()
        before = tree(game)
        output = work/('refuse-'+label)
        assert manual('prepare', output, 1)['result'] == 'MANUAL_REFUSED'
        assert not output.exists() and tree(game) == before
        undo()
        passed(label+' refused without game writes')
    def manual_remove(prepared):
        assert manual('removal-check', prepared)['result'] == 'MANUAL_REMOVAL_CHECK_PASSED'
        # Emulate the user's documented removals only in this owned fixture.
        for key in ['marker', 'proxy', 'payload', 'bank']:
            p = (game/PATHS[key]).resolve()
            assert p.is_relative_to(work)
            if p.exists():
                assert identity(p) == identity(prepared/'COPY_TO_GAME'/PATHS[key])
                p.unlink()
        assert manual('removal-check', prepared)['present'] == 0
    proof = run([ps, '-NoProfile', '-Command',
        "$ErrorActionPreference='Stop';foreach($n in @('python','python3','py','zig','clang','gcc','ffmpeg')){if(Get-Command $n -ErrorAction SilentlyContinue){throw ('Unexpected tool '+$n)}};'PYTHON_UNAVAILABLE'"])
    assert 'PYTHON_UNAVAILABLE' in proof.stdout
    passed('Python and external build/audio tools unavailable to setup children')
    baseline = tree(game)
    prepared = work/'prepared initial'
    assert manual('prepare', prepared)['result'] == 'MANUAL_FILES_PREPARED'
    assert tree(game) == baseline
    assert not any(p.name.startswith('.Supermoon-Recovery') for p in game.iterdir())
    assert identity(prepared/'COPY_TO_GAME'/PATHS['bank']) == {
        'bytes': 36017, 'sha256': '65bcbdd3cd9aaae513d21865e504f194afba584ee012f9d3d558e9c04f71a54f'}
    assert (prepared/'COPY_TO_GAME/Binaries/Supermoon/playlist.ini').read_bytes() == (stage/'Supermoon/playlist.ini').read_bytes()
    assert {p.name for p in (prepared/'licenses').iterdir()} == {p.name for p in (stage/'Supermoon/licenses').iterdir()}
    passed('fresh preparation exact DLL/bank/INI/licenses; no game changes or recovery records')
    assert manual('verify', prepared)['result'] == 'VERIFIED_INACTIVE'
    passed('inactive verification distinguished from installed readiness')
    for rel in PATHS.values():
        put(rel, (prepared/'COPY_TO_GAME'/rel).read_bytes())
    manual('verify', prepared, 1)  # Configuration is part of a complete fresh copy.
    default_ini = 'Binaries/Supermoon/playlist.ini'
    put(default_ini, (prepared/'COPY_TO_GAME'/default_ini).read_bytes())
    assert manual('verify', prepared)['result'] == 'VERIFIED_MANUAL_INSTALL'
    manual_remove(prepared)
    assert identity(game/default_ini) == identity(stage/'Supermoon/playlist.ini')
    (game/default_ini).unlink()  # Test-only removal of our exact default, not user data.
    assert tree(game) == baseline
    passed('fresh manual copy requires full default INI; checked rollback preserves it')
    existing_before = tree(prepared)
    manual('prepare', prepared, 1)
    assert tree(prepared) == existing_before and tree(game) == baseline
    passed('existing output never overwritten')
    for output in [game/'output', stage/'output', Path(env['TEMP'])/'output', Path(env['TMP'])/'output', work/'missing-parent'/'output', game]:
        manual('prepare', output, 1)
        assert tree(game) == baseline
    passed('nested game/package TEMP nonexistent-parent and game-root outputs rejected')
    # Same-length executable mutation and archive mutation, restored only in fixture.
    for rel, label in [('Binaries/NMS.exe', 'unsupported executable'),
                       ('GAMEDATA/PCBANKS/NMSARC.audioBNK.pak', 'wrong native archive')]:
        p = game/rel
        original = p.read_bytes()
        changed = bytearray(original)
        changed[-1] ^= 1
        refused_prepare(label, lambda p=p, b=changed: p.write_bytes(b), lambda p=p, b=original: p.write_bytes(b))
    for key, rel in PATHS.items():
        refused_prepare('existing '+key, lambda rel=rel: put(rel, b'UNKNOWN FILE'), lambda rel=rel: (game/rel).unlink())
    conflict = 'GAMEDATA/MODS/ConflictingMod/audio/windows/init.bnk'
    refused_prepare('conflicting loose bank', lambda: put(conflict, b'CONFLICT'), lambda: (game/conflict).unlink())
    for name in ['.Supermoon-Recovery', '.Supermoon-Recovery-1.2.1']:
        refused_prepare(name+' records', lambda name=name: (game/name).mkdir(), lambda name=name: (game/name).rmdir())
    linked = work/'hardlink owner.bin'
    linked.write_bytes(b'LINK OWNER')
    refused_prepare('hard-linked destination', lambda: os.link(linked, game/PATHS['proxy']),
                    lambda: (game/PATHS['proxy']).unlink())
    assert linked.read_bytes() == b'LINK OWNER'
    # A user-edited recipe must not authorize a different executable or DLL pair.
    changed_package = work/'changed-policy-package'
    shutil.copytree(stage, changed_package)
    policy_path = changed_package/'Supermoon/build/recipe.json'
    policy = json.loads(policy_path.read_text())
    policy['DLLs']['winmm.dll']['sha256'] = '0'*64
    policy_path.write_text(json.dumps(policy))
    manual('prepare', work/'changed-policy-output', 1, package=changed_package)
    assert not (work/'changed-policy-output').exists() and tree(game) == baseline
    passed('modified identity policy rejected rather than accepting a different DLL contract')
    ini = put('Binaries/Supermoon/playlist.ini', b'; PERSONAL\r\n[Audio]\r\nVoiceGainDb=4.5\r\n[Custom]\r\nKeep=yes\r\n')
    before = tree(game)
    preserved = work/'prepared preserving INI'
    result = manual('prepare', preserved)
    assert result['configuration'] == 'preserve-existing-do-not-copy-default'
    assert not (preserved/'COPY_TO_GAME/Binaries/Supermoon/playlist.ini').exists()
    assert tree(game) == before
    passed('existing personal configuration omitted from copy tree and preserved byte for byte')
    # Partial copy, marker last, safe identity checks, then complete manual install.
    for key in ['payload', 'bank', 'proxy']:
        put(PATHS[key], (preserved/'COPY_TO_GAME'/PATHS[key]).read_bytes())
    assert manual('verify', preserved, 1)['result'] == 'MANUAL_REFUSED'
    assert manual('removal-check', preserved)['present'] == 3
    passed('interrupted copy rejects launch readiness but permits checked removal')
    put(PATHS['marker'], (preserved/'COPY_TO_GAME'/PATHS['marker']).read_bytes())
    assert manual('verify', preserved)['result'] == 'VERIFIED_MANUAL_INSTALL'
    assert tree(game) == {**before, **{rel: identity(preserved/'COPY_TO_GAME'/rel) for rel in PATHS.values()}}
    passed('documented manual copy verified with existing INI and no data loss')
    assert manual('prepare', work/'refuse-active-manual', 1)['result'] == 'MANUAL_REFUSED'
    passed('re-preparation over active manual installation refused')
    for key in PATHS:
        p = game/PATHS[key]
        original = p.read_bytes()
        p.write_bytes(b'FOREIGN')
        unchanged = tree(game)
        manual('verify', preserved, 1)
        manual('removal-check', preserved, 1)
        assert tree(game) == unchanged
        p.write_bytes(original)
    passed('every unknown activation file refused by verification and removal checks')
    exe = game/'Binaries/NMS.exe'
    original_exe = exe.read_bytes()
    exe.write_bytes(b'UPDATED EXECUTABLE')
    manual('verify', preserved, 1)
    assert manual('removal-check', preserved)['present'] == 4
    exe.write_bytes(original_exe)
    passed('unsupported executable blocks readiness but recognized removal stays available')
    bad_bank = preserved/'COPY_TO_GAME'/PATHS['bank']
    bank_raw = bad_bank.read_bytes()
    bad_bank.write_bytes(b'CORRUPT PREPARED BANK')
    manual('verify', preserved, 1)
    bad_bank.write_bytes(bank_raw)
    extra = preserved/'COPY_TO_GAME/Binaries/unrelated.dll'
    extra.write_bytes(b'EXTRA')
    manual('verify', preserved, 1)
    extra.unlink()
    passed('prepared file corruption and unexpected copy file rejected')
    # Updating a manifest cannot authorize an additional game file.
    manifest_path = preserved/'CONTENTS.json'
    manifest_raw = manifest_path.read_bytes()
    extra.write_bytes(b'EXTRA')
    m = json.loads(manifest_raw)
    m['files']['COPY_TO_GAME/Binaries/unrelated.dll'] = identity(extra)
    manifest_path.write_text(json.dumps(m))
    manual('verify', preserved, 1)
    extra.unlink()
    manifest_path.write_bytes(manifest_raw)
    passed('manifest cannot authorize unrelated files inside copy subtree')
    manual_remove(preserved)
    assert tree(game) == before
    passed('manual rollback to absence preserves INI Music Cache State logs and other mods')
    # Existing automatic route, including transition from safely removed manual files.
    assert automatic('install')['result'] == 'verified'
    assert automatic('status')['result'] == 'candidate_present'
    config = (game/'Binaries/Supermoon/playlist.ini').read_bytes()
    assert b'VoiceGainDb=4.5' in config and b'Keep=yes' in config and b'Previous=Ctrl+Shift+F6' in config
    for rel, raw in sentinels.items():
        assert (game/rel).read_bytes() == raw
    assert automatic('update')['result'] == 'verified'
    automatic('disable')
    assert automatic('status')['result'] == 'prior_present'
    assert all(not (game/rel).exists() for rel in PATHS.values())
    manual('prepare', work/'refuse-disabled-auto-records', 1)
    passed('automatic install update status disable and data/config completion retained; manual takeover refused')
    # Failure schedules operate on a separate clean fixture without recovery records.
    fault_game = work/'fault game'
    fault_game.mkdir()
    for rel in ['Binaries/NMS.exe', 'GAMEDATA/PCBANKS/NMSARC.audioBNK.pak']:
        p = fault_game/rel
        p.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(inputs/rel, p)
    fault_before = tree(fault_game)
    module = stage/'Supermoon/tools/setup/Supermoon.Manual.psm1'
    output = work/'closed-game-required'
    command = ("$ErrorActionPreference='Stop';$m=Import-Module "+quote(module)+" -Force -PassThru;"
        "& $m {& $script:Core {function script:Assert-SMClosed{throw 'OWNED PROCESS SNAPSHOT REFUSAL'}}};"
        "try{Invoke-SMManualOperation -Action prepare -Game "+quote(fault_game)+" -Package "+quote(stage)+" -Output "+quote(output)+";throw 'Expected process refusal'}catch{if($_.Exception.Message -notlike '*OWNED PROCESS SNAPSHOT REFUSAL*'){throw};'EXPECTED_REFUSAL'}")
    assert 'EXPECTED_REFUSAL' in run([ps, '-NoProfile', '-Command', command]).stdout
    assert not output.exists() and tree(fault_game) == fault_before
    passed('indeterminate/running-game guard fails closed before export (injected existing guard)')
    for i, point in enumerate(['exported:proxy', 'exported:bank', 'before-publish']):
        output = work/f'fault-output-{i}'
        command = ("$ErrorActionPreference='Stop';$m=Import-Module "+quote(module)+" -Force -PassThru;"
            "& $m {param($point) $script:Fault=$point;function script:Invoke-SMManualCheckpoint($Stage){if($Stage -ceq $script:Fault){throw 'OWNED EXPORT FAULT'}}} "+quote(point)+";"
            "try{Invoke-SMManualOperation -Action prepare -Game "+quote(fault_game)+" -Package "+quote(stage)+" -Output "+quote(output)+";throw 'Fault not hit'}catch{if($_.Exception.Message -notlike '*OWNED EXPORT FAULT*'){throw};'EXPECTED_FAULT'}")
        assert 'EXPECTED_FAULT' in run([ps, '-NoProfile', '-Command', command]).stdout
        assert not output.exists() and tree(fault_game) == fault_before
    passed('interrupted export never publishes ready output and never changes game')
    # A late personal-INI change aborts publication; the newly written user file survives.
    output = work/'late-change-output'
    late_ini = fault_game/'Binaries/Supermoon/playlist.ini'
    command = ("$ErrorActionPreference='Stop';$m=Import-Module "+quote(module)+" -Force -PassThru;"
        "& $m {param($p) $script:LateIni=$p;function script:Invoke-SMManualCheckpoint($Stage){if($Stage -ceq 'before-publish'){[void][IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($script:LateIni));[IO.File]::WriteAllText($script:LateIni,'USER CONCURRENT EDIT')}}} "+quote(late_ini)+";"
        "try{Invoke-SMManualOperation -Action prepare -Game "+quote(fault_game)+" -Package "+quote(stage)+" -Output "+quote(output)+";throw 'Expected late refusal'}catch{if($_.Exception.Message -notlike '*INI changed during preparation*'){throw};'EXPECTED_REFUSAL'}")
    assert 'EXPECTED_REFUSAL' in run([ps, '-NoProfile', '-Command', command]).stdout
    assert not output.exists() and late_ini.read_text() == 'USER CONCURRENT EDIT'
    passed('late personal INI change aborts publication without overwriting edit')
    interactive_output = work/'interactive-prepared'
    calls += 1
    interactive = subprocess.run([str(ps), '-NoProfile', '-ExecutionPolicy', 'RemoteSigned',
        '-File', str(script), '-Interactive'], input='1\n"'+str(fault_game)+'"\n"'+str(interactive_output)+'"\n',
        capture_output=True, encoding='utf-8', errors='replace', timeout=60, env=env, cwd=work)
    (logs/f'{calls:03}-interactive.json').write_text(json.dumps({'exit': interactive.returncode,
        'stdout': interactive.stdout, 'stderr': interactive.stderr}, indent=2)+'\n')
    assert interactive.returncode == 0 and 'MANUAL_FILES_PREPARED' in interactive.stdout, (interactive.stdout, interactive.stderr)
    assert interactive_output.is_dir() and late_ini.read_text() == 'USER CONCURRENT EDIT'
    passed('interactive preparation menu accepts quoted paths and preserves personal INI')
    assert tree(stage) == stage_before and tree(inputs) == inputs_before
    passed('actual extracted package and historical game inputs unchanged')
    report = {'complete': True, 'result': 'PASS', 'pass_count': len(rows), 'cases': rows,
              'setup_child_calls': calls, 'python_unavailable': True, 'game_or_DLL_executed': False,
              'real_installation_modified': False, 'tested_package_files': stage_before}
    (work/'RESULTS.json').write_text(json.dumps(report, indent=2)+'\n')
    print('COMPLETE', len(rows), 'PASS', flush=True)

if __name__ == '__main__':
    main()
