# SPDX-License-Identifier: MIT
"""Conservative four-file Supermoon installation, inspection and recovery.

Single writer only: keep NMS, Steam updates, extractors and other filesystem writers
idle. Checks narrow races; they do not provide hostile namespace coordination.
"""
from pathlib import Path
import argparse,datetime,hashlib,json,os,re,stat,sys,tempfile,uuid
from installation_safety import no_reparse,require_closed,writer_lock

ROOT=Path(__file__).resolve().parents[1]
MARKER=b'ByteBeat automatic startup v1\r\n'
PATHS={'proxy':'Binaries/winmm.dll','payload':'Binaries/Supermoon/supermoon_playlist.dll',
       'bank':'GAMEDATA/MODS/Supermoon_MenuAudio_Test/audio/windows/init.bnk',
       'marker':'Binaries/bytebeat-playlist-experimental.enabled'}
KEYS=tuple(PATHS)
MENU_SHA='0db1fbc668384e7fb618009f09e889dd32b6b3b3b0b9806f5d099ca8b8ecfc46'


def identity(data):return {'bytes':len(data),'sha256':hashlib.sha256(data).hexdigest()}


def read_file(path,limit=32*1024*1024):
    no_reparse(path)
    try:s=path.lstat()
    except FileNotFoundError:return None
    if not stat.S_ISREG(s.st_mode) or s.st_nlink!=1 or s.st_size>limit:
        raise ValueError('Unsupported file type, hard link or size: '+str(path))
    data=path.read_bytes()
    if len(data)>limit:raise ValueError('File grew beyond limit: '+str(path))
    return data


def current(path,limit=32*1024*1024):
    data=read_file(path,limit)
    return identity(data) if data is not None else None


def unique_pairs(pairs):
    result={}
    for k,v in pairs:
        if k in result:raise ValueError('Duplicate recovery JSON key')
        result[k]=v
    return result


def load_json(path):
    raw=read_file(path,65536)
    if raw is None:raise ValueError('Missing recovery record: '+str(path))
    return json.loads(raw.decode('utf-8'),object_pairs_hook=unique_pairs)


def durable_file(path,data):
    no_reparse(path)
    with path.open('xb') as f:f.write(data);f.flush();os.fsync(f.fileno())
    if read_file(path)!=data:raise ValueError('Durable backup readback failed: '+str(path))


def durable_json(path,value):
    raw=(json.dumps(value,indent=2,sort_keys=True)+'\n').encode('utf-8')
    no_reparse(path);fd,tmp=tempfile.mkstemp(prefix='.record-',dir=path.parent)
    tmp=Path(tmp)
    try:
        with os.fdopen(fd,'wb') as f:f.write(raw);f.flush();os.fsync(f.fileno())
        if read_file(tmp)!=raw:raise ValueError('Record staging readback failed')
        os.replace(tmp,path)
        if read_file(path)!=raw:raise ValueError('Record publication readback failed')
    finally:
        if tmp.exists():tmp.unlink()


def checkpoint(stage):
    """No-op seam for owned crash/fault fixtures; never controlled by CLI/environment."""


