# SPDX-License-Identifier: MIT
"""Developer-only Python harness. Tests shipped PowerShell with private data copies.

No Python code is called by Setup.cmd/Setup.ps1. No NMS or production DLL executes.
"""
from pathlib import Path
import argparse,hashlib,json,os,shutil,subprocess,sys,time,uuid
ROOT=Path(__file__).resolve().parents[1]
PS=Path(os.environ['SystemRoot'])/'System32/WindowsPowerShell/v1.0/powershell.exe'
PATHS={'proxy':'Binaries/winmm.dll','payload':'Binaries/Supermoon/supermoon_playlist.dll','bank':'GAMEDATA/MODS/Supermoon_MenuAudio_Test/audio/windows/init.bnk','marker':'Binaries/bytebeat-playlist-experimental.enabled'}
SENTINELS={'Binaries/Supermoon/playlist.ini':b'OWNED INI','Binaries/Supermoon/Music/music.wav':b'OWNED MUSIC','Binaries/Supermoon/Cache/owned':b'OWNED CACHE','Binaries/Supermoon/State/owned':b'OWNED STATE','Binaries/Supermoon/diagnostics.log':b'OWNED DIAGNOSTICS','GAMEDATA/MODS/OtherMod/keep':b'OTHER MOD','Binaries/SETTINGS/GCMODSETTINGS.MXML':b'OWNED SETTINGS','Binaries/other.dll':b'OTHER DLL'}
def ident_bytes(b):return dict(bytes=len(b),sha256=hashlib.sha256(b).hexdigest())
def ident(p):return ident_bytes(Path(p).read_bytes())
def quote(s):return "'"+str(s).replace("'","''")+"'"
def main():
 ap=argparse.ArgumentParser();ap.add_argument('--work',type=Path,required=True);ap.add_argument('--inputs',type=Path,required=True);ap.add_argument('--stage',type=Path,required=True);ap.add_argument('--owned-process',type=Path,required=True);a=ap.parse_args()
 work=a.work.resolve();work.mkdir(exist_ok=False);stage=a.stage.resolve();inputs=a.inputs.resolve()
 (work/'OWNED_FIXTURE_ROOT.txt').write_text('Disposable setup tests. No game/DLL execution.\n')
 rows=[];counter=0;setup=stage/'Supermoon/tools/Setup.ps1';module=stage/'Supermoon/tools/setup/Supermoon.Setup.psm1'
 policy=json.loads((stage/'Supermoon/build/recipe.json').read_text());plan=json.loads((stage/'Supermoon/tools/audio_patch.json').read_text())
 tested={rel:ident(stage/rel) for rel in ['Setup.cmd','Supermoon/tools/Setup.ps1','Supermoon/tools/setup/Native.cs','Supermoon/tools/setup/Supermoon.Setup.psm1','Supermoon/tools/audio_patch.json','Supermoon/build/recipe.json','winmm.dll','Supermoon/supermoon_playlist.dll']}
 policy_command=[str(PS),'-NoProfile','-Command','Get-ExecutionPolicy -List | ConvertTo-Json -Compress']
 policy_before=subprocess.check_output(policy_command)
 assert ident(inputs/'Binaries/NMS.exe')==dict(bytes=policy['game_executable_bytes'],sha256=policy['game_executable_sha256'])
 def fence(p):
  p=Path(p).resolve();assert p.is_relative_to(work) and p!=work;return p
 def save(n,v):fence(work/n).write_text(json.dumps(v,indent=2,ensure_ascii=True)+'\n')
 def put(g,rel,b):
  p=fence(g/rel);p.parent.mkdir(parents=True,exist_ok=True);p.write_bytes(b)
 def tree(g):return {p.relative_to(g).as_posix():ident(p) for p in g.rglob('*') if p.is_file()}
 def create(name):
  g=fence(work/name);g.mkdir()
  for rel in ['Binaries/NMS.exe','GAMEDATA/PCBANKS/NMSARC.audioBNK.pak']:
   p=fence(g/rel);p.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(inputs/rel,p)
  for rel,b in SENTINELS.items():put(g,rel,b)
  return g
 def intact(g):
  for rel,b in SENTINELS.items():assert (g/rel).read_bytes()==b,rel
 def managed(g):return {k:ident(g/p) if (g/p).exists() else None for k,p in PATHS.items()}
 def record(g):
  active=json.loads((g/'.Supermoon-Recovery/ACTIVE.json').read_text());d=g/'.Supermoon-Recovery/records'/active['record'];return d,json.loads((d/'manifest.json').read_text())
 def run(args,input=None,expected=0,label='command'):
  nonlocal counter
  counter+=1;r=subprocess.run(args if isinstance(args,str) else [str(v) for v in args],input=input,text=True,encoding='utf-8',errors='replace',capture_output=True,cwd=work,timeout=45)
  save(f'{counter:03}-{label}.json',dict(args=args if isinstance(args,str) else [str(v) for v in args],exit=r.returncode,stdout=r.stdout,stderr=r.stderr))
  assert r.returncode==expected,(label,r.returncode,r.stdout,r.stderr)
  return r
 def cli(action,g,expected=0,script=setup,package=stage,record_id=None,consent=True):
  fence(g);args=[PS,'-NoProfile','-ExecutionPolicy','RemoteSigned','-File',script,'-Action',action,'-Game',g]
  if action in ['install','update']:args+=['-Package',package]
  if action not in ['status','generate'] and consent:args+=['-ConfirmAction',action.upper()]
  if record_id:args+=['-Record',record_id]
  r=run(args,input='NO\n' if not consent else None,expected=expected,label=action)
  i=r.stdout.find('{');assert i>=0,r.stdout
  return json.JSONDecoder().raw_decode(r.stdout[i:])[0]
 def passed(name):
  rows.append(dict(case=name,result='PASS'));print(name,'PASS',flush=True);save('RESULTS.json',dict(complete=False,cases=rows))
 g=create('public game & é ! % ^ (test) [1]')
 before=tree(g);assert cli('status',g)['result']=='no_selected_record';assert tree(g)==before;passed('01 read-only clean status')
 assert cli('install',g)['result']=='verified';intact(g);d,m=record(g);raw=(d/'manifest.json').read_bytes();rid=m['record'];assert all(v is None for v in m['before'].values());assert managed(g)==m['after'];passed('02 exact-byte clean install and public record')
 assert cli('status',g)['result']=='candidate_present';passed('03 installed identity status')
 assert cli('update',g)['record']==rid and (d/'manifest.json').read_bytes()==raw;intact(g);passed('04 update retains first prior state and personal files')
 # Original Python tools must read new records, and new tools must read original records.
 sys.path.insert(0,str(ROOT/'tools'));import install_supermoon as old;import prepare_audio as audio
 tool=old.Installation(g,policy,plan);assert tool.status()['result']=='candidate_present';passed('05 original Python tools validate PowerShell record without rewriting')
 assert cli('rollback',g)['result']=='rolled_back';assert all(v is None for v in managed(g).values());passed('06 rollback to absence')
 assert cli('resume',g)['result']=='verified' and managed(g)==m['after'];passed('07 saved-candidate resume')
 assert cli('disable',g)['result']=='disabled';assert all(v is None for v in managed(g).values());passed('08 disable preserves data and records')
 assert cli('install',g)['record']!=rid and d.exists();passed('09 reinstall creates new record')
 d,m=record(g);p=g/PATHS['payload'];saved=p.read_bytes();p.write_bytes(b'FOREIGN');before=tree(g);cli('disable',g,1);assert tree(g)==before;p.write_bytes(saved);passed('10 foreign payload refusal before changes')
 p=d/'new-0.bin';saved=p.read_bytes();p.write_bytes(b'CORRUPT');before=tree(g);cli('rollback',g,1);assert tree(g)==before;p.write_bytes(saved);passed('11 corrupt backup refusal before changes')
 p=g/'Binaries/NMS.exe';saved=p.read_bytes();p.write_bytes(b'UNSUPPORTED');before=managed(g);cli('rollback',g,1);assert managed(g)==before;assert cli('disable',g)['result']=='disabled';p.write_bytes(saved);passed('12 unsupported executable refuses activation and permits disable')
 put(g,PATHS['proxy'],b'FOREIGN PROXY');before=managed(g);cli('install',g,1);assert managed(g)==before;fence(g/PATHS['proxy']).unlink();passed('13 foreign proxy refused')
 rc=create('private RC policy data fixture');rcpolicy=json.loads(json.dumps(policy));rcpolicy['version']='1.0.4-rc1-private'
 desired={'proxy':(stage/'winmm.dll').read_bytes(),'payload':(stage/'Supermoon/supermoon_playlist.dll').read_bytes(),'marker':old.MARKER,'bank':audio.derive(rc,plan)}
 rctool=old.Installation(rc,rcpolicy,plan);rctool.mutate('install',desired);rd,rm=record(rc);before=tree(rc)
 assert cli('status',rc,1)['result']=='CORRUPT_RECORD_REFUSED';assert tree(rc)==before;passed('14 public tools refuse RC policy without changes')
 cli('resume',rc,1,record_id=rm['record']);assert tree(rc)==before;cli('install',rc,1);assert tree(rc)==before;assert rctool.status()['result']=='candidate_present';passed('15 explicit selection cannot import RC records')
 ig=create('original Python interruption')
 it=old.Installation(ig,policy,plan)
 def interrupt(at):
  if at=='after_replace:bank':raise RuntimeError('OWNED AFTER BANK INTERRUPTION')
 old.checkpoint=interrupt
 try:it.mutate('install',desired)
 except ValueError:pass
 else:raise AssertionError('Checkpoint not reached')
 finally:old.checkpoint=lambda at:None
 assert cli('status',ig)['result']=='partial_or_disabled';assert not (ig/PATHS['marker']).exists();passed('16 original Python interrupted record accepted and diagnosed')
 _,im=record(ig);assert cli('resume',ig)['record']==im['record'];assert cli('rollback',ig)['result']=='rolled_back';intact(ig);passed('17 PowerShell resumes and rolls back original records')
 # Actual output byte comparison. Files remain private under the developer test root.
 out=fence(work/'powershell-init.bnk');bank=audio.derive(g,plan);fence(work/'python-init.bnk').write_bytes(bank)
 command=f"Import-Module {quote(module)}; $r=Read-SMJson {quote(stage/'Supermoon/build/recipe.json')}; $p=Read-SMJson {quote(stage/'Supermoon/tools/audio_patch.json')}; $c=New-SMContext {quote(g)} $r $p; & (Get-Module Supermoon.Setup) {{param($c,$out) [IO.File]::WriteAllBytes($out,(Get-SMAudioBank $c))}} $c {quote(out)}"
 run([PS,'-NoProfile','-ExecutionPolicy','RemoteSigned','-Command',command],label='bank-bytes');assert out.read_bytes()==bank
 save('BANK_COMPARISON.json',dict(equal_byte_for_byte=True,python=ident_bytes(bank),powershell=ident(out),plan=ident(stage/'Supermoon/tools/audio_patch.json')));passed('18 exact bank byte comparison')
 # Unsupported archives: small, same-size changed, and missing.
 bad=create('bad archive');p=bad/'GAMEDATA/PCBANKS/NMSARC.audioBNK.pak';original=p.read_bytes()
 for b in [b'TRUNCATED',bytes([original[0]^1])+original[1:]]:
  p.write_bytes(b);cli('install',bad,1);assert all(v is None for v in managed(bad).values())
 p.unlink();cli('generate',bad,1);p.write_bytes(original);passed('19 corrupt and missing archive rejected')
 before=tree(g);cli('install',g,1,consent=False);assert tree(g)==before;passed('20 explicit consent refusal is read-only')
 cg=create('loose bank conflict');put(cg,'GAMEDATA/MODS/Conflict/audio/windows/init.bnk',b'FOREIGN');before=tree(cg);cli('install',cg,1);assert tree(cg)==before;passed('21 conflicting loose resource refused')
 # Genuine process guard: pre-existing owned helper, renamed, never native NMS.
 helper=fence(work/'owned-process/NMS.exe');helper.parent.mkdir();shutil.copyfile(a.owned_process,helper)
 child=subprocess.Popen([str(helper)],stdout=subprocess.PIPE,text=True)
 try:
  assert 'OWNED PROCESS FIXTURE READY' in child.stdout.readline();before=tree(g);cli('install',g,1);assert tree(g)==before
 finally:child.terminate();child.wait(timeout=5)
 passed('22 real process snapshot detects owned NMS-named helper')
 # Five abrupt process deaths. The no-op is replaced only in the child test module scope.
 for checkpoint in ['prepared','before_replace:bank','after_replace:bank','before_verified_journal','after_remove:marker']:
  cg=create('crash-'+checkpoint.replace(':','-'))
  if checkpoint=='after_remove:marker':
   for k in ['proxy','payload','marker']:put(cg,PATHS[k],desired[k])
  marker=fence(work/('checkpoint-'+uuid.uuid4().hex))
  command=f"Import-Module {quote(module)}; & (Get-Module Supermoon.Setup) {{param($g,$stage,$signal,$at) $script:Signal=$signal;$script:StopAt=$at; function script:Invoke-SMCheckpoint($Stage) {{if($Stage -ceq $script:StopAt){{[IO.File]::WriteAllText($script:Signal,'READY');[Threading.Thread]::Sleep(60000)}}}}; $c=New-SMContext $g (Read-SMJson (Join-Path $stage 'Supermoon/build/recipe.json')) (Read-SMJson (Join-Path $stage 'Supermoon/tools/audio_patch.json')); Invoke-SMOperation $c install $stage }} {quote(cg)} {quote(stage)} {quote(marker)} {quote(checkpoint)}"
  child=subprocess.Popen([str(PS),'-NoProfile','-ExecutionPolicy','RemoteSigned','-Command',command],stdout=subprocess.PIPE,stderr=subprocess.PIPE)
  try:
   deadline=time.monotonic()+20
   while not marker.exists() and child.poll() is None and time.monotonic()<deadline:time.sleep(.05)
   assert marker.exists(),child.communicate(timeout=1) if child.poll() is not None else checkpoint
   # Another setup must be excluded while the first owns the writer handle.
   if checkpoint=='prepared':cli('install',cg,1)
  finally:
   if child.poll() is None:child.terminate()
   child.communicate(timeout=5)
  _,cm=record(cg);assert cli('resume',cg)['record']==cm['record'];assert cli('rollback',cg)['result']=='rolled_back';intact(cg)
 passed('23 five abrupt deaths recover and concurrent writer refuses')
 # CLI cancellation / first menu is tested separately from shell-launch behavior.
 # Launcher accepts no argument text; its script path must work with hostile-looking names.
 launcher=fence(work/'launcher & é ! % ^ (test) [1]');shutil.copytree(stage,launcher)
 def cmdline(path):return '"'+os.environ['SystemRoot']+'\\System32\\cmd.exe" /d /s /c ""'+str(path)+'""'
 run(cmdline(launcher/'Setup.cmd'),input=str(g)+'\n7\n\n',label='launcher-exit')
 passed('24 cmd launcher in Unicode and metacharacter directory')
 # Missing script is a clear error, remains open through pause, and returns failure.
 missing=fence(work/'missing-companion');missing.mkdir();shutil.copyfile(stage/'Setup.cmd',missing/'Setup.cmd')
 r=run(cmdline(missing/'Setup.cmd'),input='\n',expected=1,label='launcher-missing');assert 'missing' in r.stdout.lower();passed('25 missing companion launcher refusal')
 # Mark-of-the-Web: RemoteSigned must refuse unsigned downloaded script. No bypass/unblock.
 blocked=fence(work/'downloaded-script');shutil.copytree(stage,blocked);ps=blocked/'Supermoon/tools/Setup.ps1'
 with open(str(ps)+':Zone.Identifier','w') as f:f.write('[ZoneTransfer]\nZoneId=3\n')
 r=run(cmdline(blocked/'Setup.cmd'),input='\n',expected=1,label='launcher-policy-block');assert 'SETUP_SECURITY.txt' in r.stdout;passed('26 downloaded unsigned script restrictions preserved')
 interactive=create('interactive install game')
 r=run(cmdline(launcher/'Setup.cmd'),input=str(interactive)+'\n2\nINSTALL\n\n',label='launcher-install')
 assert 'verified' in r.stdout and managed(interactive)==record(interactive)[1]['after'];intact(interactive)
 passed('27 interactive install completes after explicit confirmation')
 cancel=create('interactive cancellation');before=tree(cancel)
 run(cmdline(launcher/'Setup.cmd'),input=str(cancel)+'\n2\nNO\n\n',expected=1,label='launcher-cancel');assert tree(cancel)==before
 passed('28 interactive cancellation leaves game unchanged')
 # Executable-search sentinel: the setup process must never resolve tasklist from cwd/PATH.
 sentinel=fence(work/'tasklist.exe');shutil.copyfile(a.owned_process,sentinel)
 previous=os.environ.get('PATH','');os.environ['PATH']=str(work)+os.pathsep+previous
 try:cli('install',cancel)
 finally:os.environ['PATH']=previous
 assert not (work/'HIJACK_EXECUTED').exists();intact(cancel);passed('29 cwd and PATH executable-search sentinel never runs')
 before=tree(g);assert old.Installation(g,rcpolicy,plan).status()['result']=='CORRUPT_RECORD_REFUSED';assert tree(g)==before
 passed('30 private-policy tools refuse public record without changes')
 # CurrentUser/LocalMachine policy settings are observed, never altered by either launcher path.
 command="Get-ExecutionPolicy -List | ConvertTo-Json -Compress"
 r=run([PS,'-NoProfile','-Command',command],label='policy-observation')
 assert subprocess.check_output(policy_command)==policy_before
 save('EXECUTION_POLICY_OBSERVATION.json',dict(stdout=r.stdout,before_after_equal=True,scope='Persistent scope observations equal. No Set-ExecutionPolicy.'))
 for game in [g,rc,ig,bad,cg]:intact(game)
 assert tested=={rel:ident(stage/rel) for rel in tested},'Tested inputs changed during execution'
 save('RESULTS.json',dict(complete=True,pass_count=len(rows),fail_count=0,cases=rows,powershell='Windows PowerShell 5.1',game_or_production_DLL_execution=False,tested_inputs=tested))
 print('Integration totals:',len(rows),'PASS',flush=True)
if __name__=='__main__':main()
