# SPDX-License-Identifier: MIT
"""Owned deterministic fixtures. Never install into or launch the real game."""
from pathlib import Path
from unittest import mock
import contextlib,ctypes,io,json,os,queue,shutil,struct,subprocess,sys,tempfile,threading,unittest,uuid,zipfile

ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
import install_supermoon as inst
import installation_safety as safety
import package_installation as packaging
import prepare_audio as audio


class Fixture:
    def __init__(self,case,slot='game & é'):
        self.case=case;self.game=case/'persistent'/slot;self.game.mkdir(parents=True)
        self.exe=b'OWNED SYNTHETIC EXECUTABLE; NEVER RUN'
        self.desired={'proxy':b'OWNED PROXY NEW','payload':b'OWNED PAYLOAD NEW','bank':b'OWNED BANK NEW','marker':inst.MARKER}
        self.old=b'OWNED PRIOR MENU BANK'
        self.recipe={'version':'owned-test-1','game_executable_bytes':len(self.exe),'game_executable_sha256':inst.identity(self.exe)['sha256'],
                     'DLLs':{'winmm.dll':inst.identity(self.desired['proxy']),'supermoon_playlist.dll':inst.identity(self.desired['payload'])}}
        self.plan={'output_bytes':len(self.desired['bank']),'output_sha256':inst.identity(self.desired['bank'])['sha256']}
        self.put('Binaries/NMS.exe',self.exe);self.put('GAMEDATA/PCBANKS/native-sentinel',b'OWNED ARCHIVE SENTINEL')
        self.personal={
            'Binaries/Supermoon/playlist.ini':b'[Playlist]\nMusic=D:\\Personal Music\nCacheMiB=16\n[Normalization]\nEnabled=1\n[Keyboard]\nPlay=F9\n',
            'Binaries/Supermoon/Music/personal.wav':b'OWNED PERSONAL MUSIC',
            'Binaries/Supermoon/Cache/normalization.pcm':b'OWNED CACHE',
            'Binaries/Supermoon/State/order.json':b'OWNED STATE',
            'Binaries/Supermoon/unrelated-user.txt':b'KEEP USER FILE',
            'GAMEDATA/MODS/OtherMod/audio/windows/other.bnk':b'UNRELATED MOD',
            'Binaries/SETTINGS/GCMODSETTINGS.MXML':b'OWNED SETTINGS SENTINEL',
            'Binaries/unrelated.dll':b'UNRELATED DLL'}
        for rel,data in self.personal.items():self.put(rel,data)
        self.tool=inst.Installation(self.game,self.recipe,self.plan)

    def put(self,rel,data):
        path=self.game/rel;path.parent.mkdir(parents=True,exist_ok=True);path.write_bytes(data)

    def baseline(self,bank=True):
        for key,data in self.desired.items():
            if key!='bank' or bank:self.put(inst.PATHS[key],data)

    def metadata(self):return {'game':str(self.game),'recipe':self.recipe,'plan':self.plan,'desired':{k:v.hex() for k,v in self.desired.items()}}


def child():
    # Child-only fault injection, never a production CLI flag or environment seam.
    data=json.loads(Path(sys.argv[2]).read_text());game=Path(data['game']);stage=sys.argv[3]
    tempfile.tempdir=str(game.parents[1]/'TEMP');Path(tempfile.tempdir).mkdir(exist_ok=True)
    inst.require_closed=lambda:None
    def pause(at):
        if at==stage:
            print('OWNED CRASH CHECKPOINT '+at,flush=True);threading.Event().wait()
    inst.checkpoint=pause
    tool=inst.Installation(game,data['recipe'],data['plan'])
    tool.mutate('install',{k:bytes.fromhex(v) for k,v in data['desired'].items()})


