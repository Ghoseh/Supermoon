// SPDX-License-Identifier: MIT
// Offline setup only. No game code is loaded. Compiled by Windows PowerShell Add-Type.
using System;
using System.IO;
using System.Text;
using System.Collections;
using System.Collections.Generic;
using System.Globalization;
using System.ComponentModel;
using System.Runtime.InteropServices;
using Microsoft.Win32.SafeHandles;

namespace SupermoonSetup {
 public static class Native {
  [StructLayout(LayoutKind.Sequential)] struct Info {
   public uint Attributes; public System.Runtime.InteropServices.ComTypes.FILETIME Created,Accessed,Written;
   public uint Volume,SizeHigh,SizeLow,Links,IndexHigh,IndexLow;
  }
  [StructLayout(LayoutKind.Sequential,CharSet=CharSet.Unicode)] struct Entry {
   public uint Size,Usage,Pid; public UIntPtr Heap; public uint Module,Threads,Parent;
   public int Priority; public uint Flags; [MarshalAs(UnmanagedType.ByValTStr,SizeConst=260)] public string Name;
  }
  [DllImport("kernel32.dll",CharSet=CharSet.Unicode,SetLastError=true)] static extern uint GetFileAttributesW(string path);
  [DllImport("kernel32.dll",CharSet=CharSet.Unicode,SetLastError=true)] static extern SafeFileHandle CreateFileW(string path,uint access,uint share,IntPtr security,uint disposition,uint flags,IntPtr template);
  [DllImport("kernel32.dll",SetLastError=true)] static extern bool GetFileInformationByHandle(SafeFileHandle file,out Info info);
  [DllImport("kernel32.dll",SetLastError=true)] static extern uint GetFileType(SafeFileHandle file);
  [DllImport("kernel32.dll",CharSet=CharSet.Unicode,SetLastError=true)] static extern bool MoveFileExW(string from,string to,uint flags);
  [DllImport("kernel32.dll",SetLastError=true)] static extern IntPtr CreateToolhelp32Snapshot(uint flags,uint pid);
  [DllImport("kernel32.dll",CharSet=CharSet.Unicode,SetLastError=true)] static extern bool Process32FirstW(IntPtr snapshot,ref Entry entry);
  [DllImport("kernel32.dll",CharSet=CharSet.Unicode,SetLastError=true)] static extern bool Process32NextW(IntPtr snapshot,ref Entry entry);
  [DllImport("kernel32.dll",CharSet=CharSet.Unicode,SetLastError=true)] static extern uint QueryDosDeviceW(string name,StringBuilder target,uint size);
  [DllImport("kernel32.dll",CharSet=CharSet.Unicode)] static extern uint GetDriveTypeW(string root);
  [DllImport("kernel32.dll",CharSet=CharSet.Unicode,SetLastError=true)] static extern uint GetFinalPathNameByHandleW(SafeFileHandle file,StringBuilder path,uint size,uint flags);
  [DllImport("kernel32.dll")] static extern bool CloseHandle(IntPtr handle);
  static Exception Error(string message) { int code=Marshal.GetLastWin32Error();Win32Exception detail=new Win32Exception(code);return new IOException(message+" (Win32 "+code+": "+detail.Message+")",detail); }
  public static string PathName(string path) {
   if(String.IsNullOrWhiteSpace(path)) throw new IOException("An explicit local path is required");
   // Never accept device/UNC namespaces, streams, wildcard paths or ambiguous Win32 names.
   string p=path.Replace('/','\\');
   if(p.StartsWith("\\") || p.IndexOfAny(new char[]{'*','?','"','<','>','|'})>=0) throw new IOException("Unsupported path namespace or characters");
   if(p.Length<3 || !Char.IsLetter(p[0]) || p[1]!=':' || p[2]!='\\' || p.Substring(2).Contains(":")) throw new IOException("Use an absolute local drive path");
   foreach(string part in p.Substring(3).Split('\\')) {
    if(part=="."||part==".."||part.EndsWith(".")||part.EndsWith(" ")) throw new IOException("Ambiguous path component refused");
    string stem=part.Split('.')[0].ToUpperInvariant();
    if(stem=="CON"||stem=="PRN"||stem=="AUX"||stem=="NUL"||System.Text.RegularExpressions.Regex.IsMatch(stem,@"^(COM|LPT)[1-9]$")) throw new IOException("Reserved path component refused");
   }
   string full=Path.GetFullPath(p);return full.Length>3?full.TrimEnd('\\'):full;
  }
  // Syntax first, then local object-namespace admission. Neither step traverses a
  // caller-supplied filesystem path. Redirector/SUBST/unknown mappings fail closed.
  public static string LocalPath(string path) {
   string p=PathName(path);
   StringBuilder target=new StringBuilder(4096);
   if(QueryDosDeviceW(p.Substring(0,2),target,(uint)target.Capacity)==0)throw Error("Cannot establish a local drive mapping");
   if(!System.Text.RegularExpressions.Regex.IsMatch(target.ToString(),@"^\\Device\\HarddiskVolume[0-9]+$",System.Text.RegularExpressions.RegexOptions.IgnoreCase)) throw new IOException("Nonlocal or aliased drive mappings are unsupported");
   if(GetDriveTypeW(p.Substring(0,3))!=3)throw new IOException("Use a fixed local drive; removable, remote and unknown drives are unsupported");
   return p;
  }
  public static void NoReparse(string path) {
   string p=LocalPath(path),current=Path.GetPathRoot(p);
   // Check root-to-leaf: never probe a child through an unchecked junction.
   List<string> paths=new List<string>();paths.Add(current);
   foreach(string part in p.Substring(current.Length).Split('\\'))if(part.Length!=0){current=Path.Combine(current,part);paths.Add(current);}
   foreach(string query in paths) {
    uint a=GetFileAttributesW(query);
    if(a==UInt32.MaxValue) { int e=Marshal.GetLastWin32Error(); if(e==2||e==3)return;throw Error("Cannot inspect path: "+query); }
    if((a&0x400)!=0) throw new IOException("Reparse-point paths are unsupported: "+query);
   }
  }
  public static string DirectoryIdentity(string path) {
   string p=LocalPath(path);NoReparse(p);
   using(SafeFileHandle h=CreateFileW(p,0x80,7,IntPtr.Zero,3,0x02200000,IntPtr.Zero)) {
    if(h.IsInvalid)throw Error("Cannot inspect directory identity");
    Info i;if(!GetFileInformationByHandle(h,out i))throw Error("Cannot inspect directory handle");
    if((i.Attributes&0x10)==0||(i.Attributes&0x400)!=0)throw new IOException("Not an admitted directory");
    StringBuilder final=new StringBuilder(32768);
    uint n=GetFinalPathNameByHandleW(h,final,(uint)final.Capacity,0);
    if(n==0||n>=final.Capacity)throw Error("Cannot resolve directory identity");
    string result=final.ToString();
    if(!result.StartsWith(@"\\?\",StringComparison.Ordinal))throw new IOException("Unsupported final directory namespace");
    result=LocalPath(result.Substring(4));NoReparse(result);return result;
   }
  }
  static Info Check(SafeFileHandle file,long limit) {
   Info i;
   if(!GetFileInformationByHandle(file,out i)) throw Error("Cannot inspect opened file");
   long size=((long)i.SizeHigh<<32)|i.SizeLow;
   if(GetFileType(file)!=1||(i.Attributes&(0x10|0x400))!=0||i.Links!=1||size<0||size>limit) throw new IOException("Unsupported file type, hard link or size");
   return i;
  }
  public static byte[] Read(string path,long limit) {
   NoReparse(path);
   using(SafeFileHandle h=CreateFileW(path,0x80000000,7,IntPtr.Zero,3,0x00200080,IntPtr.Zero)) {
    if(h.IsInvalid) { int e=Marshal.GetLastWin32Error(); if(e==2||e==3)return null; throw Error("Cannot read: "+path); }
    Info i=Check(h,limit); int size=checked((int)(((long)i.SizeHigh<<32)|i.SizeLow));
    using(FileStream f=new FileStream(h,FileAccess.Read)) {
     byte[] data=new byte[size]; int at=0,n;
     while(at<size) { n=f.Read(data,at,size-at); if(n==0)throw new IOException("File truncated during read");at+=n; }
     if(f.ReadByte()!=-1)throw new IOException("File grew during read");
     Info end=Check(h,limit);
     if(i.SizeHigh!=end.SizeHigh||i.SizeLow!=end.SizeLow||i.Written.dwHighDateTime!=end.Written.dwHighDateTime||i.Written.dwLowDateTime!=end.Written.dwLowDateTime) throw new IOException("File changed during read");
     return data;
    }
   }
  }
  public static void WriteNew(string path,byte[] data) {
   NoReparse(path);
   using(FileStream f=new FileStream(path,FileMode.CreateNew,FileAccess.Write,FileShare.None)) { f.Write(data,0,data.Length);f.Flush(true); }
  }
  public static void Replace(string from,string to) {
   NoReparse(from);NoReparse(to);
   if(!MoveFileExW(from,to,9))throw Error("Atomic individual replacement failed");
  }
  public static SafeFileHandle WriterLock(string path) {
   NoReparse(path);
   SafeFileHandle h=CreateFileW(path,0xc0000000,0,IntPtr.Zero,4,0x00200080,IntPtr.Zero);
   if(h.IsInvalid) {h.Dispose();throw Error("Another writer or unavailable installation lock");}
   try {Info i=Check(h,0);return h;}catch {h.Dispose();throw;}
  }
  public static string[] ProcessNames() {
   IntPtr h=CreateToolhelp32Snapshot(2,0);if(h==new IntPtr(-1))throw Error("Process snapshot failed");
   try {
    List<string> names=new List<string>();Entry e=new Entry();e.Size=(uint)Marshal.SizeOf(typeof(Entry));
    bool ok=Process32FirstW(h,ref e);
    while(ok) {if(String.IsNullOrEmpty(e.Name)||names.Count>=131072)throw new IOException("Indeterminate process snapshot"); names.Add(e.Name);ok=Process32NextW(h,ref e);}
    if(Marshal.GetLastWin32Error()!=18||names.Count==0)throw Error("Incomplete process enumeration");return names.ToArray();
   }finally {CloseHandle(h);}
  }
 }
 // Bounded strict JSON. Duplicate keys (including case variants) are refused at every depth.
 // No deserialization to runtime types, expressions, dates, or filesystem destinations.
 public sealed class Json {
  string s;int p;Json(string text){s=text;}
  void Space(){while(p<s.Length&&" \r\n\t".IndexOf(s[p])>=0)p++;}
  Exception Bad(){return new FormatException("Malformed, duplicate-key or oversized JSON");}
  char Take(){if(p==s.Length)throw Bad();return s[p++];}
  string Str(){
   if(Take()!='"')throw Bad();StringBuilder b=new StringBuilder();
   while(true){char c=Take();if(c=='"')return b.ToString();if(c<32)throw Bad();
    if(c=='\\'){c=Take();switch(c){case '"':case '\\':case '/':break;case 'b':c='\b';break;case 'f':c='\f';break;case 'n':c='\n';break;case 'r':c='\r';break;case 't':c='\t';break;case 'u':
     if(p+4>s.Length)throw Bad();ushort u;if(!UInt16.TryParse(s.Substring(p,4),NumberStyles.AllowHexSpecifier,CultureInfo.InvariantCulture,out u))throw Bad();p+=4;c=(char)u;break;default:throw Bad();}}
    b.Append(c);
   }
  }
  object Value(int depth){
   if(depth>32)throw Bad();Space();if(p==s.Length)throw Bad();char c=s[p];
   if(c=='"')return Str();
   if(c=='{'){p++;Dictionary<string,object>d=new Dictionary<string,object>(StringComparer.Ordinal);HashSet<string>seen=new HashSet<string>(StringComparer.OrdinalIgnoreCase);Space();if(p<s.Length&&s[p]=='}'){p++;return d;}
    while(true){Space();string k=Str();if(!seen.Add(k))throw Bad();Space();if(Take()!=':')throw Bad();d.Add(k,Value(depth+1));Space();c=Take();if(c=='}')return d;if(c!=',')throw Bad();}}
   if(c=='['){p++;List<object>a=new List<object>();Space();if(p<s.Length&&s[p]==']'){p++;return a.ToArray();}while(true){a.Add(Value(depth+1));Space();c=Take();if(c==']')return a.ToArray();if(c!=',')throw Bad();}}
   foreach(string literal in new string[]{"true","false","null"})if(p+literal.Length<=s.Length&&s.Substring(p,literal.Length)==literal){p+=literal.Length;return literal=="null"?null:(object)(literal=="true");}
   int start=p;while(p<s.Length&&"0123456789+-.eE".IndexOf(s[p])>=0)p++;
   string n=s.Substring(start,p-start);if(!System.Text.RegularExpressions.Regex.IsMatch(n,@"^-?(0|[1-9][0-9]*)(\.[0-9]+)?([eE][+-]?[0-9]+)?$"))throw Bad();
   long l;if(Int64.TryParse(n,NumberStyles.AllowLeadingSign,CultureInfo.InvariantCulture,out l))return l;
   double f;if(!Double.TryParse(n,NumberStyles.Float,CultureInfo.InvariantCulture,out f)||Double.IsInfinity(f)||Double.IsNaN(f))throw Bad();return f;
  }
  public static object Parse(byte[] bytes){if(bytes==null||bytes.Length>65536)throw new FormatException("Missing/oversized JSON");Json j=new Json(new UTF8Encoding(false,true).GetString(bytes));object v=j.Value(0);j.Space();if(j.p!=j.s.Length)throw j.Bad();return v;}
 }
}