class Installation:
    def __init__(self,game,recipe,plan):
        game=Path(game).absolute();no_reparse(game);self.game=game.resolve(strict=True)
        self.recipe=recipe;self.plan=plan;self.store=self.game/'.Supermoon-Recovery'
        self.expected={'proxy':recipe['DLLs']['winmm.dll'],
                       'payload':recipe['DLLs']['supermoon_playlist.dll'],
                       'bank':{'bytes':plan['output_bytes'],'sha256':plan['output_sha256']},
                       'marker':identity(MARKER)}

    def target(self,key):return self.game/PATHS[key]

    def game_matches(self):
        return current(self.game/'Binaries/NMS.exe',128*1024*1024)=={'bytes':self.recipe['game_executable_bytes'],'sha256':self.recipe['game_executable_sha256']}

    def require_game(self):
        if not self.game_matches():raise ValueError('Unsupported NMS executable; use disable recovery, never force an old pair')

    def approved(self,key,value):
        return value is None or value==self.expected[key] or (key=='bank' and value['sha256']==MENU_SHA and value['bytes']<=65536)

    def coherent(self,before):
        return all(v is None for v in before.values()) or (
            all(before[k]==self.expected[k] for k in ['proxy','payload','marker']) and
            before['bank'] in [None,self.expected['bank']])

    def observed(self):return {k:current(self.target(k)) for k in KEYS}

    def active(self):
        p=self.store/'ACTIVE.json'
        if read_file(p,65536) is None:return None
        a=load_json(p)
        if set(a)!={'record','manifest_sha256'}:raise ValueError('Invalid active selector')
        return a

    def load_record(self,record=None):
        a=self.active() if record is None else None
        name=record if record is not None else a['record'] if a else None
        if not isinstance(name,str) or not re.fullmatch('[0-9a-f]{32}',name):raise ValueError('No valid selected recovery record')
        folder=self.store/'records'/name;no_reparse(folder)
        raw=read_file(folder/'manifest.json',65536)
        if raw is None:raise ValueError('Missing manifest in '+str(folder))
        pin=read_file(folder/'manifest.sha256',256)
        digest=hashlib.sha256(raw).hexdigest()
        if pin!=(digest+'\n').encode('ascii') or (a and a['manifest_sha256']!=digest):raise ValueError('Recovery manifest hash mismatch')
        m=load_json(folder/'manifest.json')
        if set(m)!={'schema','record','game','release','game_identity','before','after','utc'} or m['schema']!=1 or m['record']!=name or m['game']!=str(self.game):
            raise ValueError('Recovery target/schema mismatch')
        expected_game={'bytes':self.recipe['game_executable_bytes'],'sha256':self.recipe['game_executable_sha256']}
        if m['release']!=self.recipe['version'] or m['game_identity']!=expected_game or m['after']!=self.expected or set(m['before'])!=set(KEYS):
            raise ValueError('Recovery release/identity policy mismatch')
        for n,key in enumerate(KEYS):
            if not self.approved(key,m['before'][key]):raise ValueError('Unrecognized prior identity in record')
            for prefix,values in [('prior',m['before']),('new',m['after'])]:
                raw=read_file(folder/(prefix+'-'+str(n)+'.bin'))
                actual=identity(raw) if raw is not None else None
                if actual!=values[key]:raise ValueError('Recovery backup corrupt/missing: '+prefix+' '+key)
        return folder,m

    def stage(self,desired):
        if set(desired)!=set(KEYS) or {k:identity(v) for k,v in desired.items()}!=self.expected:
            raise ValueError('Candidate does not match the exact DLL/marker/bank identities')
        no_reparse(self.store)
        # Recovery is anchored in this persistent game installation, never mkdtemp.
        # Refuse obvious TEMP placement for CLI and tests with explicit isolated TEMP.
        temp=Path(tempfile.gettempdir()).resolve()
        if self.store==temp or self.store.is_relative_to(temp):raise ValueError('Recovery storage must be outside TEMP; install the game in a persistent directory')
        a=self.active()
        if a:
            folder,m=self.load_record();journal=load_json(folder/'journal.json')
            if journal.get('phase') not in ['rolled_back','disabled']:return folder,m
        before_data={k:read_file(self.target(k)) for k in KEYS}
        before={k:identity(v) if v is not None else None for k,v in before_data.items()}
        if not all(self.approved(k,v) for k,v in before.items()):raise ValueError('Unknown installed output/proxy/marker; preserve it and resolve the conflict')
        name=uuid.uuid4().hex;folder=self.store/'records'/name;no_reparse(folder);folder.mkdir(parents=True,exist_ok=False)
        for n,key in enumerate(KEYS):
            checkpoint('before_backup:'+key)
            if before_data[key] is not None:durable_file(folder/('prior-'+str(n)+'.bin'),before_data[key])
            durable_file(folder/('new-'+str(n)+'.bin'),desired[key])
        m={'schema':1,'record':name,'game':str(self.game),'release':self.recipe['version'],
           'game_identity':{'bytes':self.recipe['game_executable_bytes'],'sha256':self.recipe['game_executable_sha256']},
           'before':before,'after':self.expected,'utc':datetime.datetime.now(datetime.timezone.utc).isoformat()}
        raw=(json.dumps(m,indent=2,sort_keys=True)+'\n').encode('utf-8')
        durable_file(folder/'manifest.json',raw);digest=hashlib.sha256(raw).hexdigest()
        durable_file(folder/'manifest.sha256',(digest+'\n').encode('ascii'))
        durable_json(folder/'journal.json',{'phase':'prepared','observed':before})
        # Publish selector only after every immutable backup/manifest has read back.
        self.load_record(name)
        durable_json(self.store/'ACTIVE.json',{'record':name,'manifest_sha256':digest})
        checkpoint('prepared')
        return folder,m

    def check_live(self,m):
        observed=self.observed()
        for key,value in observed.items():
            if value not in [None,m['before'][key],m['after'][key]]:
                raise ValueError('Unknown current '+key+'; no recovery overwrite is allowed')
        return observed

    def change(self,key,data,m):
        require_closed();path=self.target(key);no_reparse(path)
        actual=current(path)
        if actual not in [None,m['before'][key],m['after'][key]]:raise ValueError('Destination changed: '+key)
        wanted=identity(data) if data is not None else None
        if actual==wanted:return
        if data is None:
            checkpoint('before_remove:'+key);path.unlink();checkpoint('after_remove:'+key)
        else:
            path.parent.mkdir(parents=True,exist_ok=True);no_reparse(path)
            fd,tmp=tempfile.mkstemp(prefix='.supermoon-'+m['record']+'-'+key+'-',suffix='.tmp',dir=path.parent);tmp=Path(tmp)
            try:
                with os.fdopen(fd,'wb') as f:f.write(data);f.flush();os.fsync(f.fileno())
                if read_file(tmp)!=data:raise ValueError('Candidate staging verification failed')
                require_closed();no_reparse(path)
                if current(path)!=actual:raise ValueError('Destination changed before replacement: '+key)
                checkpoint('before_replace:'+key);os.replace(tmp,path);checkpoint('after_replace:'+key)
            finally:
                if tmp.exists():tmp.unlink()
        if current(path)!=wanted:raise ValueError('Post-commit verification failed: '+key)

    def apply(self,folder,m,action):
        self.check_live(m)
        if action!='disable':self.require_game()
        if action=='rollback' and not self.coherent(m['before']):
            raise ValueError('Prior DLL/marker/bank state is not qualified for rollback; use disable and retain backups')
        values=m['after'] if action=='install' else m['before'] if action=='rollback' else dict.fromkeys(KEYS)
        progress={'install':'installing','rollback':'rolling_back','disable':'disabling'}[action]
        durable_json(folder/'journal.json',{'phase':progress,'observed':self.observed()})
        # Keep activation off during any partial operation. Missing marker is an
        # explicit intermediate state, not an all-files atomic transaction claim.
        self.change('marker',None,m)
        for n,key in enumerate(KEYS):
            if key=='marker':continue
            durable_json(folder/'journal.json',{'phase':progress,'next':key,'observed':self.observed()})
            prefix='new' if action=='install' else 'prior'
            data=read_file(folder/(prefix+'-'+str(n)+'.bin')) if values[key] is not None else None
            if (identity(data) if data is not None else None)!=values[key]:
                raise ValueError('Saved replacement changed before use: '+key)
            self.change(key,data,m)
        self.change('marker',MARKER if values['marker'] is not None else None,m)
        if self.observed()!=values:raise ValueError('Final installation verification failed')
        checkpoint('before_verified_journal')
        phase={'install':'verified','rollback':'rolled_back','disable':'disabled'}[action]
        durable_json(folder/'journal.json',{'phase':phase,'observed':values})
        return {'result':phase,'record':m['record'],'recovery':str(folder),'observed':values}

    def mutate(self,action,desired=None,record=None):
        require_closed();no_reparse(self.store)
        with writer_lock(self.game):
            if action=='install':
                self.require_game();folder,m=self.stage(desired)
            else:
                folder,m=self.load_record(record)
                self.check_live(m)
                if record is not None:
                    digest=hashlib.sha256(read_file(folder/'manifest.json',65536)).hexdigest()
                    durable_json(self.store/'ACTIVE.json',{'record':m['record'],'manifest_sha256':digest})
            try:return self.apply(folder,m,'install' if action=='resume' else action)
            except Exception as e:
                # Even when a journal write fails, the immutable backups and
                # selector precede all target writes. Status rehashes actual files.
                try:durable_json(folder/'journal.json',{'phase':'failed','action':action,'error':str(e),'observed':self.observed()})
                except Exception:pass
                raise ValueError('Operation incomplete; bank may already be replaced. Keep NMS closed. Recovery: '+str(folder)+'; '+str(e)) from e

    def status(self,record=None):
        result={'game':str(self.game),'game_supported':self.game_matches(),'recovery_root':str(self.store),'observed':self.observed(),'records':[]}
        records=self.store/'records';no_reparse(records)
        if records.exists():result['records']=[p.name for p in records.iterdir() if re.fullmatch('[0-9a-f]{32}',p.name)]
        try:
            if record is None and self.active() is None:return dict(result,result='no_selected_record')
            folder,m=self.load_record(record)
        except (OSError,ValueError,KeyError,TypeError) as e:return dict(result,result='CORRUPT_RECORD_REFUSED',error=str(e))
        observed=result['observed']
        try:journal=load_json(folder/'journal.json')
        except (OSError,ValueError) as e:journal={'phase':'UNREADABLE','error':str(e)}
        result.update(record=m['record'],journal=journal,rollback_permitted=self.coherent(m['before']) and result['game_supported'],
                      files={k:'absent' if observed[k] is None else 'candidate' if observed[k]==m['after'][k] else 'prior' if observed[k]==m['before'][k] else 'UNKNOWN' for k in KEYS})
        result['result']='UNKNOWN_CURRENT_REFUSED' if 'UNKNOWN' in result['files'].values() else 'candidate_present' if observed==m['after'] else 'prior_present' if observed==m['before'] else 'partial_or_disabled'
        scratch=[]
        for key in KEYS:
            parent=self.target(key).parent;no_reparse(parent)
            if parent.exists():
                for p in parent.glob('.supermoon-'+m['record']+'-'+key+'-*.tmp'):
                    value=current(p);scratch.append({'path':str(p),'identity':value,'matches_candidate':value==m['after'][key]})
        return dict(result,scratch=scratch)


