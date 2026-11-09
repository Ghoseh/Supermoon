# SPDX-License-Identifier: MIT
"""Owned manifest fixtures: Git metadata is excluded; unlisted payload refuses."""
from pathlib import Path
import hashlib,json,subprocess,sys,tempfile,unittest,zipfile
VERIFY=Path(__file__).resolve().parents[1]/'tools/verify.py'

class ManifestVerification(unittest.TestCase):
    def setUp(self):
        self.root=Path(tempfile.mkdtemp(prefix='Supermoon-manifest-tests-'))
        self.root.joinpath('owned.txt').write_bytes(b'OWNED FIXTURE')
        self.rows={'owned.txt':{'bytes':13,'sha256':hashlib.sha256(b'OWNED FIXTURE').hexdigest()}}
        self.pin()
    def pin(self,dirs=None):
        value={'files':self.rows}
        if dirs is not None:value['directories']=dirs
        raw=(json.dumps(value)+'\n').encode();self.root.joinpath('PUBLIC_FILE_MANIFEST.json').write_bytes(raw)
        self.root.joinpath('SHA256SUMS').write_bytes((''.join(v['sha256']+'  '+n+'\n' for n,v in sorted(self.rows.items()))+hashlib.sha256(raw).hexdigest()+'  PUBLIC_FILE_MANIFEST.json\n').encode())
    def run_verify(self,target=None):
        return subprocess.run([sys.executable,'-B',str(VERIFY),str(target or self.root)],capture_output=True)
    def test_root_git_metadata_does_not_change_payload_identity(self):
        self.root.joinpath('.git').write_text('gitdir: metadata-outside-payload\n')
        r=self.run_verify();self.assertEqual(r.returncode,0,r.stderr)
    def test_unlisted_payload_still_refuses(self):
        self.root.joinpath('unlisted.txt').write_text('unexpected')
        self.assertNotEqual(self.run_verify().returncode,0)
    def test_empty_zip_directory_must_match_manifest(self):
        zpath=self.root.parent/(self.root.name+'-fixture.zip')
        with zipfile.ZipFile(zpath,'x') as z:
            for p in self.root.iterdir():z.write(p,p.name)
            z.writestr('Music/',b'')
        self.assertNotEqual(self.run_verify(zpath).returncode,0)
        self.pin(['Music/'])
        zpath2=self.root.parent/(self.root.name+'-allowed.zip')
        with zipfile.ZipFile(zpath2,'x') as z:
            for p in self.root.iterdir():z.write(p,p.name)
            z.writestr('Music/',b'')
        r=self.run_verify(zpath2);self.assertEqual(r.returncode,0,r.stderr)

if __name__=='__main__':unittest.main()
