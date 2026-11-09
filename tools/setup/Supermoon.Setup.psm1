# SPDX-License-Identifier: MIT
# Windows PowerShell 5.1 offline four-file setup. No native game/DLL code executes.
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
if ($PSVersionTable.PSEdition -ne 'Desktop' -or $PSVersionTable.PSVersion -lt [version]'5.1') { throw 'Use built-in Windows PowerShell 5.1.' }
if (-not ('SupermoonSetup.Native' -as [type])) { Add-Type -LiteralPath (Join-Path $PSScriptRoot 'Native.cs') }
$script:Keys = @('proxy','payload','bank','marker') # Backup numbers are the original Python schema order.
$script:Paths = @{proxy='Binaries/winmm.dll';payload='Binaries/Supermoon/supermoon_playlist.dll';bank='GAMEDATA/MODS/Supermoon_MenuAudio_Test/audio/windows/init.bnk';marker='Binaries/bytebeat-playlist-experimental.enabled'}
$script:Marker = [Text.Encoding]::ASCII.GetBytes("ByteBeat automatic startup v1`r`n")
$script:MenuSha = '0db1fbc668384e7fb618009f09e889dd32b6b3b3b0b9806f5d099ca8b8ecfc46'
$script:PakSha = '29682a3015f0ffcfa1ac236e4d49a9670e6d2a79e50a25315f8faecf45cde940'
$script:PakBytes = 67842512L
$script:NativeOffset = 2301520L

