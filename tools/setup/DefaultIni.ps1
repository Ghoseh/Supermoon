# SPDX-License-Identifier: MIT
# Additive, idempotent completion of user INI. Never changes an existing value.
$script:IniDefaults=@(
 @{Name='Playlist';Values=[ordered]@{Music='Music';CacheMiB='3072'}},
 @{Name='Normalization';Values=[ordered]@{Enabled='1';TargetLUFS='-16';TruePeakDBTP='-1'}},
 @{Name='Audio';Values=[ordered]@{VoiceGainDb='3.0'}},
 @{Name='Keyboard';Values=[ordered]@{Enabled='1';Previous='Ctrl+Shift+F6';Next='Ctrl+Shift+F7';ToggleShuffle='Ctrl+Shift+F8';ShuffleAndAdvance='Ctrl+Shift+F9'}},
 @{Name='Notifications';Values=[ordered]@{ShuffleSounds='1'}}
)
function Add-SMIniInsertion($Map,[int]$Offset,[string]$Text){
    if($Map.ContainsKey($Offset)){$Map[$Offset]+=$Text}else{$Map.Add($Offset,$Text)}
}
function Get-SMIniPlan($C,[byte[]]$Template){
    $path=Join-Path $C.Game 'Binaries/Supermoon/playlist.ini';$raw=Read-SMFile $path 65536
    if($null -eq $raw){return @{path=$path;before=$null;original=$null;after=$Template;added=12}}
    $prefix=0;$encoding=[Text.Encoding]::GetEncoding(28591)
    if($raw.Length -ge 2 -and $raw[0] -eq 255 -and $raw[1] -eq 254){$prefix=2;$encoding=[Text.UnicodeEncoding]::new($false,$false,$true)}
    elseif($raw.Length -ge 2 -and $raw[0] -eq 254 -and $raw[1] -eq 255){throw 'UTF-16BE INI is unsupported; original retained. Use a supported encoding deliberately.'}
    elseif($raw.Length -ge 3 -and $raw[0] -eq 239 -and $raw[1] -eq 187 -and $raw[2] -eq 191){throw 'UTF-8 BOM INI needs manual encoding review; original retained. Automatic completion supports no-BOM text or UTF-16LE BOM.'}
    elseif($raw -contains 0){throw 'Ambiguous/binary INI encoding; original retained.'}
    $text=$encoding.GetString($raw,$prefix,$raw.Length-$prefix)
    $newline=if($text.Contains("`r`n")){"`r`n"}elseif($text.Contains("`n")){"`n"}else{"`r`n"}
    $sections=@{};$current=$null
    foreach($line in [regex]::Matches($text,'[^\r\n]+(?:\r\n|\r|\n|$)|(?:\r\n|\r|\n)')){
        $body=$line.Value.TrimEnd([char[]]"`r`n")
        if($body -match '^\s*\['){
            if($current){$current.end=$line.Index}
            $current=$null
            if($body -notmatch '^[ \t]*\[([^\[\]]+)\][ \t]*(?:;.*)?$'){throw 'Malformed INI section; original retained for review.'}
            $name=$Matches[1];$known=@($script:IniDefaults|Where-Object {$_.Name -ieq $name})
            if($known.Count){
                if($sections.ContainsKey($name)){throw 'Duplicate supported INI sections need review; original retained.'}
                $current=@{end=$text.Length;present=(New-Object 'Collections.Generic.HashSet[string]' ([StringComparer]::OrdinalIgnoreCase))};$sections[$name]=$current
            }
        }elseif($current -and $body -match '^[ \t]*([^=;\r\n]+?)[ \t]*='){
            [void]$current.present.Add($Matches[1].Trim([char[]]" `t"))
        }
    }
    $insertions=New-Object 'Collections.Generic.SortedDictionary[int,string]';$added=0;$absent=''
    foreach($section in $script:IniDefaults){
        if($sections.ContainsKey($section.Name)){
            $entry=$sections[$section.Name];$block=''
            foreach($key in $section.Values.Keys){if(-not $entry.present.Contains($key)){$block+=$key+'='+$section.Values[$key]+$newline;$added++}}
            if($block){if($entry.end -gt 0 -and $text[$entry.end-1] -ne "`n" -and $text[$entry.end-1] -ne "`r"){$block=$newline+$block};Add-SMIniInsertion $insertions $entry.end $block}
        }else{
            $absent+=$newline+'['+$section.Name+']'+$newline
            foreach($key in $section.Values.Keys){$absent+=$key+'='+$section.Values[$key]+$newline;$added++}
        }
    }
    if($absent){Add-SMIniInsertion $insertions $text.Length $absent}
    if(-not $added){return @{path=$path;before=(Get-SMIdentity $raw);original=$raw;after=$raw;added=0}}
    $updated=$text
    foreach($offset in @($insertions.Keys|Sort-Object -Descending)){$updated=$updated.Insert($offset,$insertions[$offset])}
    $encoded=$encoding.GetBytes($updated);$after=New-Object byte[] ($prefix+$encoded.Length)
    if($prefix){[Array]::Copy($raw,0,$after,0,$prefix)};[Array]::Copy($encoded,0,$after,$prefix,$encoded.Length)
    if($after.Length -gt 65536){throw 'Completed INI exceeds size limit; original retained.'}
    return @{path=$path;before=(Get-SMIdentity $raw);original=$raw;after=$after;added=$added}
}
function Get-SMConfigTemplate($C,$Package=$null){
    if(-not $C.Recipe.ContainsKey('configuration_defaults')){return $null}
    $pin=$C.Recipe['configuration_defaults'];Assert-SMKeys $pin @('bytes','sha256')
    if($pin['bytes'] -le 0 -or $pin['bytes'] -gt 65536 -or $pin['sha256'] -cnotmatch '^[0-9a-f]{64}$'){throw 'Invalid default INI identity.'}
    $raw=$null
    if($Package){
        $root=[SupermoonSetup.Native]::PathName($Package);[SupermoonSetup.Native]::NoReparse($root)
        if((Test-SMWithin $root $C.Game) -or (Test-SMWithin $C.Game $root)){throw 'Extract the package outside the game tree.'}
        $raw=Read-SMFile (Join-Path $root 'Supermoon/playlist.ini') 65536
        if($null -ne $raw -and -not(Test-SMEqual (Get-SMIdentity $raw) $pin)){throw 'Packaged default INI differs from its release identity.'}
    }
    if($null -eq $raw){$raw=Read-SMFile (Join-Path $C.Store 'default-playlist-1.2.1.ini') 65536}
    if(-not(Test-SMEqual (Get-SMIdentity $raw) $pin)){throw 'Verified complete default INI is missing; retain/use the matching package.'}
    [void](Get-SMIniPlan $C $raw) # Validate existing configuration before any activation transition.
    return ,$raw
}
function Invoke-SMConfigDefaults($C){
    if(-not $C.ContainsKey('ConfigTemplate') -or $null -eq $C.ConfigTemplate){return $null}
    Assert-SMClosed;$template=$C.ConfigTemplate
    if(-not(Test-SMEqual (Get-SMIdentity $template) $C.Recipe['configuration_defaults'])){throw 'Default INI changed before commit.'}
    $plan=Get-SMIniPlan $C $template
    [SupermoonSetup.Native]::NoReparse($C.Store)
    $saved=Join-Path $C.Store 'default-playlist-1.2.1.ini'
    $id=Get-SMCurrent $saved 65536
    if($null -eq $id){Write-SMNew $saved $template}elseif(-not(Test-SMEqual $id $C.Recipe['configuration_defaults'])){throw 'Saved defaults differ; preserve for review.'}
    if(-not $plan.added){return @{result='unchanged';added=0;original_backup=$null}}
    $backup=$null;$folder=Join-Path $C.Store 'configuration';[SupermoonSetup.Native]::NoReparse($folder);[void][IO.Directory]::CreateDirectory($folder)
    if($null -ne $plan.original){
        $backup=Join-Path $folder ($plan.before['sha256']+'.ini');$id=Get-SMCurrent $backup 65536
        if($null -eq $id){Write-SMNew $backup $plan.original}elseif(-not(Test-SMEqual $id $plan.before)){throw 'Original INI backup differs; no overwrite allowed.'}
    }
    [Console]::Error.WriteLine(('Completing playlist.ini: '+$plan.added+' missing defaults; existing values/comments preserved. Original backup: '+$backup))
    Invoke-SMCheckpoint 'config:backed_up'
    $parent=[IO.Path]::GetDirectoryName($plan.path);[SupermoonSetup.Native]::NoReparse($parent);[void][IO.Directory]::CreateDirectory($parent)
    $tmp=Join-Path $parent ('.playlist-defaults-'+[guid]::NewGuid().ToString('N')+'.tmp');$after=Get-SMIdentity $plan.after
    try{
        Write-SMNew $tmp $plan.after;Invoke-SMCheckpoint 'config:prepared';Assert-SMClosed
        if(-not(Test-SMEqual (Get-SMCurrent $plan.path 65536) $plan.before)){throw 'INI changed concurrently; no overwrite allowed.'}
        Invoke-SMCheckpoint 'config:before_replace';[SupermoonSetup.Native]::Replace($tmp,$plan.path);Invoke-SMCheckpoint 'config:after_replace'
        if(-not(Test-SMEqual (Get-SMCurrent $plan.path 65536) $after)){throw 'INI completion readback failed; retain backup.'}
    }finally{
        if([IO.File]::Exists($tmp)){
            [SupermoonSetup.Native]::NoReparse($tmp)
            if(Test-SMEqual (Get-SMCurrent $tmp 65536) $after){[IO.File]::Delete($tmp)}
        }
    }
    Write-SMJson (Join-Path $folder ('completed-'+$after['sha256']+'.json')) @{schema=1;before=$plan.before;after=$after;added=$plan.added;backup=$backup}
    return @{result=$(if($null -eq $plan.before){'created'}else{'completed'});added=$plan.added;original_backup=$backup}
}
