# SPDX-License-Identifier: MIT
"""Windows process admission and cooperative filesystem safety for offline setup."""
from pathlib import Path
from contextlib import contextmanager
import ctypes
from ctypes import wintypes as w
import os,stat


def no_reparse(path):
    # Check the supplied spelling BEFORE resolve() can hide a junction ancestor.
    for p in (Path(path),*Path(path).parents):
        try:s=p.lstat()
        except FileNotFoundError:continue
        if stat.S_ISLNK(s.st_mode) or getattr(s,'st_file_attributes',0)&0x400:
            raise ValueError('Reparse-point paths are unsupported: '+str(p))


def process_names():
    """Toolhelp snapshot, no executable search, subprocess, shell or alertable wait."""
    if os.name!='nt':raise ValueError('Installation requires Windows')
    class Entry(ctypes.Structure):
        _fields_=[('dwSize',w.DWORD),('cntUsage',w.DWORD),('th32ProcessID',w.DWORD),
                  ('th32DefaultHeapID',ctypes.c_size_t),('th32ModuleID',w.DWORD),
                  ('cntThreads',w.DWORD),('th32ParentProcessID',w.DWORD),
                  ('pcPriClassBase',w.LONG),('dwFlags',w.DWORD),('szExeFile',w.WCHAR*260)]
    # kernel32 is a Windows KnownDLL already required by the Python process.
    api=ctypes.WinDLL('kernel32',use_last_error=True)
    api.CreateToolhelp32Snapshot.argtypes=[w.DWORD,w.DWORD];api.CreateToolhelp32Snapshot.restype=w.HANDLE
    api.Process32FirstW.argtypes=[w.HANDLE,ctypes.POINTER(Entry)];api.Process32FirstW.restype=w.BOOL
    api.Process32NextW.argtypes=[w.HANDLE,ctypes.POINTER(Entry)];api.Process32NextW.restype=w.BOOL
    api.CloseHandle.argtypes=[w.HANDLE];api.CloseHandle.restype=w.BOOL
    handle=api.CreateToolhelp32Snapshot(2,0)
    if handle==ctypes.c_void_p(-1).value:raise OSError(ctypes.get_last_error(),'Process snapshot failed')
    result=[];entry=Entry();entry.dwSize=ctypes.sizeof(entry)
    try:
        ok=api.Process32FirstW(handle,ctypes.byref(entry))
        while ok:
            result.append(entry.szExeFile)
            if len(result)>131072:raise ValueError('Indeterminate process snapshot (entry bound)')
            ok=api.Process32NextW(handle,ctypes.byref(entry))
        if ctypes.get_last_error()!=18 or not result:
            raise OSError(ctypes.get_last_error(),'Incomplete process enumeration')
    finally:api.CloseHandle(handle)
    return result


def require_closed():
    names=process_names()
    if not names or any(not isinstance(n,str) or not n for n in names):raise ValueError('Indeterminate process names')
    if any(name.casefold()=='nms.exe' for name in names):
        raise ValueError('Exit NMS fully before installation/recovery; do not start it during this operation')


@contextmanager
def writer_lock(game):
    """Only cooperating tools are serialized. The empty lock file is retained."""
    path=Path(game)/'.supermoon-install.lock';no_reparse(path)
    if path.exists() and (not path.is_file() or path.stat().st_size):
        raise ValueError('Unknown installation lock file; preserve it and investigate')
    if os.name!='nt':raise ValueError('Installation requires Windows')
    api=ctypes.WinDLL('kernel32',use_last_error=True)
    api.CreateFileW.argtypes=[w.LPCWSTR,w.DWORD,w.DWORD,w.LPVOID,w.DWORD,w.DWORD,w.HANDLE]
    api.CreateFileW.restype=w.HANDLE;api.CloseHandle.argtypes=[w.HANDLE];api.CloseHandle.restype=w.BOOL
    h=api.CreateFileW(str(path),0xc0000000,0,None,4,0x00200080,None)
    if h==ctypes.c_void_p(-1).value:raise OSError(ctypes.get_last_error(),'Another writer or unavailable installation lock')
    try:yield
    finally:api.CloseHandle(h)
