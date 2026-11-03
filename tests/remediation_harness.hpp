// SPDX-License-Identifier: MIT
#pragma once
#include <cstdio>
#include <cstring>
#include <stdexcept>

namespace remediation_test {
inline unsigned failures{};
inline void expect(bool ok,const char* message) {
    std::printf("%s %s\n",ok?"OK":"FAIL",message);
    if(!ok)++failures;
}
inline void require(bool ok,const char* message) {
    if(!ok)throw std::runtime_error(message);
}
struct Case {const char* name;void (*run)();};
template<size_t N> int main(int argc,char** argv,const Case (&cases)[N]) {
    if(argc==2&&!std::strcmp(argv[1],"--list")) {
        for(const auto& c:cases)std::puts(c.name);
        return 0;
    }
    if(argc!=2){std::fputs("Supply one case name, or --list\n",stderr);return 2;}
    for(const auto& c:cases)if(!std::strcmp(argv[1],c.name)) {
        std::printf("CASE %s\n",c.name);std::fflush(stdout);
        try {c.run();}
        catch(const std::exception& e){std::printf("HARNESS ERROR %s\n",e.what());return 2;}
        std::printf("RESULT %s failures=%u\n",failures?"FAIL":"PASS",failures);
        return failures?1:0;
    }
    std::fputs("Unknown case\n",stderr);return 2;
}
}