class InstallationTests(unittest.TestCase):
    def setUp(self):
        work=Path(os.environ['SUPERMOON_INSTALL_TEST_ROOT']);self.case=work/'cases'/uuid.uuid4().hex[:8];self.case.mkdir(parents=True)
        self.ephemeral=self.case/'TEMP';self.ephemeral.mkdir()
        self.temp=mock.patch.object(tempfile,'tempdir',str(self.ephemeral));self.temp.start();self.addCleanup(self.temp.stop)
        self.closed=mock.patch.object(inst,'require_closed',return_value=None);self.closed.start();self.addCleanup(self.closed.stop)
        self.old_pin=mock.patch.object(inst,'MENU_SHA',inst.identity(b'OWNED PRIOR MENU BANK')['sha256']);self.old_pin.start();self.addCleanup(self.old_pin.stop)
        self.f=Fixture(self.case)
        (self.case/'case.json').write_text(json.dumps({'test':self.id(),**self.f.metadata()},indent=2))

    def intact(self,f=None):
        f=f or self.f
        for rel,data in f.personal.items():self.assertEqual((f.game/rel).read_bytes(),data,rel)
        self.assertEqual((f.game/'Binaries/NMS.exe').read_bytes(),f.exe)
        self.assertEqual((f.game/'GAMEDATA/PCBANKS/native-sentinel').read_bytes(),b'OWNED ARCHIVE SENTINEL')

    def install(self,f=None):
        f=f or self.f;result=f.tool.mutate('install',f.desired);self.intact(f);return result

    def test_clean_install_uses_defaults_without_creating_ini(self):
        p=self.f.game/'Binaries/Supermoon/playlist.ini';p.unlink();del self.f.personal['Binaries/Supermoon/playlist.ini']
        self.assertEqual(self.install()['result'],'verified');self.assertFalse(p.exists())
        self.assertEqual(self.f.tool.observed(),self.f.tool.expected)

    def test_update_preserves_all_personal_data_and_first_prior_state(self):
        self.f.baseline(bank=False);self.f.put(inst.PATHS['bank'],self.f.old)
        first=self.install();folder,m=self.f.tool.load_record();before=(folder/'manifest.json').read_bytes()
        self.assertEqual((folder/'prior-2.bin').read_bytes(),self.f.old)
        second=self.install();self.assertEqual(first['record'],second['record']);self.assertEqual((folder/'manifest.json').read_bytes(),before)
        self.assertEqual(m['before']['bank'],inst.identity(self.f.old));self.intact()

    def test_current_install_gets_verified_durable_record(self):
        self.f.baseline();a=self.install();folder,m=self.f.tool.load_record()
        self.assertEqual(m['before'],m['after']);self.assertEqual(self.f.tool.status()['journal']['phase'],'verified')
        self.assertTrue(Path(a['recovery']).is_relative_to(self.f.game))

    def test_temp_cleanup_cannot_remove_backups_and_resume_needs_no_package(self):
        self.f.baseline(bank=False);a=self.install();folder,m=self.f.tool.load_record();pin=inst.identity((folder/'manifest.json').read_bytes())
        (self.ephemeral/'download.zip').write_bytes(b'DISCARDABLE');target=self.ephemeral.resolve()
        self.assertTrue(target.is_relative_to(self.case.resolve()) and target.name=='TEMP');shutil.rmtree(target)
        self.ephemeral.mkdir();self.f.tool.mutate('resume');self.assertEqual(inst.identity((folder/'manifest.json').read_bytes()),pin)
        self.assertEqual(self.f.tool.load_record()[1]['before'],m['before']);self.intact()

    def test_clean_rollback_retry_and_reinstall(self):
        first=self.install();self.assertEqual(self.f.tool.mutate('rollback')['result'],'rolled_back')
        self.assertTrue(all(v is None for v in self.f.tool.observed().values()));self.intact()
        self.f.tool.mutate('rollback');second=self.install();self.assertNotEqual(first['record'],second['record'])
        self.assertEqual(len(self.f.tool.status()['records']),2)

    def test_rollback_restores_exact_known_prior_pair_and_bank_absence(self):
        self.f.baseline(bank=False);before=self.f.tool.observed();self.install();self.f.tool.mutate('rollback')
        self.assertEqual(self.f.tool.observed(),before);self.intact()

    def test_unqualified_prior_bank_cannot_be_reactivated(self):
        self.f.baseline(bank=False);self.f.put(inst.PATHS['bank'],self.f.old);self.install()
        with self.assertRaisesRegex(ValueError,'not qualified'):self.f.tool.mutate('rollback')
        self.f.tool.mutate('disable');self.assertTrue(all(v is None for v in self.f.tool.observed().values()));self.intact()
        folder,_=self.f.tool.load_record();self.assertEqual((folder/'prior-2.bin').read_bytes(),self.f.old)

    def test_unknown_program_bank_and_marker_refused_without_replacement(self):
        for key in inst.KEYS:
            with self.subTest(key=key):
                f=Fixture(self.case,key);f.put(inst.PATHS[key],b'FOREIGN '+key.encode());before=f.tool.observed()
                with self.assertRaisesRegex(ValueError,'Unknown installed'):self.install(f)
                self.assertEqual(f.tool.observed(),before);self.intact(f)

    def test_corrupt_candidate_or_missing_payload_refused(self):
        for key in inst.KEYS:
            desired=dict(self.f.desired);desired[key]=b'CORRUPTED'
            with self.subTest(key=key),self.assertRaisesRegex(ValueError,'Candidate'):self.f.tool.mutate('install',desired)
        desired=dict(self.f.desired);del desired['payload']
        with self.assertRaises(ValueError):self.f.tool.mutate('install',desired)
        self.assertTrue(all(v is None for v in self.f.tool.observed().values()));self.intact()

    def test_wrong_game_refused_and_old_records_allow_disable_only(self):
        self.install();self.f.put('Binaries/NMS.exe',b'UNSUPPORTED UPDATED GAME')
        for action in ['resume','rollback']:
            with self.subTest(action=action),self.assertRaisesRegex(ValueError,'Unsupported NMS'):self.f.tool.mutate(action)
        self.f.tool.mutate('disable');self.assertTrue(all(v is None for v in self.f.tool.observed().values()))
        self.assertEqual((self.f.game/'Binaries/NMS.exe').read_bytes(),b'UNSUPPORTED UPDATED GAME')

    def test_unknown_current_files_block_all_recovery_before_any_write(self):
        for key in inst.KEYS:
            f=Fixture(self.case,key);self.install(f);f.put(inst.PATHS[key],b'INTERVENING FOREIGN DATA');before=f.tool.observed()
            for action in ['resume','rollback','disable']:
                with self.subTest(key=key,action=action),self.assertRaisesRegex(ValueError,'Unknown current'):f.tool.mutate(action)
                self.assertEqual(f.tool.observed(),before)
            self.assertEqual(f.tool.status()['result'],'UNKNOWN_CURRENT_REFUSED');self.intact(f)

    def test_corrupt_backup_or_manifest_refuses_recovery(self):
        for name in ['new-2.bin','manifest.json','manifest.sha256']:
            f=Fixture(self.case,name);self.install(f);folder,_=f.tool.load_record();(folder/name).write_bytes(b'CORRUPTED');before=f.tool.observed()
            for action in ['resume','rollback','disable']:
                with self.subTest(name=name,action=action),self.assertRaises(ValueError):f.tool.mutate(action)
            self.assertEqual(f.tool.observed(),before);self.assertEqual(f.tool.status()['result'],'CORRUPT_RECORD_REFUSED')

    def test_corrupt_prior_backup_refused(self):
        self.f.baseline();self.install();folder,_=self.f.tool.load_record();(folder/'prior-0.bin').write_bytes(b'BROKEN')
        with self.assertRaisesRegex(ValueError,'backup corrupt'):self.f.tool.mutate('rollback')

    def test_saved_replacement_is_revalidated_immediately_before_use(self):
        self.f.baseline(bank=False);self.f.put(inst.PATHS['bank'],self.f.old)
        def corrupt(stage):
            if stage=='after_remove:marker':
                folder,_=self.f.tool.load_record();(folder/'new-2.bin').write_bytes(b'OWNED LATE BACKUP CORRUPTION')
        with mock.patch.object(inst,'checkpoint',side_effect=corrupt),self.assertRaisesRegex(ValueError,'Saved replacement changed'):self.install()
        self.assertEqual(self.f.tool.target('bank').read_bytes(),self.f.old)
        self.assertEqual(self.f.tool.status()['result'],'CORRUPT_RECORD_REFUSED');self.intact()

    def test_unreadable_journal_status_and_resume_reconstruct_actual_state(self):
        self.install();folder,_=self.f.tool.load_record();(folder/'journal.json').write_bytes(b'{bad json')
        self.assertEqual(self.f.tool.status()['journal']['phase'],'UNREADABLE')
        self.assertEqual(self.f.tool.mutate('resume')['result'],'verified');self.intact()

    def test_selector_corruption_requires_explicit_verified_record(self):
        first=self.install();(self.f.tool.store/'ACTIVE.json').write_bytes(b'{}')
        with self.assertRaises(ValueError):self.f.tool.mutate('resume')
        self.assertEqual(self.f.tool.status()['result'],'CORRUPT_RECORD_REFUSED')
        self.f.tool.mutate('rollback',record=first['record']);self.assertTrue(all(v is None for v in self.f.tool.observed().values()))
        with self.assertRaises(ValueError):self.f.tool.mutate('disable',record='../escape')

    def test_record_is_bound_to_its_original_game_path(self):
        self.install();other=Fixture(self.case,'other-game');shutil.copytree(self.f.tool.store,other.tool.store)
        with self.assertRaisesRegex(ValueError,'target/schema mismatch'):other.tool.mutate('resume')
        self.assertTrue(all(v is None for v in other.tool.observed().values()));self.intact(other)

    def test_duplicate_record_keys_and_broken_tool_policy_refuse(self):
        self.install();selector=self.f.tool.store/'ACTIVE.json';a=selector.read_text()
        selector.write_text(a.replace('{','{"record":"'+('0'*32)+'",',1))
        with self.assertRaisesRegex(ValueError,'Duplicate'):self.f.tool.mutate('resume')
        bad=self.case/'broken-tools';(bad/'build').mkdir(parents=True);(bad/'build/recipe.json').write_bytes(b'{bad json')
        error=io.StringIO()
        with mock.patch.object(inst,'ROOT',bad),mock.patch.object(sys,'argv',['install_supermoon.py','status','--game',str(self.f.game)]),contextlib.redirect_stderr(error):
            self.assertEqual(inst.main(),1)
        message=json.loads(error.getvalue());self.assertEqual(message['result'],'REFUSED_OR_INCOMPLETE');self.assertIn('verified tools',message['guidance']);self.intact()

    def test_status_is_read_only(self):
        def snapshot():return {p.relative_to(self.f.game).as_posix():p.read_bytes() for p in self.f.game.rglob('*') if p.is_file()}
        before=snapshot();self.assertEqual(self.f.tool.status()['result'],'no_selected_record');self.assertEqual(snapshot(),before)
        self.install();before=snapshot();self.f.tool.status();self.assertEqual(snapshot(),before)

    def test_temp_recovery_placement_refused(self):
        with mock.patch.object(tempfile,'tempdir',str(self.f.game.parent)):
            with self.assertRaisesRegex(ValueError,'outside TEMP'):self.install()

    def test_backup_failure_leaves_targets_and_unselected_evidence(self):
        def fail(stage):
            if stage=='before_backup:bank':raise PermissionError('OWNED backup failure')
        with mock.patch.object(inst,'checkpoint',side_effect=fail),self.assertRaises(PermissionError):self.install()
        self.assertTrue(all(v is None for v in self.f.tool.observed().values()))
        s=self.f.tool.status();self.assertEqual(s['result'],'no_selected_record');self.assertEqual(len(s['records']),1);self.intact()

    def test_receipt_write_failure_does_not_commit_targets(self):
        real=inst.durable_json
        def fail(path,value):
            if path.name=='ACTIVE.json':raise PermissionError('OWNED selector write failure')
            return real(path,value)
        with mock.patch.object(inst,'durable_json',side_effect=fail),self.assertRaises(PermissionError):self.install()
        self.assertTrue(all(v is None for v in self.f.tool.observed().values()))
        name=self.f.tool.status()['records'][0];self.assertEqual(self.f.tool.status(name)['journal']['phase'],'prepared')
        self.f.tool.mutate('resume',record=name);self.intact()

    def test_replace_failure_and_retry_keep_original_backup(self):
        self.f.baseline(bank=False);self.f.put(inst.PATHS['bank'],self.f.old);real=inst.os.replace
        def fail(src,dst):
            if Path(dst)==self.f.tool.target('bank'):raise PermissionError('OWNED replace sharing failure')
            return real(src,dst)
        with mock.patch.object(inst.os,'replace',side_effect=fail),self.assertRaisesRegex(ValueError,'bank may already'):self.install()
        folder,m=self.f.tool.load_record();self.assertEqual(self.f.tool.status()['files']['bank'],'prior')
        self.f.tool.mutate('resume');self.assertEqual(self.f.tool.load_record()[0],folder)
        self.assertEqual((folder/'prior-2.bin').read_bytes(),self.f.old);self.assertEqual(m['before']['bank'],inst.identity(self.f.old));self.intact()

    def test_post_commit_failure_reports_actual_new_bank(self):
        def fail(stage):
            if stage=='after_replace:bank':raise OSError('OWNED verification failure after commit')
        with mock.patch.object(inst,'checkpoint',side_effect=fail),self.assertRaisesRegex(ValueError,'bank may already'):self.install()
        status=self.f.tool.status();self.assertEqual(status['files']['bank'],'candidate');self.assertEqual(status['journal']['phase'],'failed')
        folder,m=self.f.tool.load_record();self.assertIsNone(m['before']['bank']);self.f.tool.mutate('resume')
        self.assertEqual(self.f.tool.load_record()[0],folder);self.intact()

    def test_post_commit_corruption_is_diagnosed_and_never_overwritten(self):
        def corrupt(stage):
            if stage=='after_replace:bank':self.f.tool.target('bank').write_bytes(b'OWNED POST-COMMIT CORRUPTION')
        with mock.patch.object(inst,'checkpoint',side_effect=corrupt),self.assertRaisesRegex(ValueError,'Post-commit verification'):self.install()
        self.assertEqual(self.f.tool.status()['result'],'UNKNOWN_CURRENT_REFUSED')
        with self.assertRaisesRegex(ValueError,'Unknown current'):self.f.tool.mutate('resume')
        bad=self.f.tool.target('bank').read_bytes();(self.case/'preserved-corrupt-bank.bin').write_bytes(bad)
        # Explicit operator quarantine in an owned fixture, not an automatic tool overwrite.
        self.f.tool.target('bank').unlink();self.f.tool.mutate('resume')
        self.assertEqual((self.case/'preserved-corrupt-bank.bin').read_bytes(),bad);self.intact()

    def test_final_journal_failure_never_claims_verified(self):
        real=inst.durable_json
        def fail(path,value):
            if value.get('phase')=='verified':raise PermissionError('OWNED final receipt failure')
            return real(path,value)
        with mock.patch.object(inst,'durable_json',side_effect=fail),self.assertRaises(ValueError):self.install()
        self.assertEqual(self.f.tool.status()['result'],'candidate_present');self.assertNotEqual(self.f.tool.status()['journal']['phase'],'verified')
        self.f.tool.mutate('resume');self.intact()

    def test_scratch_write_failure_retains_backup_and_all_personal_data(self):
        self.f.baseline(bank=False);self.f.put(inst.PATHS['bank'],self.f.old);real=inst.tempfile.mkstemp
        def fail(*args,**kwargs):
            if kwargs.get('prefix','').startswith('.supermoon-'):raise PermissionError('OWNED scratch write failure')
            return real(*args,**kwargs)
        with mock.patch.object(inst.tempfile,'mkstemp',side_effect=fail),self.assertRaises(ValueError):self.install()
        self.assertEqual(self.f.tool.target('bank').read_bytes(),self.f.old);self.f.tool.load_record();self.intact()

    def test_real_windows_sharing_violation_is_recoverable(self):
        self.f.baseline(bank=False);self.f.put(inst.PATHS['bank'],self.f.old)
        from ctypes import wintypes as w
        api=ctypes.WinDLL('kernel32',use_last_error=True);api.CreateFileW.argtypes=[w.LPCWSTR,w.DWORD,w.DWORD,w.LPVOID,w.DWORD,w.DWORD,w.HANDLE];api.CreateFileW.restype=w.HANDLE;api.CloseHandle.argtypes=[w.HANDLE]
        h=api.CreateFileW(str(self.f.tool.target('bank')),0x80000000,1,None,3,0x80,None);self.assertNotEqual(h,ctypes.c_void_p(-1).value)
        try:
            with self.assertRaisesRegex(ValueError,'bank may already'):self.install()
        finally:api.CloseHandle(h)
        self.assertEqual(self.f.tool.target('bank').read_bytes(),self.f.old);self.f.tool.mutate('resume');self.intact()

    def test_cooperating_writers_are_serialized_and_unknown_lock_preserved(self):
        with safety.writer_lock(self.f.game):
            with self.assertRaises(OSError):self.install()
        path=self.f.game/'.supermoon-install.lock';path.write_bytes(b'FOREIGN LOCK CONTENT')
        with self.assertRaisesRegex(ValueError,'Unknown installation lock'):self.install()
        self.assertEqual(path.read_bytes(),b'FOREIGN LOCK CONTENT');self.intact()

    def test_preexisting_junction_and_late_precheck_changes_refused(self):
        outside=self.case/'outside';outside.mkdir();(outside/'sentinel').write_bytes(b'OUTSIDE')
        parent=self.f.tool.target('bank').parent;parent.parent.mkdir(parents=True)
        # mklink is cmd's documented builtin; fixed owned paths, no path enumeration/deletion across shells.
        r=subprocess.run(['cmd.exe','/d','/c','mklink','/J',str(parent),str(outside)],capture_output=True,text=True)
        self.assertEqual(r.returncode,0,r.stderr)
        with self.assertRaisesRegex(ValueError,'Reparse'):self.install()
        self.assertEqual((outside/'sentinel').read_bytes(),b'OUTSIDE');self.assertFalse((outside/'init.bnk').exists())

    def test_path_switch_after_preparation_is_rechecked(self):
        outside=self.case/'outside';outside.mkdir();(outside/'sentinel').write_bytes(b'OUTSIDE')
        parent=self.f.tool.target('bank').parent;parent.mkdir(parents=True)
        def switch(stage):
            if stage=='prepared':
                moved=parent.with_name('windows-owned-original')
                self.assertTrue(parent.resolve().is_relative_to(self.case.resolve()))
                self.assertTrue(moved.resolve().is_relative_to(self.case.resolve()))
                parent.rename(moved)
                r=subprocess.run(['cmd.exe','/d','/c','mklink','/J',str(parent),str(outside)],capture_output=True,text=True)
                self.assertEqual(r.returncode,0,r.stderr)
        with mock.patch.object(inst,'checkpoint',side_effect=switch),self.assertRaisesRegex(ValueError,'Reparse'):self.install()
        self.assertEqual((outside/'sentinel').read_bytes(),b'OUTSIDE');self.assertFalse((outside/'init.bnk').exists())

    def test_destination_change_in_closed_guard_is_refused(self):
        self.f.baseline(bank=False);self.f.put(inst.PATHS['bank'],self.f.old)
        def change():
            if self.f.tool.active() is not None:self.f.tool.target('bank').write_bytes(b'INTERVENING FOREIGN BANK')
        with mock.patch.object(inst,'require_closed',side_effect=change),self.assertRaises(ValueError):self.install()
        self.assertEqual(self.f.tool.target('bank').read_bytes(),b'INTERVENING FOREIGN BANK');self.intact()

    def test_uncooperative_race_after_final_check_is_explicitly_out_of_contract(self):
        # This deliberately violates the supported exclusive-writer precondition.
        # Characterize the limit honestly rather than claim checks provide a lock.
        def change(stage):
            if stage=='before_replace:bank':self.f.tool.target('bank').write_bytes(b'UNCOOPERATIVE CHANGE')
        with mock.patch.object(inst,'checkpoint',side_effect=change):self.install()
        self.assertEqual(self.f.tool.target('bank').read_bytes(),self.f.desired['bank'])
        (self.case/'UNSUPPORTED_RACE_CHARACTERIZATION.txt').write_text('A mutation after the final comparison can be overwritten. Arbitrary concurrent writers are unsupported; no race-safety claim.\n')

    def test_crash_before_and_after_bank_replace_resume_and_rollback(self):
        for index,stage in enumerate(['prepared','before_replace:bank','after_replace:bank','before_verified_journal','after_remove:marker']):
            f=Fixture(self.case,'crash'+str(index))
            if stage=='after_remove:marker':f.baseline(bank=False)
            path=self.case/('child'+str(index)+'.json');path.write_text(json.dumps(f.metadata()))
            child_=subprocess.Popen([sys.executable,'-B',__file__,'--child',str(path),stage],stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True,creationflags=subprocess.CREATE_NO_WINDOW)
            q=queue.Queue();threading.Thread(target=lambda:q.put(child_.stdout.readline()),daemon=True).start()
            try:self.assertIn('OWNED CRASH CHECKPOINT',q.get(timeout=8))
            finally:
                child_.kill();out,err=child_.communicate(timeout=5)
                (self.case/('crash-'+str(index)+'-receipt.json')).write_text(json.dumps({'pid':child_.pid,'stage':stage,'exit':child_.returncode,'stdout_tail':out,'stderr':err,'owned_child_reaped':child_.poll() is not None},indent=2))
            self.assertEqual(err,'');folder,m=f.tool.load_record()
            if stage!='after_remove:marker':self.assertTrue(all(v is None for v in m['before'].values()))
            if stage in ['after_replace:bank','before_verified_journal']:self.assertEqual(f.tool.status()['files']['bank'],'candidate')
            elif stage=='before_replace:bank':self.assertEqual(len(f.tool.status()['scratch']),1)
            f.tool.mutate('resume');self.assertEqual(f.tool.load_record()[0],folder);f.tool.mutate('rollback')
            self.assertEqual(f.tool.observed(),m['before']);self.intact(f)

    def test_package_zip_merge_has_example_only_and_preserves_live_ini(self):
        dll=self.case/'dlls';dll.mkdir();(dll/'winmm.dll').write_bytes(self.f.desired['proxy']);(dll/'supermoon_playlist.dll').write_bytes(self.f.desired['payload'])
        out=self.case/'candidate.zip';packaging.package(dll,out,self.f.recipe)
        with zipfile.ZipFile(out) as z:
            self.assertNotIn('Supermoon/playlist.ini',z.namelist());self.assertIn('Supermoon/playlist.example.ini',z.namelist())
            z.extractall(self.f.game/'Binaries')
        self.intact();self.assertEqual((self.f.game/'Binaries/Supermoon/playlist.ini').read_bytes(),self.f.personal['Binaries/Supermoon/playlist.ini'])
        result=subprocess.run([sys.executable,'-B',ROOT/'tools/verify.py',out],capture_output=True,text=True)
        self.assertEqual(result.returncode,0,result.stderr)
        with self.assertRaises(FileExistsError):packaging.package(dll,out,self.f.recipe)
        (dll/'winmm.dll').write_bytes(b'CORRUPTED INPUT')
        with self.assertRaises(ValueError):packaging.package(dll,self.case/'bad.zip',self.f.recipe)

    def test_bank_compatibility_entry_requires_matching_pair_and_marker(self):
        with self.assertRaisesRegex(ValueError,'matching DLL pair'):inst.installed_bank(self.f.game,self.f.recipe,self.f.plan,self.f.desired['bank'])
        self.f.baseline(bank=False);self.assertEqual(inst.installed_bank(self.f.game,self.f.recipe,self.f.plan,self.f.desired['bank'])['result'],'verified')
        self.intact()

    def test_audio_derivation_and_corrupt_inputs_keep_native_files(self):
        native=bytearray(64);struct.pack_into('<I',native,4,1234);new=bytearray(native);struct.pack_into('<I',new,4,4321)
        plan={'input_bytes':64,'input_sha256':audio.sha(native),'output_bytes':64,'output_sha256':audio.sha(new),'insertions':[],
              'edits':[{'offset':4,'format':'<I','expected':1234,'value':4321}]}
        pak=bytearray(512);pak[:5]=b'HGPAK';struct.pack_into('<QQQ?7xQ',pak,8,2,2,1,False,112)
        struct.pack_into('<16sQQ',pak,48,b'owned-init',128,64);struct.pack_into('<16sQQ',pak,80,b'owned-other',256,8);pak[128:192]=native
        p=self.f.game/'GAMEDATA/PCBANKS/NMSARC.audioBNK.pak';p.write_bytes(pak)
        with mock.patch.multiple(audio,PAK_BYTES=len(pak),PAK_SHA=audio.sha(pak),NATIVE_OFFSET=128):
            self.assertEqual(audio.derive(self.f.game,plan),bytes(new));self.assertEqual(p.read_bytes(),pak)
            for bad in [b'TRUNCATED',bytes([pak[0]^1])+pak[1:]]:
                p.write_bytes(bad)
                with self.assertRaises(ValueError):audio.derive(self.f.game,plan)
        with self.assertRaises(ValueError):audio.patch_bank(b'BAD NATIVE',plan)
        bad=dict(plan);bad['output_sha256']='0'*64
        with self.assertRaises(ValueError):audio.patch_bank(native,bad)

    def test_process_guard_real_snapshot_and_benign_tasklist_cannot_run(self):
        names=safety.process_names();self.assertTrue(names);self.assertIn(Path(sys.executable).name.casefold(),[n.casefold() for n in names])
        helper=Path(os.environ['SUPERMOON_INSTALL_PROCESS_FIXTURE']);cwd=self.case/'process';cwd.mkdir()
        shutil.copy2(helper,cwd/'NMS.exe');shutil.copy2(helper,cwd/'tasklist.exe')
        child_=subprocess.Popen([cwd/'NMS.exe'],cwd=cwd,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True,creationflags=subprocess.CREATE_NO_WINDOW)
        q=queue.Queue();threading.Thread(target=lambda:q.put(child_.stdout.readline()),daemon=True).start()
        try:
            self.assertIn('OWNED PROCESS FIXTURE READY',q.get(timeout=5))
            env=os.environ.copy();env['PATH']=str(cwd)+os.pathsep+env['PATH']
            r=subprocess.run([sys.executable,'-B','-c','import sys;sys.path.insert(0,sys.argv[1]);from installation_safety import require_closed;require_closed()',str(ROOT/'tools')],cwd=cwd,env=env,capture_output=True,text=True,timeout=8)
            self.assertNotEqual(r.returncode,0);self.assertIn('Exit NMS fully',r.stderr);self.assertFalse((cwd/'HIJACK_EXECUTED').exists())
        finally:
            child_.kill();stdout,stderr=child_.communicate(timeout=5)
            (self.case/'process-receipt.json').write_text(json.dumps({'pid':child_.pid,'binary_sha256':inst.identity((cwd/'NMS.exe').read_bytes())['sha256'],'owned_child_reaped':child_.poll() is not None,'stdout_tail':stdout,'stderr':stderr,'guard_returncode':r.returncode if 'r' in locals() else None},indent=2))

    def test_process_snapshot_errors_and_guard_indeterminate_fail_closed(self):
        class Function:
            def __init__(self,f):self.f=f
            def __call__(self,*a):return self.f(*a)
        class API:
            def __init__(self,snapshot,first,next_):
                self.closed=False
                self.CreateToolhelp32Snapshot=Function(lambda *_:snapshot)
                self.Process32FirstW=Function(lambda _,p:(setattr(p._obj,'szExeFile','owned.exe'),first)[1])
                self.Process32NextW=Function(lambda *_:next_)
                self.CloseHandle=Function(lambda *_:setattr(self,'closed',True))
        for snapshot,first,next_,error in [(ctypes.c_void_p(-1).value,False,False,5),(1,False,False,5),(1,False,False,18),(1,True,False,5),(1,True,True,18)]:
            api=API(snapshot,first,next_)
            with self.subTest(snapshot=snapshot,first=first,next_=next_,error=error),mock.patch.object(ctypes,'WinDLL',return_value=api),mock.patch.object(ctypes,'get_last_error',return_value=error):
                with self.assertRaises((OSError,ValueError)):safety.process_names()
                if snapshot==1:self.assertTrue(api.closed)
        for names in [[],[''],['nMs.ExE']]:
            with mock.patch.object(safety,'process_names',return_value=names),self.assertRaises(ValueError):safety.require_closed()
        with mock.patch.object(safety,'process_names',side_effect=OSError('OWNED API failure')),self.assertRaises(OSError):safety.require_closed()


if __name__=='__main__':
    if len(sys.argv)>1 and sys.argv[1]=='--child':child()
    else:unittest.main(verbosity=2)
