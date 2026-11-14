# SPDX-License-Identifier: MIT
"""Owner-supplied exact-game/package bytes, fenced offline upgrade fixtures only.

No game/DLL execution. Requires Windows PowerShell Desktop 5.1. Python is only
the development harness; setup child PATH excludes Python/compilers/converters.
"""
from pathlib import Path
import argparse,hashlib,json,os,shutil,subprocess,zipfile

def main():
 p=argparse.ArgumentParser(description=__doc__)
 for n in ['package','legacy-package','inputs','work']:p.add_argument('--'+n,type=Path,required=True)
 a=p.parse_args();w=a.work.resolve();assert not w.is_relative_to(Path(__file__).resolve().parents[1])
 for ancestor in [w,*w.parents]:assert not (ancestor/'Binaries/NMS.exe').exists(),'Test work must be outside a game installation'
 w.mkdir(exist_ok=False);logs=w/'logs';logs.mkdir()
 stage=a.package.resolve(strict=True);legacy=w/'legacy';legacy.mkdir()
 with zipfile.ZipFile(a.legacy_package) as z:
  assert not any(Path(n).is_absolute() or '..' in Path(n).parts for n in z.namelist());z.extractall(legacy)
 recipe=json.loads((stage/'Supermoon/build/recipe.json').read_text());assert recipe['version']=='1.2.1'
 ps=Path(os.environ['SystemRoot'])/'System32/WindowsPowerShell/v1.0/powershell.exe'
 env=os.environ.copy();env['PATH']=str(ps.parent)+';'+str(Path(os.environ['SystemRoot'])/'System32')+';'+os.environ['SystemRoot']
 for n in ['PYTHONPATH','PYTHONHOME','VIRTUAL_ENV','CONDA_PREFIX']:env.pop(n,None)
 for n in ['TEMP','TMP','APPDATA','LOCALAPPDATA']:
  d=w/('isolated-'+n);d.mkdir();env[n]=str(d)
 paths={'proxy':'Binaries/winmm.dll','payload':'Binaries/Supermoon/supermoon_playlist.dll','bank':'GAMEDATA/MODS/Supermoon_MenuAudio_Test/audio/windows/init.bnk','marker':'Binaries/bytebeat-playlist-experimental.enabled'}
 rows=[];calls=0
 def fileid(p):return {'bytes':p.stat().st_size,'sha256':hashlib.sha256(p.read_bytes()).hexdigest()}
 def inventory(root):return {p.relative_to(root).as_posix():fileid(p) for p in root.rglob('*') if p.is_file()}
 def save(p,v):p.write_text(json.dumps(v,indent=2)+'\n')
 def q(s):return "'"+str(s).replace("'","''")+"'"
 def run(label,args,code=0):
  nonlocal calls;calls+=1
  r=subprocess.run(list(map(str,args)),cwd=w,env=env,capture_output=True,text=True,encoding='utf-8',errors='replace',timeout=90,creationflags=subprocess.CREATE_NO_WINDOW)
  save(logs/f'{calls:03}-{label}.json',{'exit':r.returncode,'stdout':r.stdout,'stderr':r.stderr})
  assert r.returncode==code,(label,r.stdout,r.stderr)
  return r
 def cli(g,action,code=0,old=False,package=None):
  pkg=legacy if old else package or stage
  args=[ps,'-NoProfile','-ExecutionPolicy','RemoteSigned','-File',pkg/'Supermoon/tools/Setup.ps1','-Action',action,'-Game',g,'-Package',pkg]
  if action not in ['status','generate']:args+=['-ConfirmAction',action.upper()]
  return json.loads(run(action,args,code).stdout)
 def passed(name):
  rows.append({'case':name,'result':'PASS'});save(w/'RESULTS.json',{'complete':False,'cases':rows});print('PASS',len(rows),name,flush=True)
 def create(name,old=True):
  g=w/'fixtures'/name;g.mkdir(parents=True)
  for rel in ['Binaries/NMS.exe','GAMEDATA/PCBANKS/NMSARC.audioBNK.pak']:
   d=g/rel;d.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(a.inputs/rel,d)
  for rel,data in {
    'Binaries/Supermoon/playlist.ini':b'[Playlist]\r\nMusic=Music\r\nCacheMiB=2048\r\n[Audio]\r\nVoiceGainDb=0.0\r\n[Keyboard]\r\nNext=Ctrl+Alt+F7\r\n',
    'Binaries/Supermoon/Music/private.wav':b'OWNED MUSIC NEVER PLAY',
    'Binaries/Supermoon/Cache/private.pcm':b'PRESERVE CACHE',
    'Binaries/Supermoon/State/private':b'PRESERVE STATE',
    'Binaries/Supermoon/preparation-latest.log':b'PRESERVE LOG',
    'Binaries/SETTINGS/GCMODSETTINGS.MXML':b'PRESERVE SETTINGS',
    'save-data/user.hg':b'PRESERVE SAVE',
    'GAMEDATA/MODS/Other/preserve':b'PRESERVE OTHER MOD',
    'external-library/private.mp3':b'PRESERVE EXTERNAL MUSIC'}.items():
   d=g/rel;d.parent.mkdir(parents=True,exist_ok=True);d.write_bytes(data)
  personal=inventory(g)
  if old:assert cli(g,'install',old=True)['result']=='verified'
  return g,personal
 def preserved(g,before):
  for n,expected in before.items():
   if n=='Binaries/Supermoon/playlist.ini' and fileid(g/n)!=expected:
    backups=[p for p in (g/'.Supermoon-Recovery-1.2.1/configuration').glob('*.ini') if fileid(p)==expected]
    assert backups,'Original INI backup missing'
    original=backups[0].read_bytes().splitlines();updated=(g/n).read_bytes().splitlines()
    assert all(line in updated for line in original),'An original INI line changed'
    assert b'VoiceGainDb=0.0' in updated and b'Next=Ctrl+Alt+F7' in updated
   else:assert fileid(g/n)==expected,n
 def absent(g):assert not any((g/n).exists() for n in paths.values())
 def verified(g,before):
  preserved(g,before);s=cli(g,'status');assert s['result']=='candidate_present' and s['game_supported'] and s['journal']['phase']=='verified'
  root=g/'.Supermoon-Recovery-1.2.1';assert Path(s['recovery_root'])==root
  m=json.loads((root/'records'/s['record']/'manifest.json').read_text());assert m['release']=='1.2.1' and all(x is None for x in m['before'].values())
  index=json.loads((root/'ARCHIVE-1.2.0.json').read_text());j=json.loads((root/'UPGRADE-1.2.0.json').read_text());assert j['phase']=='complete'
  archived=root/'archives'/('v1.2.0-'+j['old_record']);assert inventory(g/'.Supermoon-Recovery')==index and inventory(archived)==index
  assert fileid(g/paths['proxy'])==recipe['DLLs']['winmm.dll'] and fileid(g/paths['payload'])==recipe['DLLs']['supermoon_playlist.dll']
  return root,j
 def inject(g,point,old_disable=False,action='upgrade'):
  # Owned process-only function seam, unreachable via CLI/environment.
  special=''
  if old_disable:
   special="if($Stage -ceq 'upgrade:legacy_loaded'){& (Get-Module Supermoon120) {function script:Invoke-SMCheckpoint($s){if($s -ceq 'after_remove:proxy'){throw 'OWNED OLD DISABLE FAILURE'}}}};"
  script=f"""$ErrorActionPreference='Stop'
Import-Module {q(stage/'Supermoon/tools/setup/Supermoon.Setup.psm1')} -Force
try{{& (Get-Module Supermoon.Setup) {{param($game,$stage)
function script:Invoke-SMCheckpoint($Stage){{{special}if($Stage -ceq {q(point)}){{throw 'OWNED UPGRADE INTERRUPTION'}}}}
$c=New-SMContext $game (Read-SMJson (Join-Path $stage 'Supermoon/build/recipe.json')) (Read-SMJson (Join-Path $stage 'Supermoon/tools/audio_patch.json'))
Invoke-SMOperation $c {action} $stage | Out-Null
}} {q(g)} {q(stage)};throw 'Seam not reached'}}catch{{if($_.Exception.Message -notmatch 'OWNED'){{throw}};Write-Output $_.Exception.Message}}
"""
  run('injected', [ps,'-NoProfile','-Command',script])
 # Confirm installer child tool unavailability.
 run('no-python',[ps,'-NoProfile','-Command',"foreach($n in @('python','python3','py','zig','clang','ffmpeg')){if(Get-Command $n -ErrorAction SilentlyContinue){throw $n}};if($PSVersionTable.PSEdition -ne 'Desktop'){throw 'wrong PowerShell'}"]);passed('Python unavailable to setup children')
 g,b=create('complete');old_before=inventory(g/'.Supermoon-Recovery');cli(g,'upgrade');root,j=verified(g,b)
 # Immutable old manifests/backups stay exact; only matching Disable journal changes.
 for n,v in old_before.items():
  if not n.endswith('/journal.json'):assert fileid(g/'.Supermoon-Recovery'/n)==v
 passed('exact published v1.2.0 upgrade, independent records, verified archive and data preservation')
 cli(g,'upgrade');verified(g,b);passed('repeat completed Upgrade is idempotent')
 cli(g,'rollback');absent(g);preserved(g,b);passed('upgraded installation rollback to absence without old DLL restoration')
 g,b=create('already-disabled');cli(g,'disable',old=True);cli(g,'upgrade');verified(g,b);passed('matching already-disabled public installation upgrades')
 for point in ['upgrade:prepared','upgrade:disabled120','upgrade:archive_file','upgrade:archived120','upgrade:installing121','prepared','after_replace:bank','before_verified_journal']:
  g,b=create('interrupt-'+point.replace(':','-'));inject(g,point);preserved(g,b)
  assert cli(g,'resume')['result']=='verified';verified(g,b);passed('interrupted Upgrade Resume: '+point)
 g,b=create('old-disable-failed');inject(g,'never',old_disable=True);preserved(g,b)
 assert not (g/'.Supermoon-Recovery-1.2.1/ACTIVE.json').exists()
 cli(g,'disable');absent(g);preserved(g,b);passed('failed matching old Disable safely finishes inactive without new DLLs')
 cli(g,'upgrade');verified(g,b);passed('inactive interrupted upgrade can restart after verified archive')
 g,b=create('preinstall-rollback');inject(g,'upgrade:disabled120');cli(g,'rollback');absent(g);preserved(g,b);passed('pre-install interrupted Upgrade rollback stays inactive')
 g,b=create('game-updated-after-interruption');inject(g,'upgrade:prepared');exe=g/'Binaries/NMS.exe';raw=bytearray(exe.read_bytes());raw[-1]^=1;exe.write_bytes(raw);b['Binaries/NMS.exe']=fileid(exe)
 cli(g,'resume',1);cli(g,'disable');absent(g);preserved(g,b);passed('unsupported executable blocks activation but interrupted-upgrade Disable stays available')
 # Actual Windows sharing denial while rolling back, then verified Disable recovery.
 g,b=create('failed-rollback');cli(g,'upgrade');verified(g,b)
 script=f"""$h=[IO.File]::Open({q(g/paths['payload'])},[IO.FileMode]::Open,[IO.FileAccess]::Read,[IO.FileShare]::Read)
try{{& {q(ps)} -NoProfile -ExecutionPolicy RemoteSigned -File {q(stage/'Supermoon/tools/Setup.ps1')} -Action rollback -Game {q(g)} -Package {q(stage)} -ConfirmAction ROLLBACK; if($LASTEXITCODE -ne 1){{throw 'Expected rollback refusal'}}}}finally{{$h.Dispose()}}"""
 run('locked-rollback',[ps,'-NoProfile','-Command',script]);cli(g,'disable');absent(g);preserved(g,b);passed('real sharing failure during rollback recovers through safe Disable')
 for mutation in ['unknown-dll','bad-backup','bad-selector','unsupported-exe','wrong-policy','unknown-record-file']:
  g,b=create(mutation);store=g/'.Supermoon-Recovery';active=json.loads((store/'ACTIVE.json').read_text());record=store/'records'/active['record']
  if mutation=='unknown-dll':p0=g/paths['payload'];raw=bytearray(p0.read_bytes());raw[-1]^=1;p0.write_bytes(raw)
  if mutation=='bad-backup':(record/'new-0.bin').write_bytes(b'CORRUPT')
  if mutation=='bad-selector':active['manifest_sha256']='0'*64;save(store/'ACTIVE.json',active)
  if mutation=='unsupported-exe':p0=g/'Binaries/NMS.exe';raw=bytearray(p0.read_bytes());raw[-1]^=1;p0.write_bytes(raw)
  if mutation=='wrong-policy':
   m=json.loads((record/'manifest.json').read_text());m['release']='PRIVATE';save(record/'manifest.json',m);h=fileid(record/'manifest.json')['sha256'];(record/'manifest.sha256').write_text(h+'\n');active['manifest_sha256']=h;save(store/'ACTIVE.json',active)
  if mutation=='unknown-record-file':(record/'unknown.bin').write_bytes(b'KEEP UNKNOWN')
  snapshot=inventory(g);cli(g,'upgrade',1);assert inventory(g)==snapshot;passed('fail closed without data changes: '+mutation)
 g,b=create('wrong-candidate');bad=w/'wrong-package';shutil.copytree(stage,bad);(bad/'winmm.dll').write_bytes(b'WRONG');snapshot=inventory(g);cli(g,'upgrade',1,package=bad);assert inventory(g)==snapshot;passed('candidate verified before old Disable')
 g,b=create('archive-corrupt');inject(g,'upgrade:archive_file');absent(g)
 archivefiles=list((g/'.Supermoon-Recovery-1.2.1/archives').rglob('*.json'));assert archivefiles;archivefiles[0].write_bytes(b'UNKNOWN COPY')
 before=inventory(g);cli(g,'resume',1);absent(g);assert inventory(g)==before;preserved(g,b);passed('corrupt partial archive refuses without overwriting and stays inactive')
 g,b=create('ordinary-update');snapshot=inventory(g);cli(g,'update',1);assert inventory(g)==snapshot;passed('ordinary Update cannot bypass explicit version transition')
 # Fresh path and explicit preference preservation use the same final extraction.
 g,b=create('fresh',False);cli(g,'install');s=cli(g,'status');assert s['result']=='candidate_present' and s['journal']['phase']=='verified';preserved(g,b)
 cli(g,'disable');absent(g);preserved(g,b);passed('fresh Python-free install/status/disable with explicit neutral INI preserved')
 save(w/'RESULTS.json',{'complete':True,'result':'PASS','pass_count':len(rows),'cases':rows,'package_DLLs':recipe['DLLs'],'setup_processes':calls,'scope':'Exact extracted release package, exact published old DLLs/records, owned game byte fixtures. No game/DLL execution.'})
 print('COMPLETE',len(rows),'PASS',flush=True)

if __name__=='__main__':main()
