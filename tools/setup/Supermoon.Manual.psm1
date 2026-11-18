# SPDX-License-Identifier: MIT
# Companion to unchanged automatic setup. Only the preparation folder is written.
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
$script:Core=Import-Module (Join-Path $PSScriptRoot 'Supermoon.Setup.psm1') -Force -PassThru
$script:ManualPaths=[ordered]@{
    proxy='Binaries/winmm.dll';payload='Binaries/Supermoon/supermoon_playlist.dll'
    bank='GAMEDATA/MODS/Supermoon_MenuAudio_Test/audio/windows/init.bnk'
    marker='Binaries/bytebeat-playlist-experimental.enabled'
}
function Invoke-SMManualCheckpoint($Stage) {} # Owned tests only; not a CLI/environment control.
function Assert-SMManualSeparate($Left,$Right){
    if((& $script:Core {param($a,$b) (Test-SMWithin $a $b) -or (Test-SMWithin $b $a)} $Left $Right)){
        throw 'Game, package and prepared output must be separate, non-nested directories.'
    }
}
function Get-SMManualDirectory($Path){
    $p=[SupermoonSetup.Native]::PathName($Path);[SupermoonSetup.Native]::NoReparse($p)
    if(-not [IO.Directory]::Exists($p)){throw 'An existing fixed local directory is required.'}
    return [SupermoonSetup.Native]::DirectoryIdentity($p)
}
function Assert-SMManualNoRecords($C){
    foreach($name in @('.Supermoon-Recovery','.Supermoon-Recovery-1.2.1')){
        $p=Join-Path $C.Game $name;[SupermoonSetup.Native]::NoReparse($p)
        if([IO.File]::Exists($p) -or [IO.Directory]::Exists($p)){
            throw 'An automatic installation/recovery store exists. Use matching Setup.cmd Status/Disable/Upgrade; do not delete records or replace files manually.'
        }
    }
}
function Get-SMManualObserved($C){return & $script:Core {param($c) Get-SMObserved $c} $C}
function Assert-SMManualFresh($C){
    & $script:Core {param($c) Assert-SMClosed;Assert-SMGame $c;Assert-SMNoLooseConflict $c} $C
    Assert-SMManualNoRecords $C
    foreach($id in (Get-SMManualObserved $C).Values){if($null -ne $id){throw 'Manual preparation requires all four activation files absent. Use matching automatic setup for installed/partial/unknown installations.'}}
}
function Get-SMManualIni($C){
    return & $script:Core {param($c) Get-SMCurrent (Join-Path $c.Game 'Binaries/Supermoon/playlist.ini') 65536} $C
}
function Write-SMManualFile($Root,$Relative,[byte[]]$Bytes){
    $p=Join-Path $Root $Relative;[SupermoonSetup.Native]::NoReparse($p)
    [void][IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($p))
    & $script:Core {param($p,$b) Write-SMNew $p $b} $p $Bytes
}
function Test-SMManualEqual($A,$B){return & $script:Core {param($a,$b) Test-SMEqual $a $b} $A $B}
function Get-SMManualFile($P,[long]$Limit=33554432){return ,([SupermoonSetup.Native]::Read($P,$Limit))}
function Get-SMManualIdentity($Bytes){return & $script:Core {param($b) Get-SMIdentity $b} $Bytes}