function Read-SMFile($Path, [long]$Limit=33554432) { return ,([SupermoonSetup.Native]::Read($Path,$Limit)) }
function Get-SMIdentity($Data) {
    if ($null -eq $Data) { return $null }
    $hash=[Security.Cryptography.SHA256]::Create()
    try { return @{bytes=[long]$Data.Length;sha256=([BitConverter]::ToString($hash.ComputeHash([byte[]]$Data))).Replace('-','').ToLowerInvariant()} } finally { $hash.Dispose() }
}
function Get-SMCurrent($Path,[long]$Limit=33554432) { Get-SMIdentity (Read-SMFile $Path $Limit) }
function Test-SMEqual($Left,$Right) {
    if ($null -eq $Left -or $null -eq $Right) { return ($null -eq $Left -and $null -eq $Right) }
    if ($Left -is [Collections.IDictionary] -or $Right -is [Collections.IDictionary]) {
        if ($Left -isnot [Collections.IDictionary] -or $Right -isnot [Collections.IDictionary]) { return $false }
        if ($Left.Count -ne $Right.Count) { return $false }
        foreach ($k in $Left.Keys) { if (-not $Right.ContainsKey($k) -or -not (Test-SMEqual $Left[$k] $Right[$k])) { return $false } };return $true
    }
    if ($Left -is [string] -or $Right -is [string]) { return ($Left -is [string] -and $Right -is [string] -and $Left -ceq $Right) }
    if ($Left -is [bool] -or $Right -is [bool]) { return ($Left -is [bool] -and $Right -is [bool] -and $Left -eq $Right) }
    if ($Left -is [Collections.IList] -or $Right -is [Collections.IList]) { return $false }
    return ($Left -eq $Right)
}
function Read-SMJson($Path) { return ,([SupermoonSetup.Json]::Parse((Read-SMFile $Path 65536))) }
function ConvertTo-SMBytes($Value) { return ,([Text.UTF8Encoding]::new($false,$true).GetBytes((ConvertTo-Json -InputObject $Value -Depth 24 -Compress)+"`n")) }
function Write-SMNew($Path,$Data) {
    [SupermoonSetup.Native]::WriteNew($Path,$Data)
    if (-not (Test-SMEqual (Get-SMCurrent $Path) (Get-SMIdentity $Data))) { throw "Durable backup readback failed: $Path" }
}
function Write-SMJson($Path,$Value) {
    [SupermoonSetup.Native]::NoReparse($Path)
    $raw=ConvertTo-SMBytes $Value;$tmp=Join-Path ([IO.Path]::GetDirectoryName($Path)) ('.record-'+[guid]::NewGuid().ToString('N')+'.tmp')
    try { Write-SMNew $tmp $raw;[SupermoonSetup.Native]::Replace($tmp,$Path)
        if (-not (Test-SMEqual (Get-SMCurrent $Path) (Get-SMIdentity $raw))) { throw 'Record publication readback failed' }
    } finally { if ([IO.File]::Exists($tmp)) { [SupermoonSetup.Native]::NoReparse($tmp);[IO.File]::Delete($tmp) } }
}
function Assert-SMClosed {
    $names=[SupermoonSetup.Native]::ProcessNames()
    if ($names.Count -eq 0 -or @($names | Where-Object { [string]::IsNullOrWhiteSpace($_) -or $_ -ieq 'NMS.exe' }).Count) { throw 'Exit NMS fully before installation/recovery; keep Steam updates and other writers idle.' }
}
function Invoke-SMCheckpoint($Stage) { } # No-op seam. Not controllable by CLI/environment. Owned tests replace this function in module scope.
function Assert-SMKeys($Value,$Keys) {
    if ($Value -isnot [Collections.IDictionary] -or $Value.Count -ne $Keys.Count) { throw 'Invalid object fields' }
    foreach($k in $Keys) { if (-not $Value.ContainsKey($k)) { throw "Missing field: $k" } }
}
function New-SMContext($Game,$Recipe,$Plan) {
    $gamePath=[SupermoonSetup.Native]::PathName($Game);[SupermoonSetup.Native]::NoReparse($gamePath)
    if ($gamePath.Length -le 3 -or -not [IO.Directory]::Exists($gamePath)) { throw 'Select the existing game root containing Binaries and GAMEDATA.' }
    $c=@{Game=$gamePath;Recipe=$Recipe;Plan=$Plan;Store=(Join-Path $gamePath '.Supermoon-Recovery')}
    $c.Expected=@{proxy=$Recipe['DLLs']['winmm.dll'];payload=$Recipe['DLLs']['supermoon_playlist.dll'];bank=@{bytes=$Plan['output_bytes'];sha256=$Plan['output_sha256']};marker=(Get-SMIdentity $script:Marker)}
    # Validate identities before they can authorize target/recovery operations.
    foreach($id in @($c.Expected.Values)+@(@{bytes=$Recipe['game_executable_bytes'];sha256=$Recipe['game_executable_sha256']})) {
        Assert-SMKeys $id @('bytes','sha256')
        if ($id['bytes'] -isnot [long] -and $id['bytes'] -isnot [int]) { throw 'Invalid identity size' }
        if ($id['bytes'] -le 0 -or $id['sha256'] -isnot [string] -or $id['sha256'] -cnotmatch '^[0-9a-f]{64}$') { throw 'Invalid identity' }
    }
    if ($Recipe['version'] -isnot [string] -or [string]::IsNullOrWhiteSpace($Recipe['version'])) { throw 'Invalid release policy' }
    return $c
}
function Get-SMTarget($C,$Key) { Join-Path $C.Game $script:Paths[$Key] }
function Test-SMGame($C) { Test-SMEqual (Get-SMCurrent (Join-Path $C.Game 'Binaries/NMS.exe') 134217728) @{bytes=$C.Recipe['game_executable_bytes'];sha256=$C.Recipe['game_executable_sha256']} }
function Assert-SMGame($C) { if (-not (Test-SMGame $C)) { throw 'Unsupported NMS executable; use disable recovery, never force an old pair.' } }
function Get-SMObserved($C) { $r=@{};foreach($k in $script:Keys) { $r[$k]=Get-SMCurrent (Get-SMTarget $C $k) };return $r }
function Test-SMApproved($C,$Key,$Value) {
    if ($null -eq $Value) { return $true }
    Assert-SMKeys $Value @('bytes','sha256')
    return ((Test-SMEqual $Value $C.Expected[$Key]) -or ($Key -ceq 'bank' -and $Value['sha256'] -ceq $script:MenuSha -and $Value['bytes'] -le 65536 -and $Value['bytes'] -gt 0))
}
function Test-SMCoherent($C,$Before) {
    if (@($script:Keys | Where-Object { $null -ne $Before[$_] }).Count -eq 0) { return $true }
    foreach($k in @('proxy','payload','marker')) { if (-not (Test-SMEqual $Before[$k] $C.Expected[$k])) { return $false } }
    return ($null -eq $Before['bank'] -or (Test-SMEqual $Before['bank'] $C.Expected['bank']))
}
function Get-SMActive($C) {
    $p=Join-Path $C.Store 'ACTIVE.json';if ($null -eq (Read-SMFile $p 65536)) { return $null }
    $a=Read-SMJson $p;Assert-SMKeys $a @('record','manifest_sha256')
    if($a['record'] -isnot [string] -or $a['record'] -cnotmatch '^[0-9a-f]{32}$' -or $a['manifest_sha256'] -isnot [string] -or $a['manifest_sha256'] -cnotmatch '^[0-9a-f]{64}$'){throw 'Invalid active selector field types or identities'}
    return $a
}
function Get-SMRecord($C,$Record) {
    $a=$null;if (-not $Record) { $a=Get-SMActive $C;if ($null -ne $a) { $Record=$a['record'] } }
    if ($Record -isnot [string] -or $Record -cnotmatch '^[0-9a-f]{32}$') { throw 'No valid selected recovery record' }
    $folder=Join-Path $C.Store ('records/'+$Record);[SupermoonSetup.Native]::NoReparse($folder)
    $raw=Read-SMFile (Join-Path $folder 'manifest.json') 65536;$digest=(Get-SMIdentity $raw)['sha256']
    $pin=Read-SMFile (Join-Path $folder 'manifest.sha256') 256
    if ($null -eq $raw -or -not (Test-SMEqual (Get-SMIdentity $pin) (Get-SMIdentity ([Text.Encoding]::ASCII.GetBytes($digest+"`n")))) -or ($null -ne $a -and $a['manifest_sha256'] -cne $digest)) { throw 'Recovery manifest hash mismatch' }
    $m=[SupermoonSetup.Json]::Parse($raw);Assert-SMKeys $m @('schema','record','game','release','game_identity','before','after','utc')
    if ($m['schema'] -isnot [long] -or $m['schema'] -ne 1 -or $m['record'] -isnot [string] -or $m['game'] -isnot [string] -or $m['release'] -isnot [string] -or $m['utc'] -isnot [string] -or $m['record'] -cne $Record -or $m['game'] -cne $C.Game) { throw 'Recovery target/schema mismatch' }
    $gameId=@{bytes=$C.Recipe['game_executable_bytes'];sha256=$C.Recipe['game_executable_sha256']}
    if ($m['release'] -cne $C.Recipe['version'] -or -not (Test-SMEqual $m['game_identity'] $gameId) -or -not (Test-SMEqual $m['after'] $C.Expected)) { throw 'Recovery release/identity policy mismatch; use its original matching tools. Never edit records.' }
    Assert-SMKeys $m['before'] $script:Keys
    for($i=0;$i -lt $script:Keys.Count;$i++) {
        $k=$script:Keys[$i];if (-not (Test-SMApproved $C $k $m['before'][$k])) { throw 'Unrecognized prior identity in record' }
        foreach($prefix in @('prior','new')) {
            $values=if($prefix -ceq 'prior'){$m['before']}else{$m['after']}
            if (-not (Test-SMEqual (Get-SMCurrent (Join-Path $folder ($prefix+'-'+$i+'.bin'))) $values[$k])) { throw "Recovery backup corrupt/missing: $prefix $k" }
        }
    }
    return @{Folder=$folder;Manifest=$m}
}
function Test-SMWithin($Path,$Root) { return ($Path.Equals($Root,[StringComparison]::OrdinalIgnoreCase) -or $Path.StartsWith($Root.TrimEnd('\')+'\',[StringComparison]::OrdinalIgnoreCase)) }
function New-SMRecord($C,$Desired) {
    Assert-SMKeys $Desired $script:Keys;$ids=@{};foreach($k in $script:Keys){$ids[$k]=Get-SMIdentity $Desired[$k]}
    if (-not (Test-SMEqual $ids $C.Expected)) { throw 'Candidate does not match exact DLL/marker/bank identities' }
    [SupermoonSetup.Native]::NoReparse($C.Store)
    # Reject TEMP junctions before following them. Compare final directory names to
    # catch short-name aliases as well, without rebinding any existing record path.
    $temp=[SupermoonSetup.Native]::DirectoryIdentity([IO.Path]::GetTempPath())
    $game=[SupermoonSetup.Native]::DirectoryIdentity($C.Game)
    if (Test-SMWithin ([IO.Path]::Combine($game,'.Supermoon-Recovery')) $temp) { throw 'Recovery storage must be outside TEMP.' }
    $a=Get-SMActive $C
    if ($null -ne $a) { $r=Get-SMRecord $C $null;$j=Read-SMJson (Join-Path $r.Folder 'journal.json');if ($j['phase'] -cnotin @('rolled_back','disabled')) { return $r } }
    $data=@{};$before=@{};foreach($k in $script:Keys){$data[$k]=Read-SMFile (Get-SMTarget $C $k);$before[$k]=Get-SMIdentity $data[$k];if (-not (Test-SMApproved $C $k $before[$k])){throw "Unknown installed $k; preserve it and resolve the conflict."}}
    $name=[guid]::NewGuid().ToString('N');$folder=Join-Path $C.Store ('records/'+$name);[SupermoonSetup.Native]::NoReparse($folder);[void][IO.Directory]::CreateDirectory($folder)
    for($i=0;$i -lt $script:Keys.Count;$i++){$k=$script:Keys[$i];Invoke-SMCheckpoint ('before_backup:'+$k);if($null -ne $data[$k]){Write-SMNew (Join-Path $folder ('prior-'+$i+'.bin')) $data[$k]};Write-SMNew (Join-Path $folder ('new-'+$i+'.bin')) $Desired[$k]}
    $m=@{schema=1;record=$name;game=$C.Game;release=$C.Recipe['version'];game_identity=@{bytes=$C.Recipe['game_executable_bytes'];sha256=$C.Recipe['game_executable_sha256']};before=$before;after=$C.Expected;utc=[DateTime]::UtcNow.ToString('o')}
    $raw=ConvertTo-SMBytes $m;Write-SMNew (Join-Path $folder 'manifest.json') $raw;$digest=(Get-SMIdentity $raw)['sha256'];Write-SMNew (Join-Path $folder 'manifest.sha256') ([Text.Encoding]::ASCII.GetBytes($digest+"`n"))
    Write-SMJson (Join-Path $folder 'journal.json') @{phase='prepared';observed=$before}
    $r=Get-SMRecord $C $name;Write-SMJson (Join-Path $C.Store 'ACTIVE.json') @{record=$name;manifest_sha256=$digest};Invoke-SMCheckpoint 'prepared';return $r
}
function Assert-SMLive($C,$M) {
    $o=Get-SMObserved $C;foreach($k in $script:Keys){if($null -ne $o[$k] -and -not (Test-SMEqual $o[$k] $M['before'][$k]) -and -not (Test-SMEqual $o[$k] $M['after'][$k])){throw "Unknown current $k; no recovery overwrite allowed."}};return $o
}
function Set-SMTarget($C,$Key,$Data,$M) {
    Assert-SMClosed;$path=Get-SMTarget $C $Key;[SupermoonSetup.Native]::NoReparse($path);$actual=Get-SMCurrent $path
    if($null -ne $actual -and -not (Test-SMEqual $actual $M['before'][$Key]) -and -not (Test-SMEqual $actual $M['after'][$Key])){throw "Destination changed: $Key"}
    $wanted=Get-SMIdentity $Data;if(Test-SMEqual $actual $wanted){return}
    if($null -eq $Data){Invoke-SMCheckpoint ('before_remove:'+$Key);[SupermoonSetup.Native]::NoReparse($path);[IO.File]::Delete($path);Invoke-SMCheckpoint ('after_remove:'+$Key)}
    else {
        $parent=[IO.Path]::GetDirectoryName($path);[SupermoonSetup.Native]::NoReparse($parent);[void][IO.Directory]::CreateDirectory($parent);[SupermoonSetup.Native]::NoReparse($parent)
        $tmp=Join-Path $parent ('.supermoon-'+$M['record']+'-'+$Key+'-'+[guid]::NewGuid().ToString('N')+'.tmp')
        try {Invoke-SMCheckpoint ('before_scratch:'+$Key);Write-SMNew $tmp $Data;Assert-SMClosed;[SupermoonSetup.Native]::NoReparse($path)
            if(-not(Test-SMEqual (Get-SMCurrent $path) $actual)){throw "Destination changed before replacement: $Key"}
            Invoke-SMCheckpoint ('before_replace:'+$Key);[SupermoonSetup.Native]::Replace($tmp,$path);Invoke-SMCheckpoint ('after_replace:'+$Key)
        } finally {if([IO.File]::Exists($tmp)){[SupermoonSetup.Native]::NoReparse($tmp);[IO.File]::Delete($tmp)}}
    }
    if(-not(Test-SMEqual (Get-SMCurrent $path) $wanted)){throw "Post-commit verification failed: $Key"}
}
function Invoke-SMApply($C,$R,$Action) {
    $m=$R.Manifest;$folder=$R.Folder;[void](Assert-SMLive $C $m)
    if($Action -cne 'disable'){Assert-SMGame $C}
    if($Action -ceq 'rollback' -and -not(Test-SMCoherent $C $m['before'])){throw 'Prior DLL/marker/bank state is not qualified for rollback; use disable and retain backups.'}
    $values=if($Action -ceq 'install'){$m['after']}elseif($Action -ceq 'rollback'){$m['before']}else{@{proxy=$null;payload=$null;bank=$null;marker=$null}}
    $progress=@{install='installing';rollback='rolling_back';disable='disabling'}[$Action]
    Write-SMJson (Join-Path $folder 'journal.json') @{phase=$progress;observed=(Get-SMObserved $C)}
    Set-SMTarget $C 'marker' $null $m
    for($i=0;$i -lt $script:Keys.Count;$i++){
        $k=$script:Keys[$i];if($k -ceq 'marker'){continue}
        Write-SMJson (Join-Path $folder 'journal.json') @{phase=$progress;next=$k;observed=(Get-SMObserved $C)}
        $data=$null;if($null -ne $values[$k]){$prefix=if($Action -ceq 'install'){'new'}else{'prior'};$data=Read-SMFile (Join-Path $folder ($prefix+'-'+$i+'.bin'))}
        if(-not(Test-SMEqual (Get-SMIdentity $data) $values[$k])){throw "Saved replacement changed before use: $k"}
        Set-SMTarget $C $k $data $m
    }
    $markerData=if($null -ne $values['marker']){,$script:Marker}else{$null};Set-SMTarget $C 'marker' $markerData $m
    if(-not(Test-SMEqual (Get-SMObserved $C) $values)){throw 'Final installation verification failed'}
    Invoke-SMCheckpoint 'before_verified_journal';$phase=@{install='verified';rollback='rolled_back';disable='disabled'}[$Action]
    Write-SMJson (Join-Path $folder 'journal.json') @{phase=$phase;observed=$values}
    return @{result=$phase;record=$m['record'];recovery=$folder;observed=$values}
}
function Invoke-SMMutation($C,$Action,$Desired=$null,$Record=$null) {
    Assert-SMClosed;[SupermoonSetup.Native]::NoReparse($C.Store);$lock=[SupermoonSetup.Native]::WriterLock((Join-Path $C.Game '.supermoon-install.lock'))
    try {
        if($Action -cin @('install','update')){Assert-SMGame $C;$r=New-SMRecord $C $Desired;$Action='install'}
        else {$r=Get-SMRecord $C $Record;[void](Assert-SMLive $C $r.Manifest);if($Record){$digest=(Get-SMCurrent (Join-Path $r.Folder 'manifest.json'))['sha256'];Write-SMJson (Join-Path $C.Store 'ACTIVE.json') @{record=$Record;manifest_sha256=$digest}}}
        try {$apply=if($Action -ceq 'resume'){'install'}else{$Action};return Invoke-SMApply $C $r $apply}
        catch { $message=$_.Exception.Message;try{Write-SMJson (Join-Path $r.Folder 'journal.json') @{phase='failed';action=$Action;error=$message;observed=(Get-SMObserved $C)}}catch{}
            throw ('Operation incomplete; bank may already be replaced. Keep NMS closed. Recovery: '+$r.Folder+'; '+$message)
        }
    } finally {$lock.Dispose()}
}
function Get-SMStatus($C,$Record=$null) {
    # Status may report an unreadable/unsupported executable. Activation still uses
    # Assert-SMGame; verified Disable must remain reachable after a game update.
    $supported=$false;$gameError=$null
    try{$supported=Test-SMGame $C}catch{$gameError=$_.Exception.Message}
    $result=@{game=$C.Game;game_supported=$supported;recovery_root=$C.Store;observed=(Get-SMObserved $C);records=@()}
    if($gameError){$result.game_inspection_error=$gameError}
    $records=Join-Path $C.Store 'records';[SupermoonSetup.Native]::NoReparse($records)
    if([IO.Directory]::Exists($records)){$result.records=@([IO.Directory]::EnumerateDirectories($records) | ForEach-Object {[IO.Path]::GetFileName($_)} | Where-Object {$_ -cmatch '^[0-9a-f]{32}$'})}
    try {if(-not $Record -and $null -eq (Get-SMActive $C)){$result.result='no_selected_record';return $result};$r=Get-SMRecord $C $Record}
    catch {$result.result='CORRUPT_RECORD_REFUSED';$result.error=$_.Exception.Message;return $result}
    try{$journal=Read-SMJson (Join-Path $r.Folder 'journal.json')}catch{$journal=@{phase='UNREADABLE';error=$_.Exception.Message}}
    $files=@{};foreach($k in $script:Keys){$v=$result.observed[$k];$files[$k]=if($null -eq $v){'absent'}elseif(Test-SMEqual $v $r.Manifest['after'][$k]){'candidate'}elseif(Test-SMEqual $v $r.Manifest['before'][$k]){'prior'}else{'UNKNOWN'}}
    $result.record=$r.Manifest['record'];$result.journal=$journal;$result.files=$files;$result.rollback_permitted=((Test-SMCoherent $C $r.Manifest['before']) -and $result.game_supported)
    $result.result=if($files.Values -ccontains 'UNKNOWN'){'UNKNOWN_CURRENT_REFUSED'}elseif(Test-SMEqual $result.observed $r.Manifest['after']){'candidate_present'}elseif(Test-SMEqual $result.observed $r.Manifest['before']){'prior_present'}else{'partial_or_disabled'}
    $scratch=@();foreach($k in $script:Keys){$parent=[IO.Path]::GetDirectoryName((Get-SMTarget $C $k));[SupermoonSetup.Native]::NoReparse($parent);if([IO.Directory]::Exists($parent)){foreach($p in [IO.Directory]::EnumerateFiles($parent,('.supermoon-'+$result.record+'-'+$k+'-*.tmp'))){$id=Get-SMCurrent $p;$scratch+=@{path=$p;identity=$id;matches_candidate=(Test-SMEqual $id $r.Manifest['after'][$k])}}}}
    $result.scratch=$scratch;return $result
}

function Get-SMAudioBank($C) {
    Assert-SMGame $C;$plan=$C.Plan
    $data=Read-SMFile (Join-Path $C.Game 'GAMEDATA/PCBANKS/NMSARC.audioBNK.pak') $script:PakBytes
    if(-not(Test-SMEqual (Get-SMIdentity $data) @{bytes=$script:PakBytes;sha256=$script:PakSha}) -or [Text.Encoding]::ASCII.GetString($data,0,5) -cne 'HGPAK'){throw 'Unsupported native audio archive'}
    $count=[BitConverter]::ToUInt64($data,16);$start=[BitConverter]::ToUInt64($data,40)
    if([BitConverter]::ToUInt64($data,8) -ne 2 -or $data[32] -ne 0 -or $count -le 1 -or $count -ge 10000 -or $start -lt (48+32*$count)){throw 'Unsupported native audio archive layout'}
    $matches=0;for($i=0;$i -lt $count;$i++){$at=48+32*$i;if([BitConverter]::ToUInt64($data,$at+16) -eq $script:NativeOffset -and [BitConverter]::ToUInt64($data,$at+24) -eq $plan['input_bytes']){$matches++}}
    if($matches -ne 1){throw 'Native init entry identity mismatch'}
    $native=New-Object byte[] ([int]$plan['input_bytes']);[Array]::Copy($data,$script:NativeOffset,$native,0,$native.Length)
    return ,(Convert-SMBank $native $plan)
}
function Convert-SMBank($Native,$Plan) {
    if(-not(Test-SMEqual (Get-SMIdentity $Native) @{bytes=$Plan['input_bytes'];sha256=$Plan['input_sha256']})){throw 'Unsupported native init bank'}
    $changed=[byte[]]$Native.Clone()
    foreach($e in $Plan['edits']){
        $at=[long]$e['offset'];$fmt=$e['format'];$size=if($fmt -ceq '<H'){2}else{4}
        if($fmt -cnotin @('<I','<H','<f') -or $at -lt 0 -or $at -gt $Native.Length-$size){throw 'Invalid audio patch operation'}
        $old=switch -CaseSensitive ($fmt){'<I'{[BitConverter]::ToUInt32($Native,$at)}'<H'{[BitConverter]::ToUInt16($Native,$at)}'<f'{[BitConverter]::ToSingle($Native,$at)}}
        if($old -ne $e['expected']){throw 'Audio patch precondition failed'}
        $bytes=switch -CaseSensitive ($fmt){'<I'{[BitConverter]::GetBytes([uint32]$e['value'])}'<H'{[BitConverter]::GetBytes([uint16]$e['value'])}'<f'{[BitConverter]::GetBytes([single]$e['value'])}}
        [Array]::Copy([byte[]]$bytes,0,$changed,$at,$size)
    }
    $positions=@{};foreach($i in @($Plan['insertions'] | Sort-Object { $_['at'] } -Descending)){
        $at=[long]$i['at'];$start=[long]$i['source_offset'];$n=[long]$i['bytes']
        if($positions.ContainsKey($at) -or $at -lt 0 -or $at -gt $Native.Length -or $n -lt 0 -or $start -lt 0 -or $start -gt $Native.Length-$n){throw 'Invalid audio template range'}
        $positions[$at]=$true;$payload=New-Object byte[] ([int]$n);[Array]::Copy($Native,$start,$payload,0,$n)
        foreach($offset in $i['replace_u32'].Keys){$o=[long]$offset;if($o -lt 0 -or $o -gt $n-4){throw 'Invalid template field'};[Array]::Copy([BitConverter]::GetBytes([uint32]$i['replace_u32'][$offset]),0,$payload,$o,4)}
        $next=New-Object byte[] ($changed.Length+[int]$n);[Array]::Copy($changed,0,$next,0,$at);[Array]::Copy($payload,0,$next,$at,$n);[Array]::Copy($changed,$at,$next,$at+$n,$changed.Length-$at);$changed=$next
    }
    if(-not(Test-SMEqual (Get-SMIdentity $changed) @{bytes=$Plan['output_bytes'];sha256=$Plan['output_sha256']})){throw 'Generated audio bank failed exact identity check'};return ,$changed
}
function Assert-SMNoLooseConflict($C) {
    # Conservative whole loose-MODS scan: no settings writes or assumptions about enabled status.
    # Packed priorities remain unobservable, as with the original contract.
    $mods=Join-Path $C.Game 'GAMEDATA/MODS';[SupermoonSetup.Native]::NoReparse($mods);if(-not[IO.Directory]::Exists($mods)){return}
    $own=Get-SMTarget $C 'bank';$count=0
    foreach($folder in [IO.Directory]::EnumerateDirectories($mods)){
        if(++$count -gt 4096){throw 'Too many MODS directories to inspect safely'}
        [SupermoonSetup.Native]::NoReparse($folder)
        $bank=Join-Path $folder 'audio/windows/init.bnk'
        if($bank.Equals($own,[StringComparison]::OrdinalIgnoreCase)){continue}
        $id=Get-SMCurrent $bank 65536
        if($null -ne $id -and -not(Test-SMEqual $id $C.Expected['bank'])){throw "Conflicting loose audio-bank resource; preserve and resolve it: $bank"}
    }
}
function Invoke-SMOperation($C,$Action,$Package=$null,$Record=$null) {
    if($Action -isnot [string]){throw 'Unknown setup action'}
    $Action=$Action.ToLowerInvariant()
    if($Action -ceq 'status'){return Get-SMStatus $C $Record}
    if($Action -ceq 'generate'){$bank=Get-SMAudioBank $C;return @{result='PASS_GENERATED_IN_MEMORY_NO_GAME_WRITES';identity=(Get-SMIdentity $bank)}}
    if($Action -cin @('install','update')){
        if(-not $Package -or $Record){throw 'Install/update requires a separately extracted package, without Record.'}
        $packagePath=[SupermoonSetup.Native]::PathName($Package);[SupermoonSetup.Native]::NoReparse($packagePath)
        if((Test-SMWithin $packagePath $C.Game) -or (Test-SMWithin $C.Game $packagePath)){throw 'Extract the package separately, outside the game tree.'}
        Assert-SMClosed;Assert-SMGame $C;Assert-SMNoLooseConflict $C
        $desired=@{proxy=(Read-SMFile (Join-Path $packagePath 'winmm.dll'));payload=(Read-SMFile (Join-Path $packagePath 'Supermoon/supermoon_playlist.dll'));marker=(Read-SMFile (Join-Path $packagePath 'bytebeat-playlist-experimental.enabled'))}
        foreach($k in @('proxy','payload','marker')){if(-not(Test-SMEqual (Get-SMIdentity $desired[$k]) $C.Expected[$k])){throw "Wrong/missing package $k"}}
        $desired.bank=Get-SMAudioBank $C;return Invoke-SMMutation $C $Action $desired
    }
    if($Action -cnotin @('resume','rollback','disable')){throw 'Unknown setup action'}
    if($Action -cne 'disable'){Assert-SMNoLooseConflict $C}
    return Invoke-SMMutation $C $Action $null $Record
}
Export-ModuleMember -Function New-SMContext,Read-SMJson,Get-SMStatus,Invoke-SMOperation
