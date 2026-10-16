# SPDX-FileCopyrightText: 2026 Supermoon contributors
# SPDX-License-Identifier: MIT
"""Qualified informational PE normalization; use only on owned build outputs."""
import struct
from pe import PE
def normalize(path):
    before=PE(path);data=bytearray(before.data);rva,size=before.directory(6)
    if rva:
        assert size and size%28==0;begin=before.offset(rva)
        for pos in range(begin,begin+size,28):
            _,_,_,_,kind,n,_,raw=struct.unpack_from('<IIHHIIII',data,pos);assert kind in (2,16)
            if n:
                assert kind==2 and n>=25 and raw>=0x400 and raw+n<=len(data) and data[raw:raw+4]==b'RSDS'
                assert data[raw+n-1]==0 and b'.pdb' in data[raw+24:raw+n].lower();data[raw:raw+n]=bytes(n)
        data[begin:begin+size]=bytes(size);at=before.opt+112+6*8;data[at:at+8]=bytes(8)
    offsets=[before.u32(0x3c)+8];export,_=before.directory(0)
    if export:offsets.append(before.offset(export)+4)
    for at in offsets:struct.pack_into('<I',data,at,0)
    path.write_bytes(data);after=PE(path)
    assert before.exports()==after.exports() and before.imports()==after.imports()
    assert before.directory(3)==after.directory(3) and before.directory(5)==after.directory(5)