function Get-SMManualContents($Root){
    $all=@{};$pending=New-Object 'Collections.Generic.Queue[string]';$pending.Enqueue($Root)
    while($pending.Count){
        $dir=$pending.Dequeue();[SupermoonSetup.Native]::NoReparse($dir)
        foreach($p in [IO.Directory]::EnumerateFileSystemEntries($dir)){
            [SupermoonSetup.Native]::NoReparse($p)
            if([IO.Directory]::Exists($p)){$pending.Enqueue($p)}else{
                $rel=$p.Substring($Root.Length+1).Replace('\','/')
                if($rel -cne 'CONTENTS.json'){$all[$rel]=Get-SMManualIdentity (Get-SMManualFile $p)}
            }
            if($all.Count+$pending.Count -gt 256){throw 'Unexpectedly large preparation folder.'}
        }
    }
    return ,$all
}
function Assert-SMManualPrepared($C,$Root){
    $m=& $script:Core {param($p) Read-SMJson $p} (Join-Path $Root 'CONTENTS.json')
    if($m['schema'] -ne 1 -or $m['release'] -cne '1.2.1' -or $m['manual_revision'] -ne 1){throw 'Unsupported manual-preparation manifest.'}
    $expectedGame=@{bytes=$C.Recipe['game_executable_bytes'];sha256=$C.Recipe['game_executable_sha256']}
    if(-not(Test-SMManualEqual $m['game_identity'] $expectedGame)){throw 'Preparation executable identity mismatch.'}
    if(-not(Test-SMManualEqual $m['files'] (Get-SMManualContents $Root))){throw 'Preparation contents changed or are incomplete. Do not copy them.'}
    foreach($k in $script:ManualPaths.Keys){
        $id=Get-SMManualIdentity (Get-SMManualFile (Join-Path $Root ('COPY_TO_GAME/'+$script:ManualPaths[$k])))
        if(-not(Test-SMManualEqual $id $C.Expected[$k])){throw ('Prepared '+$k+' identity mismatch.')}
    }
    $config='COPY_TO_GAME/Binaries/Supermoon/playlist.ini'
    if($m['configuration'] -ceq 'create-default-if-still-absent'){
        if(-not(Test-SMManualEqual $m['files'][$config] $C.Recipe['configuration_defaults'])){throw 'Prepared INI identity mismatch.'}
    }elseif($m['configuration'] -ceq 'preserve-existing-do-not-copy-default'){
        if($m['files'].ContainsKey($config)){throw 'Existing personal INI must not be overwritten by preparation.'}
    }else{throw 'Unknown manual configuration policy.'}
    # The copy subtree is a fixed allowlist. A changed manifest cannot authorize
    # another DLL or an arbitrary file to be copied into the game.
    $allowed=@($script:ManualPaths.Values | ForEach-Object {'COPY_TO_GAME/'+$_})
    if($m['configuration'] -ceq 'create-default-if-still-absent'){$allowed+=,$config}
    foreach($rel in $m['files'].Keys){if($rel.StartsWith('COPY_TO_GAME/',[StringComparison]::OrdinalIgnoreCase) -and $rel -cnotin $allowed){throw 'Unexpected file in copy subtree.'}}
    return ,$m
}
function New-SMManualFiles($C,$Package,$Output){
    Assert-SMManualFresh $C
    $beforeIni=Get-SMManualIni $C
    $desired=@{
        proxy=(Get-SMManualFile (Join-Path $Package 'winmm.dll'))
        payload=(Get-SMManualFile (Join-Path $Package 'Supermoon/supermoon_playlist.dll'))
        marker=(Get-SMManualFile (Join-Path $Package 'bytebeat-playlist-experimental.enabled'))
    }
    foreach($k in @('proxy','payload','marker')){if(-not(Test-SMManualEqual (Get-SMManualIdentity $desired[$k]) $C.Expected[$k])){throw ('Wrong package '+$k+'; nothing exported.')}}
    $template=Get-SMManualFile (Join-Path $Package 'Supermoon/playlist.ini') 65536
    if(-not(Test-SMManualEqual (Get-SMManualIdentity $template) $C.Recipe['configuration_defaults'])){throw 'Packaged INI is not the verified complete default.'}
    $desired.bank=& $script:Core {param($c) ,(Get-SMAudioBank $c)} $C
    # Resolve the existing parent before admission, including short-name aliases.
    $requested=[SupermoonSetup.Native]::PathName($Output);[SupermoonSetup.Native]::NoReparse($requested)
    $parent=Get-SMManualDirectory ([IO.Path]::GetDirectoryName($requested))
    $target=Join-Path $parent ([IO.Path]::GetFileName($requested))
    Assert-SMManualSeparate $target $C.Game;Assert-SMManualSeparate $target $Package
    foreach($tempPath in @([IO.Path]::GetTempPath(),$env:TEMP,$env:TMP) | Select-Object -Unique){
        if(-not [string]::IsNullOrWhiteSpace($tempPath)){
            $temp=Get-SMManualDirectory $tempPath
            if(& $script:Core {param($p,$t) Test-SMWithin $p $t} $target $temp){throw 'Use a persistent preparation folder outside TEMP/TMP.'}
        }
    }
    if([IO.File]::Exists($target) -or [IO.Directory]::Exists($target)){throw 'Output already exists. Choose a new folder; nothing will be overwritten.'}
    $building=Join-Path $parent ('.supermoon-manual-INCOMPLETE-'+[guid]::NewGuid().ToString('N'))
    [SupermoonSetup.Native]::NoReparse($building);[void][IO.Directory]::CreateDirectory($building)
    try {
        foreach($k in $script:ManualPaths.Keys){
            Write-SMManualFile $building ('COPY_TO_GAME/'+$script:ManualPaths[$k]) $desired[$k]
            Invoke-SMManualCheckpoint ('exported:'+$k)
        }
        $policy='preserve-existing-do-not-copy-default'
        if($null -eq $beforeIni){$policy='create-default-if-still-absent';Write-SMManualFile $building 'COPY_TO_GAME/Binaries/Supermoon/playlist.ini' $template}
        Write-SMManualFile $building 'DEFAULT_CONFIGURATION/playlist.ini' $template
        [void][IO.Directory]::CreateDirectory((Join-Path $building 'COPY_TO_GAME/Binaries/Supermoon/Music'))
        foreach($rel in @('MANUAL_INSTALL.md','LICENSE','COPYING','LICENSING.md','THIRD_PARTY_NOTICES.md','ASSET_NOTICE.md')){
            $bytes=Get-SMManualFile (Join-Path $Package ('Supermoon/'+$rel));if($null -eq $bytes){throw ('Required instructions/license missing: '+$rel)}
            Write-SMManualFile $building $rel $bytes
        }
        $licenses=Join-Path $Package 'Supermoon/licenses';[SupermoonSetup.Native]::NoReparse($licenses)
        foreach($p in [IO.Directory]::EnumerateFiles($licenses)){
            Write-SMManualFile $building ('licenses/'+[IO.Path]::GetFileName($p)) (Get-SMManualFile $p)
        }
        $readme=@'
Supermoon v1.2.1 - locally prepared manual installation
Read MANUAL_INSTALL.md. Preparation did not install anything into NMS.
Keep NMS closed while copying or removing files. This is for a fresh inactive game.
Copy only the contents of COPY_TO_GAME, in the order specified by the guide.
Copy the activation marker last. Never overwrite unknown files or personal INI.
If playlist.ini exists at copy time, keep it; defaults are in DEFAULT_CONFIGURATION.
After copying, use ManualSetup.cmd Verify with this folder and the same game.
No automatic recovery records are created. Retain this folder for verification.
Do not upload/share this locally generated bank or preparation folder.
For an existing automatic installation or v1.2.0 upgrade use matching Setup.cmd.
'@
        Write-SMManualFile $building 'READ_THIS_FIRST.txt' ([Text.UTF8Encoding]::new($false).GetBytes($readme+"`r`n"))
        $manifest=@{schema=1;release='1.2.1';manual_revision=1;game_identity=@{bytes=$C.Recipe['game_executable_bytes'];sha256=$C.Recipe['game_executable_sha256']};configuration=$policy;files=(Get-SMManualContents $building)}
        $raw=& $script:Core {param($v) ,(ConvertTo-SMBytes $v)} $manifest
        Write-SMManualFile $building 'CONTENTS.json' $raw
        [void](Assert-SMManualPrepared $C $building)
        Invoke-SMManualCheckpoint 'before-publish'
        Assert-SMManualFresh $C
        if(-not(Test-SMManualEqual $beforeIni (Get-SMManualIni $C))){throw 'Personal INI changed during preparation; prepare again into a new folder.'}
        [SupermoonSetup.Native]::NoReparse($target);[SupermoonSetup.Native]::NoReparse($building)
        [IO.Directory]::Move($building,$target) # Same parent, no replacement of an existing output.
        return @{result='MANUAL_FILES_PREPARED';release='1.2.1';manual_revision=1;output=$target;configuration=$policy;game_writes_attempted=$false}
    }catch{throw ('Preparation incomplete; do not copy '+$building+'. Game was not written. '+$_.Exception.Message)}
}
function Invoke-SMManualOperation {
    param([string]$Game,[string]$Package,[string]$Output,[string]$Action='prepare')
    if($Action -cnotin @('prepare','verify','removal-check')){throw 'Unknown manual action.'}
    $packageRoot=Get-SMManualDirectory $Package;$gameRoot=Get-SMManualDirectory $Game
    Assert-SMManualSeparate $packageRoot $gameRoot
    $recipePath=Join-Path $packageRoot 'Supermoon/build/recipe.json'
    $planPath=Join-Path $packageRoot 'Supermoon/tools/audio_patch.json'
    # Companion revision 1 is bound to the exact published policy, not a
    # user-editable recipe that could authorize arbitrary executable/DLL bytes.
    if((Get-SMManualIdentity (Get-SMManualFile $recipePath 65536)).sha256 -cne 'f1a45789e9f6510c45d9c0854118a1ca51c6816ad79497e336a4ea51dc57b52a' -or
       (Get-SMManualIdentity (Get-SMManualFile $planPath 65536)).sha256 -cne 'e094e3f6d8d6820528fa37e05f98dfc32d3b12b1a87266af00b0adf8849c3621'){
        throw 'Manual revision 1 requires the unchanged published v1.2.1 recipe and audio patch policy.'
    }
    $recipe=& $script:Core {param($p) Read-SMJson $p} $recipePath
    $plan=& $script:Core {param($p) Read-SMJson $p} $planPath
    if($recipe['version'] -cne '1.2.1' -or $recipe['steam_build_id'] -ne 26540391){throw 'Manual revision 1 supports only Supermoon 1.2.1 / Steam build 26540391.'}
    $c=& $script:Core {param($g,$r,$p) New-SMContext $g $r $p} $gameRoot $recipe $plan
    if($Action -ceq 'prepare'){return New-SMManualFiles $c $packageRoot $Output}
    $prepared=Get-SMManualDirectory $Output
    Assert-SMManualSeparate $prepared $gameRoot;Assert-SMManualSeparate $prepared $packageRoot
    [void](Assert-SMManualPrepared $c $prepared)
    & $script:Core {Assert-SMClosed}
    Assert-SMManualNoRecords $c
    if($Action -ceq 'verify'){& $script:Core {param($c) Assert-SMGame $c;Assert-SMNoLooseConflict $c} $c}
    $observed=Get-SMManualObserved $c;$present=0
    foreach($k in $script:ManualPaths.Keys){
        if($null -ne $observed[$k]){
            if(-not(Test-SMManualEqual $observed[$k] $c.Expected[$k])){throw ('Unknown/modified '+$k+'; do not overwrite or delete it.')}
            $present++
        }
    }
    if($Action -ceq 'verify' -and $present -gt 0 -and $present -lt 4){throw 'Incomplete manual copy. Keep NMS closed; remove recognized files following the guide, or complete the copy and verify.'}
    if($Action -ceq 'verify' -and $present -eq 4 -and $null -eq (Get-SMManualIni $c)){throw 'playlist.ini is missing; create it from DEFAULT_CONFIGURATION, then verify again.'}
    $result=if($Action -ceq 'removal-check'){'MANUAL_REMOVAL_CHECK_PASSED'}elseif($present -eq 4){'VERIFIED_MANUAL_INSTALL'}else{'VERIFIED_INACTIVE'}
    return @{result=$result;present=$present;observed=$observed;game_writes_attempted=$false;configuration='Personal INI is user-owned; never remove or overwrite it. No automatic recovery records created.'}
}
Export-ModuleMember -Function Invoke-SMManualOperation
