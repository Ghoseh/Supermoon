# SPDX-FileCopyrightText: 2026 Supermoon contributors
# SPDX-License-Identifier: MIT
"""Bounded PE64 metadata reader. Reads files as data, never executes them."""
from pathlib import Path
import struct


class PE:
    def __init__(self, path):
        self.data = Path(path).read_bytes()
        p = self.u32(0x3c)
        assert self.data[:2] == b'MZ' and self.data[p:p+4] == b'PE\0\0'
        assert self.u16(p+4) == 0x8664 and self.u16(p+24) == 0x20b
        self.opt = p+24
        self.sections = []
        for i in range(self.u16(p+6)):
            q = self.opt+self.u16(p+20)+40*i
            vs, va, size, raw = struct.unpack_from('<IIII', self.data, q+8)
            self.sections.append((va, size, raw))

    def u16(self, p): return struct.unpack_from('<H', self.data, p)[0]
    def u32(self, p): return struct.unpack_from('<I', self.data, p)[0]
    def u64(self, p): return struct.unpack_from('<Q', self.data, p)[0]
    def offset(self, rva):
        for va, size, raw in self.sections:
            if va <= rva < va+size: return raw+rva-va
        raise ValueError(f'Unbacked RVA {rva:x}')
    def string(self, rva):
        p = self.offset(rva)
        return self.data[p:self.data.index(0, p)].decode('ascii')
    def directory(self, n):
        return struct.unpack_from('<II', self.data, self.opt+112+8*n)

    def exports(self):
        rva, size = self.directory(0)
        p = self.offset(rva)
        base, count, named, funcs, names, ords = struct.unpack_from('<IIIIII', self.data, p+16)
        labels = {}
        for i in range(named):
            index = self.u16(self.offset(ords)+2*i)
            labels[index] = self.string(self.u32(self.offset(names)+4*i))
        result = []
        for i in range(count):
            target = self.u32(self.offset(funcs)+4*i)
            if target:
                result.append({'ordinal':base+i, 'name':labels.get(i), 'rva':target,
                               'forward':self.string(target) if rva <= target < rva+size else None})
        return result

    def imports(self, delay=False):
        rva, _ = self.directory(13 if delay else 1)
        result = []
        if not rva: return result
        p = self.offset(rva)
        stride = 32 if delay else 20
        while any(self.data[p:p+stride]):
            if delay:
                attrs, name, _, ft, oft = struct.unpack_from('<IIIII', self.data, p)
                assert attrs == 1, 'Only RVA delay imports supported'
            else:
                oft, _, _, name, ft = struct.unpack_from('<IIIII', self.data, p)
            symbols = []
            n = 0
            while True:
                t = self.u64(self.offset((oft or ft)+8*n))
                if not t: break
                symbols.append({'symbol':t & 0xffff if t >> 63 else self.string(t+2), 'iat_rva':ft+8*n})
                n += 1
            result.append({'dll':self.string(name), 'symbols':symbols, 'delay':delay})
            p += stride
        return result