def installed_bank(game,recipe,plan,bank):
    """Compatibility entry: current DLLs/marker must already match, never extracted here."""
    tool=Installation(game,recipe,plan)
    desired={k:read_file(tool.target(k)) for k in ['proxy','payload','marker']}
    if any(v is None or identity(v)!=tool.expected[k] for k,v in desired.items()):
        raise ValueError('Install the exact matching DLL pair and marker with install_supermoon.py first')
    desired['bank']=bank
    return tool.mutate('install',desired)


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('action',choices=['install','resume','status','rollback','disable'])
    p.add_argument('--game',type=Path,required=True);p.add_argument('--package',type=Path);p.add_argument('--record',help='Explicit 32-hex record for inspection/recovery, including an unselected prepared record')
    a=p.parse_args();tool=None
    try:
        recipe=load_json(ROOT/'build/recipe.json');plan=load_json(Path(__file__).with_name('audio_patch.json'))
        tool=Installation(a.game,recipe,plan)
        if a.action=='status':result=tool.status(a.record)
        elif a.action=='install':
            if a.package is None or a.record is not None:raise ValueError('install requires --package, without --record')
            package=a.package.absolute();no_reparse(package);package=package.resolve(strict=True)
            if package==tool.game or package.is_relative_to(tool.game) or tool.game.is_relative_to(package):raise ValueError('Extract the package separately, outside the game tree')
            tool.require_game()
            desired={'proxy':read_file(package/'winmm.dll'),'payload':read_file(package/'Supermoon/supermoon_playlist.dll'),'marker':read_file(package/'bytebeat-playlist-experimental.enabled')}
            if any(v is None or identity(v)!=tool.expected[k] for k,v in desired.items()):raise ValueError('Wrong/missing package DLL pair or marker')
            from prepare_audio import derive
            desired['bank']=derive(tool.game,plan);result=tool.mutate('install',desired)
        else:result=tool.mutate(a.action,record=a.record)
        print(json.dumps(result,indent=2));return 1 if 'REFUSED' in result['result'] else 0
    except (OSError,ValueError,KeyError,TypeError) as e:
        print(json.dumps({'result':'REFUSED_OR_INCOMPLETE','error':str(e),'recovery_root':str(tool.store if tool else a.game/'.Supermoon-Recovery'),'guidance':'Keep NMS closed; use verified tools and run status. Do not overwrite unknown files or discard records.'},indent=2),file=sys.stderr);return 1


if __name__=='__main__':sys.exit(main())
