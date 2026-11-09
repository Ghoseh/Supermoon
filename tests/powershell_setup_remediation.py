# SPDX-License-Identifier: MIT
"""Developer regressions for F1-F5/R1. Uses original audit inputs; never writes them.

Python is only the test driver. Setup.cmd and production setup never invoke it.
"""
from pathlib import Path
import argparse,copy,ctypes,hashlib,json,os,shutil,subprocess,sys,uuid
SRC=Path(__file__).resolve().parents[1]
PS=Path(os.environ['SystemRoot'])/'System32/WindowsPowerShell/v1.0/powershell.exe'
CMD=Path(os.environ['SystemRoot'])/'System32/cmd.exe'
PATHS={'proxy':'Binaries/winmm.dll','payload':'Binaries/Supermoon/supermoon_playlist.dll','bank':'GAMEDATA/MODS/Supermoon_MenuAudio_Test/audio/windows/init.bnk','marker':'Binaries/bytebeat-playlist-experimental.enabled'}
def ident(p):
 b=Path(p).read_bytes();return dict(bytes=len(b),sha256=hashlib.sha256(b).hexdigest())
def quote(p):return "'"+str(p).replace("'","''")+"'"
def main():
 ap=argparse.ArgumentParser();ap.add_argument('--work',type=Path,required=True);ap.add_argument('--stage',type=Path,required=True);ap.add_argument('--inputs',type=Path,required=True);ap.add_argument('--audit',type=Path,required=True);a=ap.parse_args()
 work=a.work.resolve();work.mkdir(exist_ok=False);stage=a.stage.resolve();inputs=a.inputs.resolve();audit=a.audit.resolve()
 old=audit/'source-530fee2/tools/Setup.ps1';setup=stage/'Supermoon/tools/Setup.ps1'
 findings=json.loads((audit/'FINDINGS.json').read_text(encoding='utf-8'));assert {f['id'] for f in findings['findings']}=={'F1','F2','F3','F4','F5','R1'}
 policy=json.loads((stage/'Supermoon/build/recipe.json').read_text());plan=json.loads((stage/'Supermoon/tools/audio_patch.json').read_text())
 assert ident(inputs/'Binaries/NMS.exe')==dict(bytes=policy['game_executable_bytes'],sha256=policy['game_executable_sha256'])
 assert ident(inputs/'GAMEDATA/PCBANKS/NMSARC.audioBNK.pak')['sha256']=='29682a3015f0ffcfa1ac236e4d49a9670e6d2a79e50a25315f8faecf45cde940'
 tested={rel:ident(stage/rel) for rel in ['Setup.cmd','Supermoon/tools/Setup.ps1','Supermoon/tools/setup/Native.cs','Supermoon/tools/setup/Supermoon.Setup.psm1','Supermoon/tools/audio_patch.json','Supermoon/build/recipe.json']}
 rows=[];counter=0
 def fence(p):p=Path(p).resolve();assert p.is_relative_to(work) and p!=work;return p
 def save(name,obj):
  p=fence(work/name);p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(obj,indent=2,ensure_ascii=True)+'\n',encoding='utf-8')
 def run(label,args,input=None,env=None,expected=0):
  nonlocal counter
  counter+=1;r=subprocess.run([str(v) for v in args],input=input.encode('utf-8') if input is not None else None,capture_output=True,cwd=work,env=env,timeout=45)
  save(f'receipts/{counter:03}-{label}.json',dict(args=[str(v) for v in args],exit=r.returncode,stdout_hex=r.stdout.hex(),stderr_hex=r.stderr.hex(),stdout=r.stdout.decode('utf-8',errors='replace'),stderr=r.stderr.decode('utf-8',errors='replace')))
  assert r.returncode==expected,(label,r.returncode,r.stdout,r.stderr)
  return r
 def cli(action,g,expected=0,legacy=False,consent=True,extra=None,input=None,env=None):
  fence(g);args=[PS,'-NoProfile','-ExecutionPolicy','RemoteSigned','-File',old if legacy else setup,'-Action',action,'-Game',g]
  if action.lower() in ['install','update']:args+=['-Package',stage]
  if action.lower() not in ['status','generate'] and consent:args+=['-ConfirmAction',action.upper()]
  if extra:args+=extra
  r=run(('old-' if legacy else '')+action,args,input=input,env=env,expected=expected)
  if legacy:
   text=r.stdout.decode('utf-8',errors='replace');obj=json.JSONDecoder().raw_decode(text[text.find('{'):])[0]
  else:
   assert not r.stdout.startswith(b'\xef\xbb\xbf');obj=json.loads(r.stdout.decode('utf-8'));assert isinstance(obj,dict)
  return r,obj
 def passed(name):rows.append(dict(case=name,result='PASS'));save('RESULTS.json',dict(complete=False,cases=rows));print(name,'PASS',flush=True)
 def game(name):
  g=fence(work/name);g.mkdir(parents=True,exist_ok=False)
  for rel in ['Binaries/NMS.exe','GAMEDATA/PCBANKS/NMSARC.audioBNK.pak']:
   p=fence(g/rel);p.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(inputs/rel,p)
  for rel,b in personal.items():p=fence(g/rel);p.parent.mkdir(parents=True,exist_ok=True);p.write_bytes(b)
  return g
 def intact(g):
  for rel,b in personal.items():assert (g/rel).read_bytes()==b,rel
  assert external.read_bytes()==b'EXTERNAL PERSONAL MUSIC'
 def resources(g):return {k:ident(g/rel) if (g/rel).exists() else None for k,rel in PATHS.items()}
 def tree(g):return {p.relative_to(g).as_posix():ident(p) for p in g.rglob('*') if p.is_file()}
 def record(g):
  selected=json.loads((g/'.Supermoon-Recovery/ACTIVE.json').read_text(encoding='utf-8'));d=g/'.Supermoon-Recovery/records'/selected['record'];return d,json.loads((d/'manifest.json').read_text(encoding='utf-8'))
 personal={'Binaries/Supermoon/playlist.ini':b'PERSONAL INI','Binaries/Supermoon/Music/owned.wav':b'PERSONAL MUSIC','Binaries/Supermoon/Cache/owned':b'CACHE','Binaries/Supermoon/State/owned':b'STATE','Binaries/Supermoon/diagnostics.log':b'DIAGNOSTICS','GAMEDATA/MODS/Other/keep':b'OTHER MOD','Binaries/other.dll':b'OTHER DLL'}
 external=work/'external-personal-music';external.write_bytes(b'EXTERNAL PERSONAL MUSIC')
 # F1: same effective TEMP configuration and genuine inputs as original audit.
 physical=fence(work/'temporary-physical');physical.mkdir();alias=work/'temporary-alias'
 run('junction',[CMD,'/d','/c','mklink','/J',alias,physical])
 env=os.environ.copy();env['TEMP']=str(alias);env['TMP']=str(alias)
 oldgame=game('temporary-physical/original-installer');newgame=game('temporary-physical/remediated-installer')
 assert cli('install',oldgame,legacy=True,env=env)[1]['result']=='verified'
 _,o=cli('install',newgame,expected=1,env=env);assert 'Reparse-point' in o['error']
 assert all(v is None for v in resources(newgame).values()) and not(newgame/'.Supermoon-Recovery').exists();intact(newgame)
 passed('F1-original-acceptance-new-TEMP-junction-refusal')
 plainenv=os.environ.copy();plainenv['TEMP']=str(physical);plainenv['TMP']=str(physical)
 _,o=cli('install',newgame,expected=1,env=plainenv);assert 'outside TEMP' in o['error'];assert not(newgame/'.Supermoon-Recovery').exists()
 passed('F1-plain-TEMP-containment-refusal-retained')
 k=ctypes.WinDLL('kernel32',use_last_error=True);k.GetShortPathNameW.argtypes=[ctypes.c_wchar_p,ctypes.c_wchar_p,ctypes.c_uint32];k.GetShortPathNameW.restype=ctypes.c_uint32
 short=ctypes.create_unicode_buffer(32768);n=k.GetShortPathNameW(str(physical),short,len(short))
 short_exercised=bool(n and short.value.lower()!=str(physical).lower())
 if short_exercised:
  shortenv=os.environ.copy();shortenv['TEMP']=short.value;shortenv['TMP']=short.value
  _,o=cli('install',newgame,expected=1,env=shortenv);assert 'outside TEMP' in o['error'];assert not(newgame/'.Supermoon-Recovery').exists()
 save('SHORT_ALIAS_COVERAGE.json',dict(exercised=short_exercised,reason=None if short_exercised else 'This directory has no distinct Windows short-name alias; no host setting changed.'))
 passed('F1-short-alias-check' if short_exercised else 'F1-short-alias-availability-recorded')
 g=game('genuine game & é ! % [test]')
 _,o=cli('InStAlL',g);assert o['result']=='verified'
 _,o=cli('STATUS',g);assert o['game']==str(g);passed('F3-F4-strict-UTF8-JSON-and-mixed-case-install-status')
 d,m=record(g);manifest=(d/'manifest.json').read_bytes();active=g/'.Supermoon-Recovery/ACTIVE.json';valid=active.read_bytes()
 cli('ROLLBACK',g);assert all(v is None for v in resources(g).values())
 # F2 audit shape and additional invalid selectors. All admissions must be unchanged.
 value=json.loads(valid)
 malformed=[('singleton',[value]),('empty',[]),('nested',[[value]]),('multiple',[value,value]),('null',None),('string','record'),('record-array',dict(value,record=[value['record']])),('hash-array',dict(value,manifest_sha256=[value['manifest_sha256']])),('unknown-key',dict(value,unexpected=1)),('hash-type',dict(value,manifest_sha256=1))]
 for label,obj in malformed:
  active.write_bytes(json.dumps(obj).encode('utf-8'));before=tree(g)
  assert cli('status',g,expected=1)[1]['result']=='CORRUPT_RECORD_REFUSED'
  for action in ['resume','disable']:cli(action,g,expected=1)
  assert tree(g)==before,label;active.write_bytes(valid)
 assert (d/'manifest.json').read_bytes()==manifest
 passed('F2-ten-invalid-selector-roots-fields-refuse-before-writes')
 # Original shape reproduction still fails at 530fee2; read-only old tools only.
 active.write_bytes(b'['+valid.strip()+b']');assert cli('status',g,legacy=True)[1]['result']=='prior_present';active.write_bytes(valid)
 passed('F2-original-array-acceptance-regression-counterexample')
 # Successful case-insensitive actions. Strict stdout is parsed for every CLI call.
 for action,expected_result in [('ReSuMe','verified'),('UpDaTe','verified'),('DiSaBlE','disabled'),('INSTALL','verified'),('GeNeRaTe','PASS_GENERATED_IN_MEMORY_NO_GAME_WRITES')]:
  assert cli(action,g)[1]['result']==expected_result
 assert (g/PATHS['bank']).stat().st_size==36017 and ident(g/PATHS['bank'])['sha256']==plan['output_sha256']
 passed('F3-F4-all-seven-actions-strict-JSON-and-case-consistency')
 r,_=cli('resume',g,legacy=True)
 try:json.loads(r.stdout)
 except (ValueError,UnicodeError):pass
 else:raise AssertionError('Original stdout counterexample did not reproduce')
 assert cli('RESUME',g,legacy=True,expected=1)[1]['error']=='Unknown setup action'
 passed('F3-F4-original-CLI-counterexamples-retained')
 before=tree(g)
 for args in [['-Action','UNKNOWN'],['-Action'],['-UnknownOption'],['-Game'],['-Action','status','-Action','status']]:
  r=run('JSON-argument-failure',[PS,'-NoProfile','-ExecutionPolicy','RemoteSigned','-File',setup,*args],expected=1)
  assert json.loads(r.stdout.decode('utf-8'))['result']=='REFUSED_OR_INCOMPLETE'
 cli('install',g,expected=1,consent=False,input='NO\n');assert tree(g)==before
 passed('F3-argument-errors-and-cancellation-are-JSON-only-no-writes')
 # F5: all activation paths still refuse changed game images; GUI Disable works.
 for variant in ['missing','truncated','oversized','sharing-denied']:
  cli('install',g);exe=g/'Binaries/NMS.exe';handle=None
  if variant=='missing':fence(exe).unlink()
  elif variant=='truncated':exe.write_bytes(b'UNSUPPORTED')
  elif variant=='oversized':
   with exe.open('wb') as f:f.truncate(134217729)
  else:
   k.CreateFileW.argtypes=[ctypes.c_wchar_p,ctypes.c_uint32,ctypes.c_uint32,ctypes.c_void_p,ctypes.c_uint32,ctypes.c_uint32,ctypes.c_void_p];k.CreateFileW.restype=ctypes.c_void_p;k.CloseHandle.argtypes=[ctypes.c_void_p]
   handle=k.CreateFileW(str(exe),0x80000000,0,None,3,0x80,None);assert handle!=ctypes.c_void_p(-1).value
  try:
   current=resources(g)
   for action in ['install','update','resume','rollback','generate']:cli(action,g,expected=1);assert resources(g)==current
   r=run('GUI-disable-'+variant,[PS,'-NoProfile','-ExecutionPolicy','RemoteSigned','-File',setup,'-Interactive','-Game',g],input='6\nDISABLE\n')
   assert b'1 Status' in r.stdout and b'Completed: disabled' in r.stdout;assert all(v is None for v in resources(g).values())
  finally:
   if handle:k.CloseHandle(handle)
   shutil.copyfile(inputs/'Binaries/NMS.exe',exe)
  intact(g)
 passed('F5-four-game-identity-errors-GUI-disable-and-strict-activation-refusal')
 cli('install',g);(g/'Binaries/NMS.exe').write_bytes(b'UNSUPPORTED');payload=g/PATHS['payload'];saved=payload.read_bytes();payload.write_bytes(b'FOREIGN PAYLOAD');before=tree(g)
 run('GUI-unknown-payload-refusal',[PS,'-NoProfile','-ExecutionPolicy','RemoteSigned','-File',setup,'-Interactive','-Game',g],input='6\nDISABLE\n',expected=1)
 assert tree(g)==before;payload.write_bytes(saved);shutil.copyfile(inputs/'Binaries/NMS.exe',g/'Binaries/NMS.exe');cli('disable',g)
 passed('F5-unknown-managed-file-refusal-not-weakened')
 # R1: use the original VDF vector and guard before any provider/network action.
 fake=work/'fake-programfiles/Steam/steamapps';fake.mkdir(parents=True)
 audit_vdf=json.loads((audit/'DISCOVERY_ORDER_RESULT.json').read_text())['VDF']
 junction=work/'library-junction';run('library-junction',[CMD,'/d','/c','mklink','/J',junction,physical])
 forbidden=[chr(92)*2+'127.0.0.1'+chr(92)+'SupermoonAuditNoShare',chr(92)*2+'?'+chr(92)+'C:'+chr(92)+'unsupported','C:relative',str(junction)]
 vdf=audit_vdf+'\n'+ '\n'.join('"path" '+json.dumps(s) for s in forbidden[1:]);(fake/'libraryfolders.vdf').write_text(vdf,encoding='utf-8')
 forbidden_file=work/'forbidden-roots.json';forbidden_file.write_text(json.dumps(forbidden),encoding='utf-8')
 env=os.environ.copy();env['ProgramFiles(x86)']=str(work/'fake-programfiles')
 for legacy in [True,False]:
  trace=work/('discovery-old.trace' if legacy else 'discovery-new.trace')
  r=run('discovery-'+str(legacy),[PS,'-NoProfile','-ExecutionPolicy','RemoteSigned','-File',SRC/'tests/setup_discovery_guard.ps1','-Setup',old if legacy else setup,'-Trace',trace,'-Forbidden',forbidden_file],env=env,input=str(g)+'\n7\n')
  assert trace.exists()==legacy
  if not legacy:assert b'unsupported Steam library' in r.stdout
 passed('R1-original-VDF-UNC-device-relative-junction-before-provider-guard')
 # Fallback root admission also precedes Join-Path.
 env['ProgramFiles(x86)']=forbidden[0];trace=work/'unsupported-root.trace'
 run('nonlocal-fallback-root',[PS,'-NoProfile','-ExecutionPolicy','RemoteSigned','-File',SRC/'tests/setup_discovery_guard.ps1','-Setup',setup,'-Trace',trace,'-Forbidden',forbidden_file],env=env,input=str(g)+'\n7\n');assert not trace.exists()
 passed('R1-nonlocal-environment-root-refused-before-provider')
 # Preserve original public record semantics; no policy or manifest relabeling.
 sys.path.insert(0,str(SRC/'tools'));import install_supermoon as original
 assert original.Installation(g,policy,plan).status()['result']=='prior_present'
 # Exercise the exported dispatcher itself, independently of CLI normalization.
 command=';'.join([
  '$ErrorActionPreference="Stop"',
  'Import-Module '+quote(stage/'Supermoon/tools/setup/Supermoon.Setup.psm1'),
  '$r=Read-SMJson '+quote(stage/'Supermoon/build/recipe.json'),
  '$p=Read-SMJson '+quote(stage/'Supermoon/tools/audio_patch.json'),
  '$c=New-SMContext '+quote(g)+' $r $p',
  'Invoke-SMOperation $c "StAtUs" | ConvertTo-Json -Depth 24'])
 r=run('direct-module-mixed-case-status',[PS,'-NoProfile','-ExecutionPolicy','RemoteSigned','-Command',command])
 assert json.loads(r.stdout)['result']=='prior_present'
 assert (d/'manifest.json').read_bytes()==manifest;intact(g);intact(newgame)
 passed('public-record-interoperability-and-personal-preservation')
 assert tested=={rel:ident(stage/rel) for rel in tested}
 save('RESULTS.json',dict(complete=True,PASS=len(rows),FAIL=0,cases=rows,tested_inputs=tested,short_alias_exercised=short_exercised,original_audit_findings=ident(audit/'FINDINGS.json'),physical_Explorer_test=False,network_effects_exercised=False))
 print('Remediation regressions:',len(rows),'PASS',flush=True)
if __name__=='__main__':main()
