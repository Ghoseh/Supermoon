# SPDX-License-Identifier: MIT
"""Disposable source-tool reservation fixtures; no game paths are used."""
from pathlib import Path
from unittest import mock
import json,os,subprocess,sys,tempfile,unittest,uuid
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
import common

class Outputs(unittest.TestCase):
    def setUp(self):
        self.case=Path(os.environ['SUPERMOON_RELIABILITY_ROOT'])/'tool-cases'/uuid.uuid4().hex[:8];self.case.mkdir(parents=True)
        self.source=self.case/'workspace/source';self.source.mkdir(parents=True)
        patch=mock.patch.object(common,'ROOT',self.source);patch.start();self.addCleanup(patch.stop)

    def fresh(self,parent,label='owned'):
        parent.mkdir(parents=True,exist_ok=True)
        with mock.patch.object(tempfile,'tempdir',str(parent)):return common.fresh(label)

    def test_safe_temp_ancestor_reserves_sibling(self):
        output=self.fresh(self.source.parent)
        self.assertEqual(output.parent,self.source.parent);self.assertFalse(output.is_relative_to(self.source))
        self.assertTrue((output/'OWNED_DISPOSABLE_OUTPUT.txt').is_file());self.assertEqual(list(self.source.iterdir()),[])

    def test_safe_independent_temp(self):self.assertTrue(self.fresh(self.case/'temp').is_dir())

    def test_nested_source_temp_refused_without_output(self):
        for parent in [self.source,self.source/'nested']:
            parent.mkdir(exist_ok=True)
            before=set(parent.iterdir())
            with self.assertRaises(ValueError):self.fresh(parent)
            self.assertEqual(set(parent.iterdir()),before)

    def test_protected_game_and_review_ancestry(self):
        for name in ['game','review','baselines','payload']:
            parent=self.case/name;parent.mkdir()
            if name=='game':(parent/'NMS.exe').write_bytes(b'OWNED SENTINEL')
            if name=='payload':(parent/'supermoon_playlist.dll').write_bytes(b'OWNED SENTINEL')
            with self.assertRaises(ValueError):self.fresh(parent/'temp')

    def test_reparse_temp_refused_before_resolve_hides_it(self):
        target=self.case/'outside';target.mkdir();link=self.case/'junction'
        r=subprocess.run(['cmd.exe','/d','/c','mklink','/J',str(link),str(target)],capture_output=True,text=True)
        self.assertEqual(r.returncode,0,r.stderr)
        with self.assertRaises(ValueError):self.fresh(link)
        self.assertEqual(list(target.iterdir()),[])

    def test_reserved_output_checked_and_bad_labels_refused(self):
        parent=self.case/'temp';parent.mkdir()
        for label in ['../escape','x/y','x\\y','']:
            with self.assertRaises(ValueError):self.fresh(parent,label)
        self.assertEqual(list(parent.iterdir()),[])
        for returned in [self.source,self.source.parent]:
            with mock.patch.object(tempfile,'mkdtemp',return_value=str(returned)):
                with self.assertRaises(ValueError):self.fresh(parent)
            self.assertFalse((returned/'OWNED_DISPOSABLE_OUTPUT.txt').exists())

if __name__=='__main__':unittest.main(verbosity=2)
