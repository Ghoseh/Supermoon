# SPDX-FileCopyrightText: 2026 Supermoon contributors
# SPDX-License-Identifier: MIT
"""Generate Supermoon's audio override from the supported owner's game bank."""
from pathlib import Path
import argparse,csv,datetime,hashlib,io,json,os,struct,subprocess,sys,tempfile

ROOT=Path(__file__).resolve().parents[1]
PAK_BYTES=67842512
PAK_SHA='29682a3015f0ffcfa1ac236e4d49a9670e6d2a79e50a25315f8faecf45cde940'
NATIVE_OFFSET=2301520
MENU_SHA='0db1fbc668384e7fb618009f09e889dd32b6b3b3b0b9806f5d099ca8b8ecfc46'
BANK_REL=Path('GAMEDATA/MODS/Supermoon_MenuAudio_Test/audio/windows/init.bnk')

def sha(data):return hashlib.sha256(data).hexdigest()

def patch_bank(native,plan):
    if len(native)!=plan['input_bytes'] or sha(native)!=plan['input_sha256']:
        raise ValueError('Unsupported native init bank; leave the game files unchanged.')
    changed=bytearray(native)
    for edit in plan['edits']:
        at=edit['offset'];fmt=edit['format'];size=struct.calcsize(fmt)
        if fmt not in ('<I','<H','<f') or not 0<=at<=len(native)-size:
            raise ValueError('Invalid audio patch operation')
        if struct.unpack_from(fmt,native,at)[0]!=edit['expected']:
            raise ValueError('Audio patch precondition failed')
        struct.pack_into(fmt,changed,at,edit['value'])
    positions=set()
    for insertion in sorted(plan['insertions'],key=lambda r:r['at'],reverse=True):
        at,start,n=insertion['at'],insertion['source_offset'],insertion['bytes']
        if at in positions or not 0<=at<=len(native) or not 0<=start<=len(native)-n:
            raise ValueError('Invalid audio template range')
        positions.add(at);payload=bytearray(native[start:start+n])
        for offset,value in insertion['replace_u32'].items():
            offset=int(offset)
            if not 0<=offset<=n-4:raise ValueError('Invalid template field')
            struct.pack_into('<I',payload,offset,value)
        changed[at:at]=payload
    result=bytes(changed)
    if len(result)!=plan['output_bytes'] or sha(result)!=plan['output_sha256']:
        raise ValueError('Generated audio bank failed its exact identity check')
    return result

def derive(game,plan):
    pak=game/'GAMEDATA/PCBANKS/NMSARC.audioBNK.pak'
    if pak.stat().st_size!=PAK_BYTES:raise ValueError('Unsupported native audio archive')
    data=pak.read_bytes()
    if sha(data)!=PAK_SHA or data[:5]!=b'HGPAK':raise ValueError('Unsupported native audio archive')
    # This exact archive has one uncompressed init entry at the pinned range.
    # No manifest paths are used as filesystem destinations and no native file
    # is modified. The inner-bank hash independently verifies that range.
    version,count,chunks,compressed,start=struct.unpack_from('<QQQ?7xQ',data,8)
    if version!=2 or compressed or not 1<count<10000 or start<48+32*count:
        raise ValueError('Unsupported native audio archive layout')
    matches=[struct.unpack_from('<16sQQ',data,48+32*i) for i in range(count)]
    if len([r for r in matches if r[1:]==(NATIVE_OFFSET,plan['input_bytes'])])!=1:
        raise ValueError('Native init entry identity mismatch')
    return patch_bank(data[NATIVE_OFFSET:NATIVE_OFFSET+plan['input_bytes']],plan)

def require_closed():
    if os.name!='nt':raise ValueError('Installation requires Windows; generation can run elsewhere.')
    r=subprocess.run(['tasklist','/FI','IMAGENAME eq NMS.exe','/FO','CSV','/NH'],capture_output=True,text=True,check=True,creationflags=subprocess.CREATE_NO_WINDOW)
    if any(row and row[0].lower()=='nms.exe' for row in csv.reader(io.StringIO(r.stdout))):
        raise ValueError('Exit NMS fully before installing the audio override.')

def no_reparse(path):
    for p in (path,*path.parents):
        if p.exists() and (p.is_symlink() or getattr(p.lstat(),'st_file_attributes',0)&0x400):
            raise ValueError('Reparse-point installation paths are unsupported')

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--game',type=Path,required=True)
    p.add_argument('--install',action='store_true',help='Install the locally generated bank while NMS is closed')
    args=p.parse_args();game=args.game.resolve(strict=True);no_reparse(game)
    recipe=json.loads((ROOT/'build/recipe.json').read_text());plan=json.loads((Path(__file__).with_name('audio_patch.json')).read_text())
    exe=game/'Binaries/NMS.exe'
    if exe.stat().st_size!=recipe['game_executable_bytes'] or sha(exe.read_bytes())!=recipe['game_executable_sha256']:
        raise ValueError('Unsupported NMS executable; no files changed.')
    bank=derive(game,plan)
    report={'status':'PASS_GENERATED_IN_MEMORY_NO_GAME_WRITES','bytes':len(bank),'sha256':sha(bank)}
    if args.install:
        require_closed();target=game/BANK_REL;no_reparse(target)
        # The DLL and bank are one qualified unit. Never install the bank next
        # to an older or unrelated mod pair.
        for name,rel in [('winmm.dll','Binaries/winmm.dll'),('supermoon_playlist.dll','Binaries/Supermoon/supermoon_playlist.dll')]:
            data=(game/rel).read_bytes();identity=recipe['DLLs'][name]
            if len(data)!=identity['bytes'] or sha(data)!=identity['sha256']:
                raise ValueError('Install the matching Supermoon DLL pair before its audio override.')
        existing=target.read_bytes() if target.exists() else None
        if existing is not None and sha(existing) not in (MENU_SHA,plan['output_sha256']):
            raise ValueError('Conflicting audio override; preserve it and resolve the conflict before installing.')
        if existing==bank:
            report['status']='PASS_AUDIO_OVERRIDE_ALREADY_CURRENT'
        else:
            backup=Path(tempfile.mkdtemp(prefix='Supermoon-audio-backup-'));receipt={'target':str(target),'prior_exists':existing is not None,'new_sha256':sha(bank),'utc':datetime.datetime.now(datetime.timezone.utc).isoformat()}
            if existing is not None:(backup/'prior-init.bnk').write_bytes(existing);receipt['prior_sha256']=sha(existing)
            (backup/'receipt.json').write_text(json.dumps(receipt,indent=2)+'\n')
            print('Keep this audio backup/receipt: '+str(backup),file=sys.stderr)
            target.parent.mkdir(parents=True,exist_ok=True);require_closed()
            fd,tmp=tempfile.mkstemp(prefix='supermoon-',suffix='.tmp',dir=target.parent)
            try:
                with os.fdopen(fd,'wb') as f:f.write(bank);f.flush();os.fsync(f.fileno())
                if (target.read_bytes() if target.exists() else None)!=existing:raise ValueError('Audio destination changed during installation')
                require_closed();os.replace(tmp,target)
            finally:
                if os.path.exists(tmp):os.unlink(tmp)
            if sha(target.read_bytes())!=plan['output_sha256']:raise ValueError('Installed audio bank verification failed')
            report.update(status='PASS_AUDIO_OVERRIDE_INSTALLED',backup=str(backup))
    print(json.dumps(report,indent=2))

if __name__=='__main__':main()
