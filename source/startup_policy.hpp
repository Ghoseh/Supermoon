// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#pragma once
#include <cstdint>
#include <array>
#include <string_view>

namespace bytebeat::startup {
enum class Readiness { waiting, ready, conflict };
// Only immutable addresses in the pinned image are accepted. Never chase the
// audio manager or a partially constructed game object from the worker.
inline Readiness classify(uintptr_t base, uintptr_t execute, uintptr_t format, uintptr_t gain) noexcept {
    if (!execute && !format && !gain) return Readiness::waiting;
    // The setter publishes execute then format then gain. Its intermediate
    // expected/zero state is not an error and must not authorize activation.
    if ((!execute || execute == base+0x2E2F6D0) &&
        (!format || format == base+0x2E2F750) && !gain)
        return execute && format ? Readiness::ready : Readiness::waiting;
    return Readiness::conflict;
}

inline bool whitespace(char c) noexcept { return c==' ' || c=='\t' || c=='\r' || c=='\n'; }
inline bool xml_character(unsigned c) noexcept {
    return (c>=32&&c<=0xd7ff)||(c>=0xe000&&c<=0xfffd)||(c>=0x10000&&c<=0x10ffff)||c==9||c==10||c==13;
}
inline bool xml_reference(std::string_view entity,unsigned& code) noexcept {
    if(entity=="amp")code='&';else if(entity=="lt")code='<';else if(entity=="gt")code='>';
    else if(entity=="quot")code='"';else if(entity=="apos")code='\'';
    else {
        if(entity.empty()||entity[0]!='#')return false;
        size_t i=1;unsigned base=10;code=0;
        if(i<entity.size()&&entity[i]=='x'){base=16;++i;}if(i==entity.size())return false;
        for(;i<entity.size();++i){
            char c=entity[i];unsigned d=c>='0'&&c<='9'?unsigned(c-'0'):c>='a'&&c<='f'?unsigned(c-'a'+10):c>='A'&&c<='F'?unsigned(c-'A'+10):16;
            if(d>=base||code>(0x10ffff-d)/base)return false;code=code*base+d;
        }
    }
    return xml_character(code);
}
// Compare validated attribute text with its logical ASCII value. References are
// decoded exactly once; no allocation, recursive expansion or external entities.
// The three authorization targets are ASCII; this is not a general UTF-8 decoder.
inline bool xml_attribute_equals(std::string_view raw,std::string_view expected) noexcept {
    size_t p=0,n=0;
    while(p<raw.size()){
        unsigned code=static_cast<unsigned char>(raw[p++]);
        if(code=='&'){
            auto end=raw.find(';',p);if(end==raw.npos||!xml_reference(raw.substr(p,end-p),code))return false;
            p=end+1;
        }
        if(n==expected.size()||code!=static_cast<unsigned char>(expected[n++]))return false;
    }
    return n==expected.size();
}
// Bounded supported MXML grammar: one Data root, balanced Property children,
// whitespace/comments and a restricted optional UTF-8 XML declaration. Limits
// are 64 KiB, 32 open elements and 16 attributes; this is not general XML/schema
// support. A setting name is compared after reference decoding, at any depth:
// a nested alias or any second logical DisableAllMods refuses authorization.
// Unsupported markup never
// authorizes startup. No allocation, recursive entity expansion or external resources.
inline bool mods_enabled(std::string_view xml) noexcept {
    if (xml.empty() || xml.size()>65536)return false;
    // XML 1.0 characters and well-formed UTF-8, including non-ASCII filenames.
    for(size_t i=0;i<xml.size();){
        unsigned c=static_cast<unsigned char>(xml[i++]),n=0,minimum=0;
        if(c>=128){
            if(c>=0xc2&&c<=0xdf){c&=31;n=1;minimum=0x80;}
            else if(c>=0xe0&&c<=0xef){c&=15;n=2;minimum=0x800;}
            else if(c>=0xf0&&c<=0xf4){c&=7;n=3;minimum=0x10000;}
            else return false;
            if(n>xml.size()-i)return false;
            while(n--){auto b=static_cast<unsigned char>(xml[i++]);if((b&0xc0)!=0x80)return false;c=(c<<6)|(b&63);}
            if(c<minimum)return false;
        }
        if(!xml_character(c))return false;
    }
    unsigned matches=0,depth=0;bool enabled=false,root=false,closed=false,declaration=false;
    std::array<bool,32> leaf{};
    size_t p=0;
    if(xml.substr(0,3)=="\xef\xbb\xbf")p=3;
    const size_t document_start=p;
    auto space=[&]{while(p<xml.size()&&whitespace(xml[p]))++p;};
    auto name_char=[](char c){return (c>='a'&&c<='z')||(c>='A'&&c<='Z')||c=='_'||(c>='0'&&c<='9')||c=='-'||c=='.';};
    auto name=[&](){size_t begin=p;if(p==xml.size()||!((xml[p]>='a'&&xml[p]<='z')||(xml[p]>='A'&&xml[p]<='Z')||xml[p]=='_'))return std::string_view{};while(p<xml.size()&&name_char(xml[p]))++p;return xml.substr(begin,p-begin);};
    while(p<xml.size()){
        space();if(p==xml.size())break;
        if(xml[p]!='<')return false;
        if (xml.substr(p,4)=="<!--") {
            auto end=xml.find("--",p+4);if(end==xml.npos||xml.substr(end,3)!="-->")return false;
            p=end+3; continue;
        }
        bool decl=xml.substr(p,5)=="<?xml",closing=xml.substr(p,2)=="</";
        if(decl){if(root||declaration||p!=document_start)return false;declaration=true;p+=2;}
        else p+=closing?2:1;
        auto tag=name();if(tag.empty())return false;
        if(closing){space();if(p==xml.size()||xml[p++]!='>'||!depth||tag!=(depth==1?"Data":"Property"))return false;if(--depth==0)closed=true;continue;}
        std::string_view property,value,type,version,encoding,standalone;
        std::array<std::string_view,16> attrs{};size_t count=0;bool self=false;
        for(;;){
            auto before=p;space();
            if(xml.substr(p,2)==(decl?"?>":"/>")){p+=2;self=true;break;}
            if(!decl&&p<xml.size()&&xml[p]=='>'){++p;break;}
            if(p==before||count==attrs.size())return false;
            auto attr=name();if(attr.empty())return false;
            for(size_t i=0;i<count;++i)if(attrs[i]==attr)return false;
            attrs[count++]=attr;space();if(p==xml.size()||xml[p++]!='=')return false;space();
            if(p==xml.size()||(xml[p]!='\"'&&xml[p]!='\''))return false;
            char quote=xml[p++];auto begin=p;
            while(p<xml.size()&&xml[p]!=quote){
                if(xml[p]=='<')return false;
                if(xml[p]=='&'){
                    auto end=xml.find(';',p+1);unsigned code=0;
                    if(end==xml.npos||!xml_reference(xml.substr(p+1,end-p-1),code))return false;p=end;
                }++p;
            }
            if(p==xml.size())return false;auto text=xml.substr(begin,p++-begin);
            if(attr=="name")property=text;else if(attr=="value")value=text;else if(attr=="template")type=text;
            else if(decl&&attr=="version")version=text;else if(decl&&attr=="encoding")encoding=text;else if(decl&&attr=="standalone")standalone=text;
            else return false;
        }
        if(decl){
            if(tag!="xml"||!count||attrs[0]!="version"||version!="1.0"||count>3)return false;
            if(count>=2&&attrs[1]!="encoding"&&attrs[1]!="standalone")return false;
            if(count==3&&(attrs[1]!="encoding"||attrs[2]!="standalone"))return false;
            for(size_t i=1;i<count;++i){if(attrs[i]=="encoding"&&encoding!="UTF-8"&&encoding!="utf-8")return false;if(attrs[i]=="standalone"&&standalone!="yes"&&standalone!="no")return false;}
            continue;
        }
        if(!depth){if(root||closed||tag!="Data"||!xml_attribute_equals(type,"GcModSettings")||count!=1||self)return false;root=true;}
        else {if(tag!="Property"||leaf[depth-1]||property.empty())return false;if(xml_attribute_equals(property,"DisableAllMods")){if(depth!=1||++matches!=1)return false;enabled=xml_attribute_equals(value,"false");}}
        if(!self){if(depth==leaf.size())return false;leaf[depth++]=!value.empty();}
    }
    return root&&closed&&!depth&&matches==1&&enabled;
}
}
