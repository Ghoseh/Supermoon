# SPDX-License-Identifier: MIT
param([Parameter(Mandatory=$true)][string]$Package,[Parameter(Mandatory=$true)][string]$Work)
Set-StrictMode -Version Latest;$ErrorActionPreference='Stop'
$Package=[IO.Path]::GetFullPath($Package);$Work=[IO.Path]::GetFullPath($Work)
if(Test-Path -LiteralPath $Work){throw 'New owned test output required'}
[void][IO.Directory]::CreateDirectory($Work)
$module=Import-Module (Join-Path $Package 'Supermoon/tools/setup/Supermoon.Setup.psm1') -Force -PassThru
Add-Type -TypeDefinition @'
using System;using System.Text;using System.Runtime.InteropServices;
public static class IniFixtureRead {
 [DllImport("kernel32.dll",CharSet=CharSet.Unicode)]static extern uint GetPrivateProfileString(string section,string key,string fallback,StringBuilder result,uint count,string file);
 public static string Read(string file,string section,string key){var b=new StringBuilder(4096);GetPrivateProfileString(section,key,"__MISSING__",b,4096,file);return b.ToString();}
}
'@
$recipe=& $module {param($p) Read-SMJson (Join-Path $p 'Supermoon/build/recipe.json')} $Package
$plan=& $module {param($p) Read-SMJson (Join-Path $p 'Supermoon/tools/audio_patch.json')} $Package
$schema=& $module {$script:IniDefaults};$script:passed=New-Object 'Collections.Generic.List[string]'
function Check($Test,$Message){if(-not $Test){throw $Message}}
function Run-Case($Name,[byte[]]$Raw,[string]$Fault=''){
 $root=Join-Path $Work $Name;[void][IO.Directory]::CreateDirectory((Join-Path $root 'Binaries/Supermoon'))
 $ini=Join-Path $root 'Binaries/Supermoon/playlist.ini'
 if($null -ne $Raw){[IO.File]::WriteAllBytes($ini,$Raw)}
 $before=@{}
 foreach($s in $schema){foreach($k in $s.Values.Keys){$before[$s.Name+'.'+$k]=[IniFixtureRead]::Read($ini,$s.Name,$k)}}
 $c=New-SMContext $root $recipe $plan
 & $module {param($c,$p) $c.ConfigTemplate=Get-SMConfigTemplate $c $p;[void][IO.Directory]::CreateDirectory($c.Store)} $c $Package
 if($Fault){
  try{& $module {param($c,$point) $script:IniFault=$point;function script:Invoke-SMCheckpoint($Stage){if($Stage -ceq $script:IniFault){throw 'OWNED INI FAULT'}};Invoke-SMConfigDefaults $c} $c $Fault;throw 'Fault not reached'}catch{if($_.Exception.Message -notmatch 'OWNED INI FAULT'){throw}}
  & $module {function script:Invoke-SMCheckpoint($Stage){}}
 }
 $result=& $module {param($c) Invoke-SMConfigDefaults $c} $c
 $after=& $module {param($ini) Get-SMCurrent $ini 65536} $ini
 foreach($s in $schema){foreach($k in $s.Values.Keys){
  $expected=$before[$s.Name+'.'+$k];if($expected -ceq '__MISSING__'){$expected=$s.Values[$k]}
  Check ([IniFixtureRead]::Read($ini,$s.Name,$k) -ceq $expected) ($Name+' changed/missing '+$s.Name+'.'+$k)
 }}
 if($null -ne $Raw -and -not(& $module {param($raw,$after) Test-SMEqual (Get-SMIdentity $raw) $after} $Raw $after)){
  $original=& $module {param($raw) Get-SMIdentity $raw} $Raw
  $backup=Join-Path $c.Store ('configuration/'+$original.sha256+'.ini')
  Check (& $module {param($p,$id) Test-SMEqual (Get-SMCurrent $p 65536) $id} $backup $original) 'Original INI backup not exact'
 }
 $again=& $module {param($c) Invoke-SMConfigDefaults $c} $c
 Check ($again.added -eq 0) 'Second completion added keys again'
 Check (& $module {param($p,$id) Test-SMEqual (Get-SMCurrent $p 65536) $id} $ini $after) 'Second completion rewrote file'
 [void]$script:passed.Add($Name);Write-Host ('PASS '+$Name)
 return $c
}
$ascii=[Text.Encoding]::ASCII
[void](Run-Case 'fresh-missing' $null)
[void](Run-Case 'empty-existing' ([byte[]]@()))
$body="; USER COMMENT`r`n[Playlist]`r`nMusic=Custom Music`r`nCacheMiB=2048`r`n[Audio]`r`nVoiceGainDb=0.0`r`n[Keyboard]`r`nNext=Ctrl+Alt+F7`r`n[Notifications]`r`nShuffleSounds=0`r`n[Unknown]`r`nKeep=untouched"
$raw=$ascii.GetBytes($body);[void](Run-Case 'existing-crlf-preferences' $raw)
$c=Run-Case 'existing-lf-without-final-newline' ($ascii.GetBytes($body.Replace("`r`n","`n")))
$text=[IO.File]::ReadAllText((Join-Path $c.Game 'Binaries/Supermoon/playlist.ini'))
Check ($text.Contains('; USER COMMENT') -and $text.Contains('Keep=untouched')) 'Comments/unknown section lost'
[void](Run-Case 'empty-explicit-gain-preserved' ($ascii.GetBytes("[Audio]`nVoiceGainDb=`n")))
[void](Run-Case 'malformed-explicit-gain-preserved' ($ascii.GetBytes("[Audio]`nVoiceGainDb=NaN`n")))
[void](Run-Case 'mixed-case-and-spaces' ($ascii.GetBytes("[aUdIo]`n  voicegaindb = 4.5`n[kEyBoArD]`nNext=Ctrl+F7`n")))
$unicode=[Text.UnicodeEncoding]::new($false,$true,$true);$utf=@($unicode.GetPreamble())+@($unicode.GetBytes($body))
[void](Run-Case 'utf16le-bom' ([byte[]]$utf))
$utf8=[Text.UTF8Encoding]::new($false);[void](Run-Case 'utf8-no-bom' ($utf8.GetBytes($body.Replace('Custom Music','Music-cafÃ©'))))
foreach($point in @('config:backed_up','config:prepared','config:before_replace','config:after_replace')){[void](Run-Case ('interrupted-'+$point.Replace(':','-')) $raw $point)}
# Actual sharing failure: retain the original, then retry after releasing the handle.
$root=Join-Path $Work 'sharing-failure';[void][IO.Directory]::CreateDirectory((Join-Path $root 'Binaries/Supermoon'));$ini=Join-Path $root 'Binaries/Supermoon/playlist.ini';[IO.File]::WriteAllBytes($ini,$raw)
$c=New-SMContext $root $recipe $plan;& $module {param($c,$p) $c.ConfigTemplate=Get-SMConfigTemplate $c $p;[void][IO.Directory]::CreateDirectory($c.Store)} $c $Package
$h=[IO.File]::Open($ini,[IO.FileMode]::Open,[IO.FileAccess]::Read,[IO.FileShare]::Read)
try{try{& $module {param($c) Invoke-SMConfigDefaults $c} $c;throw 'Expected sharing failure'}catch{if($_.Exception.Message -eq 'Expected sharing failure'){throw}}}finally{$h.Dispose()}
Check ([Convert]::ToBase64String([IO.File]::ReadAllBytes($ini)) -ceq [Convert]::ToBase64String($raw)) 'Sharing failure changed original'
[void](& $module {param($c) Invoke-SMConfigDefaults $c} $c);[void]$script:passed.Add('sharing-failure-retry');Write-Output 'PASS sharing-failure-retry'
# Ambiguous duplicate sections refuse before creating records or changing content.
$root=Join-Path $Work 'duplicate-sections';[void][IO.Directory]::CreateDirectory((Join-Path $root 'Binaries/Supermoon'));$ini=Join-Path $root 'Binaries/Supermoon/playlist.ini';$bad=$ascii.GetBytes("[Audio]`nVoiceGainDb=0`n[Audio]`nVoiceGainDb=6`n");[IO.File]::WriteAllBytes($ini,$bad)
$c=New-SMContext $root $recipe $plan
try{& $module {param($c,$p) Get-SMConfigTemplate $c $p} $c $Package;throw 'Expected duplicate refusal'}catch{if($_.Exception.Message -eq 'Expected duplicate refusal'){throw}}
Check ([Convert]::ToBase64String([IO.File]::ReadAllBytes($ini)) -ceq [Convert]::ToBase64String($bad)) 'Ambiguous file changed'
Check (-not(Test-Path -LiteralPath $c.Store)) 'Ambiguous configuration created recovery state'
[void]$script:passed.Add('duplicate-sections-refused');Write-Output 'PASS duplicate-sections-refused'
# Unsupported BOM encodings refuse before altering an existing file.
foreach($kind in @('utf8-bom','utf16be-bom')){
 $root=Join-Path $Work $kind;[void][IO.Directory]::CreateDirectory((Join-Path $root 'Binaries/Supermoon'));$ini=Join-Path $root 'Binaries/Supermoon/playlist.ini'
 $e=if($kind -eq 'utf8-bom'){[Text.UTF8Encoding]::new($true)}else{[Text.UnicodeEncoding]::new($true,$true,$true)}
 $data=[byte[]](@($e.GetPreamble())+@($e.GetBytes($body)));[IO.File]::WriteAllBytes($ini,$data);$c=New-SMContext $root $recipe $plan
 try{& $module {param($c,$p) Get-SMConfigTemplate $c $p} $c $Package;throw 'Expected encoding refusal'}catch{if($_.Exception.Message -eq 'Expected encoding refusal'){throw}}
 Check ([Convert]::ToBase64String([IO.File]::ReadAllBytes($ini)) -ceq [Convert]::ToBase64String($data)) 'Unsupported encoding was changed'
 [void]$script:passed.Add($kind+'-refused');Write-Output ('PASS '+$kind+'-refused')
}
$summary=@{result='PASS';cases=$script:passed.ToArray();case_count=$script:passed.Count;all_twelve_settings_checked_with_native_Windows_INI_API=$true;no_game_or_DLL_loaded=$true}
[IO.File]::WriteAllText((Join-Path $Work 'RESULTS.json'),(ConvertTo-Json -InputObject $summary -Depth 5),[Text.UTF8Encoding]::new($false))
