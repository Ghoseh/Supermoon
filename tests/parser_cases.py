# SPDX-FileCopyrightText: 2026 Supermoon contributors
# SPDX-License-Identifier: MIT
import xml.etree.ElementTree as ET
import hashlib,json
def dump(p,d):
    with p.open("x",encoding="utf-8") as f:json.dump(d,f,indent=2)

def corpus(work):
    cases=[]
    def wrap(body):return "<Data template='GcModSettings'>"+body+"</Data>"
    def prop(name='DisableAllMods',value='false'):return "<Property name='"+name+"' value='"+value+"'/>"
    def oracle(xml):
        try:r=ET.fromstring(xml)
        except (ET.ParseError,ValueError):return False
        if r.tag!='Data' or r.attrib!={'template':'GcModSettings'}:return False
        all_settings=[e for e in r.iter('Property') if e.attrib.get('name')=='DisableAllMods']
        return len(all_settings)==1 and all_settings[0] in list(r) and all_settings[0].attrib.get('value')=='false'
    def add(name,xml,expected=None):
        if isinstance(xml,str):xml=xml.encode('utf-8')
        logical=oracle(xml)
        if expected is not None:assert logical==expected,(name,logical,expected)
        cases.append((name,xml,logical))
    literal=prop()
    add('literal_enabled',wrap(literal),True)
    add('duplicate_decimal_conflict',wrap(literal+prop('DisableAllMod&#115;','true')),False)
    add('duplicate_hex_conflict',wrap(literal+prop('&#x44;isableAllMods','true')),False)
    add('reversed_decimal_conflict',wrap(prop('DisableAllMod&#115;','true')+literal),False)
    add('nested_decimal_alias',wrap(literal+"<Property name='Group'>"+prop('DisableAllMod&#115;','true')+"</Property>"),False)
    target='DisableAllMods'
    for i,c in enumerate(target):
        for kind,encoded in [('dec','&#'+str(ord(c))+';'),('hex','&#x'+format(ord(c),'x')+';'),('HEX','&#x'+format(ord(c),'X')+';'),('zero','&#00000'+str(ord(c))+';')]:
            alias=target[:i]+encoded+target[i+1:];key=str(i)+'_'+kind
            add('single_'+key,wrap(prop(alias)),True)
            for value in ['false','true']:
                add('tail_'+value+'_'+key,wrap(literal+prop(alias,value)),False)
                add('head_'+value+'_'+key,wrap(prop(alias,value)+literal),False)
            add('nested_'+key,wrap(literal+"<Property name='Group'>"+prop(alias)+"</Property>"),False)
    all_dec=''.join('&#'+str(ord(c))+';' for c in target)
    all_hex=''.join('&#x'+format(ord(c),'x')+';' for c in target)
    add('both_all_encoded',wrap(prop(all_dec)+prop(all_hex)),False)
    add('all_encoded_single',wrap(prop(all_hex,'fal&#115;e')),True)
    for i,c in enumerate('false'):
        add('encoded_false_dec_'+str(i),wrap(prop(value='false'[:i]+'&#'+str(ord(c))+';'+'false'[i+1:])),True)
        add('encoded_false_hex_'+str(i),wrap(prop(value='false'[:i]+'&#x'+format(ord(c),'x')+';'+'false'[i+1:])),True)
    for value in ['true','tr&#117;e','False','FALSE','0','','unknown',' false','false ','false&#32;']:
        add('disabled_'+str(len(cases)),wrap(prop(value=value)),False)
    add('missing',wrap(''),False)
    add('comment_alias_ignored',wrap(literal+'<!--'+prop(all_dec,'true')+'-->'),True)
    add('double_expansion_not_alias',wrap(prop('DisableAllMod&#38;#115;')),False)
    add('unrelated_double_expansion',wrap(literal+prop('Other&#38;#115;','true')),True)
    add('ordinary_entities',wrap(literal+prop('A&amp;B','&lt;&gt;&quot;&apos;&#65;&#x41;')),True)
    add('unicode_unrelated',wrap(literal+prop('Other','é / &#x1F319;')),True)
    add('long_zero_reference',wrap(prop('DisableAllMod&#'+'0'*64+'115;')),True)
    add('encoded_template',"<Data template='GcModSetting&#115;'>"+literal+'</Data>',True)
    malformed=['&','&;','&#;','&#x;','&#X73;','&#-1;','&#+115;','&#x+73;','&#12z;','&#x7g;','&#0;','&#1;','&#11;','&#12;','&#xD800;','&#xDFFF;','&#xFFFE;','&#65535;','&#x110000;','&#1114112;','&#999999999999999999999999999;','&unknown;','&#115','&amp','&# 115;']
    for i,reference in enumerate(malformed):
        for attr in ['name','value','other']:
            body=literal+prop('Other','bad'+reference) if attr=='other' else prop('DisableAllMod'+reference) if attr=='name' else prop(value='fal'+reference+'e')
            add('malformed_'+attr+'_'+str(i),wrap(body),False)
    valid="<?xml version='1.0' encoding='utf-8'?>"+wrap(prop('DisableAllMod&#115;','fal&#x73;e'))
    for n in range(len(valid)):add('truncate_'+str(n),valid[:n],False)
    add('encoded_valid_declaration',valid,True)
    for i,b in enumerate([b'\xc0\x80',b'\xed\xa0\x80',b'\xf4\x90\x80\x80',b'\x00',b'\x0b']):add('invalid_utf8_or_char_'+str(i),wrap(literal).encode().replace(b'</Data>',b+ b'</Data>'),False)
    fixture=work/'xml-cases';fixture.mkdir()
    lines=[]
    for i,(name,xml,expected) in enumerate(cases):
        p=fixture/(str(i)+'.xml');p.write_bytes(xml);lines.append(name+'\t'+str(int(expected))+'\t'+str(p))
    manifest=work/'CASES.tsv';manifest.write_text('\n'.join(lines)+'\n')
    dump(work/'ORACLE_CASES.json',[{'case':n,'xml_sha256':hashlib.sha256(b).hexdigest(),'bytes':len(b),'expected':e} for n,b,e in cases])
    return manifest
