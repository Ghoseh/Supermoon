# Third-party notices

Supermoon original source: MIT (LICENSE). Third-party terms remain separate.
The source, generated meter, and statically linked runtime portions include:

| Component | Applicable notice files in licenses/ |
|---|---|
| dr_mp3 0.7.4 with local bounds/error patches; embedded minimp3 | dr_mp3-repository-LICENSE.txt; dr_mp3-header-and-minimp3-notices.txt (MIT-0 selected; embedded CC0 notice retained) |
| libebur128 1.2.6, including the modified production meter | libebur128-COPYING.txt (MIT) |
| Berkeley queue.h 8.5 | Berkeley-queue-BSD-3-Clause.txt |
| MinGW-w64 13.0.0 alpha snapshot | MinGW-COPYING-ZPL-2.1.txt; Lucent-gdtoa-per-file-NOTICES.txt |
| libc++, libc++abi, libunwind from pinned Zig | LLVM-libcxx-LICENSE.txt; LLVM-libcxxabi-LICENSE.txt; LLVM-libunwind-LICENSE.txt (Apache-2.0 WITH LLVM exception and retained legacy/per-file text) |
| Zig 0.15.2 builtins/compiler runtime; borrowed math helpers | Zig-MIT.txt; musl-COPYRIGHT.txt; musl-derived-math-per-file-NOTICES.txt |

The Lucent file retains the actual gdtoa copyright variants, not a substituted
license with similar wording. BSD nonendorsement conditions apply. Preserve the
complete notices directory with binary redistribution. The LLVM compiler-inserted
object exception can waive some Apache notice duties; full notices are retained
conservatively. There is no assertion that every musl object is linked.

Windows UCRT/API sets, KERNEL32, USER32, bcrypt and system WINMM are OS dependencies
obtained from Windows, not copied into the package. No Man's Sky is proprietary
host software and is not distributed. This project is not endorsed by its authors.

The full compiler archive is not bundled. toolchain-only-winpthreads-MIT.txt is
retained for reference; winpthreads was not positively identified as linked into
these DLLs. Other-platform toolchain libraries are likewise not Windows runtime
dependencies. See docs/DEPENDENCIES.md and docs/DEPENDENCIES.json for all 23 entries,
versions, integration evidence, qualifications and source-distribution obligations.
