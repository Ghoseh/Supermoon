# SPDX-License-Identifier: MIT
# Explicit public 1.2.0 -> 1.2.1 transition. Legacy records remain in place.
function Get-SMUpgradePath($C){Join-Path $C.Store 'UPGRADE-1.2.0.json'}
function Test-SMUpgradePending($C){$C.Recipe['version'] -ceq '1.2.1' -and [IO.File]::Exists((Get-SMUpgradePath $C))}
function Get-SMLegacy120($C,$AllowUnsupported=$false){
    if($C.Recipe['version'] -cne '1.2.1'){throw 'Upgrade supports only public 1.2.0 -> 1.2.1.'}
    $dir=Join-Path $PSScriptRoot 'legacy120'
    $pins=@{
      'Supermoon120.psm1'=@{bytes=24472L;sha256='4285edd47f50a849fe755358a47fc1b20fddefe3aaf49d2f85826e8394b4c28f'}
      'Native.cs'=@{bytes=11734L;sha256='aa603e5b64bd8a574db2ff956660002d924bba354950dbb10f8fea1bdf53cd26'}
      'recipe.json'=@{bytes=2572L;sha256='24bbc93788c60cbcc8c78a5f1e94de3efe655ab587c8562251656a5f6c66c8a1'}
      'audio_patch.json'=@{bytes=4440L;sha256='e094e3f6d8d6820528fa37e05f98dfc32d3b12b1a87266af00b0adf8849c3621'}
    }
    foreach($n in $pins.Keys){if(-not(Test-SMEqual (Get-SMCurrent (Join-Path $dir $n)) $pins[$n])){throw ('Matching public v1.2.0 tool identity mismatch: '+$n)}}
    $legacy=Import-Module (Join-Path $dir 'Supermoon120.psm1') -Prefix Legacy120 -PassThru -Force -Scope Local
    $recipe=Read-SMJson (Join-Path $dir 'recipe.json');$plan=Read-SMJson (Join-Path $dir 'audio_patch.json')
    $old=& $legacy {param($g,$r,$p) New-SMContext $g $r $p} $C.Game $recipe $plan
    if(-not $AllowUnsupported){& $legacy {param($c) Assert-SMGame $c} $old}
    Invoke-SMCheckpoint 'upgrade:legacy_loaded'
    return @{Module=$legacy;Context=$old}
}
function Get-SMTree($Root){
    [SupermoonSetup.Native]::NoReparse($Root);if(-not[IO.Directory]::Exists($Root)){throw 'Recovery tree is missing.'}
    $result=@{};$pending=New-Object 'Collections.Generic.Stack[string]';$pending.Push($Root);$total=0L;$directories=0
    while($pending.Count){
        if(++$directories -gt 128){throw 'Recovery directory bound exceeded'}
        $dir=$pending.Pop();[SupermoonSetup.Native]::NoReparse($dir)
        foreach($p in [IO.Directory]::EnumerateFileSystemEntries($dir)){
            [SupermoonSetup.Native]::NoReparse($p)
            if([IO.Directory]::Exists($p)){$pending.Push($p);if($pending.Count -gt 128){throw 'Recovery directory bound exceeded'};continue}
            $rel=$p.Substring($Root.Length+1).Replace('\','/')
            $id=Get-SMCurrent $p;if($null -eq $id){throw 'Recovery file disappeared.'}
            $total+=$id['bytes'];if($result.Count -ge 128 -or $total -gt 536870912){throw 'Recovery archive size bound exceeded; retain records for review.'}
            $result[$rel]=$id
        }
    }
    return $result
}
function Get-SMOldRecord120($Legacy){
    $old=$Legacy.Context;$module=$Legacy.Module
    $record=& $module {param($c) Get-SMRecord $c $null} $old
    $tree=Get-SMTree $old.Store
    foreach($n in $tree.Keys){if($n -cnotmatch '^(ACTIVE\.json|records/[0-9a-f]{32}/(manifest\.json|manifest\.sha256|journal\.json|(new|prior)-[0-3]\.bin))$'){throw ('Unrecognized legacy recovery file; preserve for review: '+$n)}}
    $records=Join-Path $old.Store 'records'
    foreach($d in [IO.Directory]::EnumerateDirectories($records)){
        $id=[IO.Path]::GetFileName($d);if($id -cnotmatch '^[0-9a-f]{32}$'){throw 'Unknown legacy recovery directory.'}
        [void](& $module {param($c,$id) Get-SMRecord $c $id} $old $id)
    }
    [void](& $module {param($c,$m) Assert-SMLive $c $m} $old $record.Manifest)
    return $record
}
function Get-SMUpgradeJournal($C,$OldRecord){
    $p=Get-SMUpgradePath $C
    if($null -eq (Read-SMFile $p 65536)){return $null}
    $j=Read-SMJson $p
    Assert-SMKeys $j @('schema','from','to','game','old_record','old_manifest_sha256','new_expected','phase')
    if($j['schema'] -isnot [long] -or $j['schema'] -ne 1 -or $j['phase'] -isnot [string] -or $j['from'] -cne '1.2.0' -or $j['to'] -cne '1.2.1' -or $j['game'] -cne $C.Game -or $j['old_record'] -cne $OldRecord.Manifest['record'] -or $j['old_manifest_sha256'] -cne (Get-SMCurrent (Join-Path $OldRecord.Folder 'manifest.json'))['sha256'] -or -not(Test-SMEqual $j['new_expected'] $C.Expected) -or $j['phase'] -cnotin @('prepared','disabled120','archived120','installing121','complete','inactive')){throw 'Upgrade journal target/version/identity mismatch; retain records.'}
    return $j
}
function Set-SMUpgradePhase($C,$Journal,$Phase){$Journal['phase']=$Phase;Write-SMJson (Get-SMUpgradePath $C) $Journal;Invoke-SMCheckpoint ('upgrade:'+ $Phase)}
function Assert-SMAbsent($C){if(@((Get-SMObserved $C).Values | Where-Object {$null -ne $_}).Count){throw 'Upgrade needs all four managed resources absent.'}}
function Save-SMOldArchive120($C,$Legacy,$Record){
    $source=$Legacy.Context.Store;$tree=Get-SMTree $source
    $root=Join-Path $C.Store ('archives/v1.2.0-'+$Record.Manifest['record']);[SupermoonSetup.Native]::NoReparse($root)
    foreach($rel in ($tree.Keys | Sort-Object)){
        $to=Join-Path $root $rel;$raw=Read-SMFile (Join-Path $source $rel)
        if(-not(Test-SMEqual (Get-SMIdentity $raw) $tree[$rel])){throw 'Legacy archive source changed.'}
        $existing=Get-SMCurrent $to
        if($null -eq $existing){[SupermoonSetup.Native]::NoReparse($to);[void][IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($to));Write-SMNew $to $raw}
        elseif(-not(Test-SMEqual $existing $tree[$rel])){throw 'Previous archive copy differs; do not overwrite it.'}
        Invoke-SMCheckpoint 'upgrade:archive_file'
    }
    if(-not(Test-SMEqual (Get-SMTree $root) $tree) -or -not(Test-SMEqual (Get-SMTree $source) $tree)){throw 'Legacy archive verification failed.'}
    $index=Join-Path $C.Store 'ARCHIVE-1.2.0.json'
    if($null -ne (Read-SMFile $index 65536)){
        if(-not(Test-SMEqual (Read-SMJson $index) $tree)){throw 'Archived record identities changed.'}
    }else{Write-SMNew $index (ConvertTo-SMBytes $tree)}
    return $root
}
function Get-SMUpgradeDesired($C,$Package){
    if(-not $Package){throw 'Upgrade/resume needs the complete separately extracted v1.2.1 package.'}
    $root=[SupermoonSetup.Native]::PathName($Package);[SupermoonSetup.Native]::NoReparse($root)
    if((Test-SMWithin $root $C.Game) -or (Test-SMWithin $C.Game $root)){throw 'Extract the package outside the game tree.'}
    $desired=@{proxy=(Read-SMFile (Join-Path $root 'winmm.dll'));payload=(Read-SMFile (Join-Path $root 'Supermoon/supermoon_playlist.dll'));marker=(Read-SMFile (Join-Path $root 'bytebeat-playlist-experimental.enabled'))}
    foreach($k in @('proxy','payload','marker')){if(-not(Test-SMEqual (Get-SMIdentity $desired[$k]) $C.Expected[$k])){throw ('Wrong v1.2.1 package '+$k)}}
    $desired.bank=Get-SMAudioBank $C
    return $desired
}
function Invoke-SMUpgrade120($C,$Package){
    Assert-SMClosed;Assert-SMGame $C;Assert-SMNoLooseConflict $C
    $temp=[SupermoonSetup.Native]::DirectoryIdentity([IO.Path]::GetTempPath());$game=[SupermoonSetup.Native]::DirectoryIdentity($C.Game)
    if(Test-SMWithin ([IO.Path]::Combine($game,[IO.Path]::GetFileName($C.Store))) $temp){throw 'Recovery storage must be outside TEMP.'}
    $desired=Get-SMUpgradeDesired $C $Package # Verify complete candidate BEFORE disabling anything.
    $C.ConfigTemplate=Get-SMConfigTemplate $C $Package
    $legacy=Get-SMLegacy120 $C
    $lock=[SupermoonSetup.Native]::WriterLock((Join-Path $C.Game '.supermoon-install.lock'))
    try{
        # With a new active record, resume only its own pinned pair; never run old Disable.
        if($null -ne (Get-SMActive $C)){
            if(-not(Test-SMUpgradePending $C)){throw 'Existing v1.2.1 record; use its normal Update/Resume.'}
            $j=Read-SMJson (Get-SMUpgradePath $C)
            Assert-SMKeys $j @('schema','from','to','game','old_record','old_manifest_sha256','new_expected','phase')
            if($j['schema'] -isnot [long] -or $j['schema'] -ne 1 -or $j['from'] -cne '1.2.0' -or $j['to'] -cne '1.2.1' -or $j['old_record'] -isnot [string] -or $j['old_record'] -cnotmatch '^[0-9a-f]{32}$' -or $j['old_manifest_sha256'] -isnot [string] -or $j['old_manifest_sha256'] -cnotmatch '^[0-9a-f]{64}$' -or $j['phase'] -isnot [string] -or $j['game'] -cne $C.Game -or -not(Test-SMEqual $j['new_expected'] $C.Expected) -or $j['phase'] -cnotin @('installing121','complete')){throw 'Inconsistent upgrade completion state.'}
            $r=Get-SMRecord $C $null;$configuration=Invoke-SMConfigDefaults $C;$result=Invoke-SMApply $C $r 'install';if($null -ne $configuration){$result.configuration=$configuration};Set-SMUpgradePhase $C $j 'complete';return $result
        }
        $record=Get-SMOldRecord120 $legacy;$j=Get-SMUpgradeJournal $C $record
        if($null -eq $j){
            $observed=Get-SMObserved $C
            $allAbsent=@($observed.Values|Where-Object {$null -ne $_}).Count -eq 0
            if(-not $allAbsent -and -not(Test-SMEqual $observed $legacy.Context.Expected)){throw 'Partial v1.2.0 installation: recover/Disable with matching old tools before Upgrade.'}
            [SupermoonSetup.Native]::NoReparse($C.Store);[void][IO.Directory]::CreateDirectory($C.Store)
            $j=@{schema=1;from='1.2.0';to='1.2.1';game=$C.Game;old_record=$record.Manifest['record'];old_manifest_sha256=(Get-SMCurrent (Join-Path $record.Folder 'manifest.json'))['sha256'];new_expected=$C.Expected;phase='prepared'}
            Write-SMNew (Get-SMUpgradePath $C) (ConvertTo-SMBytes $j);Invoke-SMCheckpoint 'upgrade:prepared'
        }
        if($j['phase'] -cin @('prepared','inactive')){
            $module=$legacy.Module
            if(@((Get-SMObserved $C).Values|Where-Object {$null -ne $_}).Count){[void](& $module {param($c,$r) Invoke-SMApply $c $r 'disable'} $legacy.Context $record)}
            Assert-SMAbsent $C;Set-SMUpgradePhase $C $j 'disabled120'
        }
        Assert-SMAbsent $C
        $archive=Save-SMOldArchive120 $C $legacy $record;Set-SMUpgradePhase $C $j 'archived120'
        # All-null prior state creates independent new recovery; old records stay unchanged.
        Set-SMUpgradePhase $C $j 'installing121';$r=New-SMRecord $C $desired
        $configuration=Invoke-SMConfigDefaults $C;$result=Invoke-SMApply $C $r 'install';if($null -ne $configuration){$result.configuration=$configuration};Set-SMUpgradePhase $C $j 'complete'
        $result['legacy_archive']=$archive;return $result
    }finally{$lock.Dispose()}
}
function Stop-SMUpgrade120($C){
    Assert-SMClosed;$legacy=Get-SMLegacy120 $C $true
    $lock=[SupermoonSetup.Native]::WriterLock((Join-Path $C.Game '.supermoon-install.lock'))
    try{
        if($null -ne (Get-SMActive $C)){throw 'Use the new record for Disable/rollback.'}
        $record=Get-SMOldRecord120 $legacy;$j=Get-SMUpgradeJournal $C $record
        if($null -eq $j){throw 'No verified interrupted upgrade.'}
        $module=$legacy.Module
        if($j['phase'] -cin @('prepared','inactive') -and @((Get-SMObserved $C).Values|Where-Object {$null -ne $_}).Count){[void](& $module {param($c,$r) Invoke-SMApply $c $r 'disable'} $legacy.Context $record)}
        Assert-SMAbsent $C;[void](Save-SMOldArchive120 $C $legacy $record);Set-SMUpgradePhase $C $j 'inactive'
        return @{result='disabled';observed=(Get-SMObserved $C);recovery=$C.Store;guidance='Safe inactive transition; no old DLL restoration. Retain both stores and package.'}
    }finally{$lock.Dispose()}
}
