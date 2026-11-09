#requires -Version 5.1
# SPDX-License-Identifier: MIT
# Parse named arguments inside the error boundary: malformed options also receive
# a JSON response in noninteractive mode. No argument text is evaluated as code.
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
$WarningPreference='Stop'
$ProgressPreference='SilentlyContinue'
$Action='status';$Game=$null;$Package=$null;$Record=$null;$ConfirmAction=$null
$Interactive=$false;$exitCode=1
function Write-SMSetupMessage([string]$Text) {
    if($Interactive){Write-Host $Text}else{[Console]::Error.WriteLine($Text)}
}
function Write-SMSetupJson($Value) {
    # Bypass the native host's cached text writer: emit UTF-8 via Console.Out.
    [Console]::Out.WriteLine((ConvertTo-Json -InputObject $Value -Depth 24))
    [Console]::Out.Flush()
}
try {
    [Console]::OutputEncoding=[Text.UTF8Encoding]::new($false)
    if([Console]::IsInputRedirected){[Console]::InputEncoding=[Text.UTF8Encoding]::new($false)}
    $names=@{'-action'='Action';'-game'='Game';'-package'='Package';'-record'='Record';'-confirmaction'='ConfirmAction'}
    $seen=@{}
    for($i=0;$i -lt $args.Count;$i++){
        if($args[$i] -isnot [string]){throw 'Setup arguments must be strings.'}
        $key=$args[$i].ToLowerInvariant()
        if($seen.ContainsKey($key)){throw ('Duplicate setup option: '+$args[$i])};$seen[$key]=$true
        if($key -eq '-interactive'){$Interactive=$true;continue}
        if(-not $names.ContainsKey($key)){throw ('Unknown setup option: '+$args[$i])}
        if($i+1 -ge $args.Count -or $args[$i+1] -isnot [string] -or $args[$i+1].StartsWith('-')){throw ('Missing value for setup option: '+$args[$i])}
        $i++;Set-Variable -Name $names[$key] -Value $args[$i] -Scope Script
    }
    $Action=$Action.ToLowerInvariant()
    if($Action -notin @('status','generate','install','update','resume','rollback','disable')){throw 'Unknown setup action'}
    Import-Module (Join-Path $PSScriptRoot 'setup/Supermoon.Setup.psm1') -Force -ErrorAction Stop
    $root=[IO.Path]::GetDirectoryName($PSScriptRoot)
    $recipe=Read-SMJson (Join-Path $root 'build/recipe.json')
    $plan=Read-SMJson (Join-Path $PSScriptRoot 'audio_patch.json')
    if(-not $Package){$Package=[IO.Path]::GetDirectoryName($root)}
    function Find-SteamGames {
        $roots=New-Object 'Collections.Generic.HashSet[string]' ([StringComparer]::OrdinalIgnoreCase)
        foreach($entry in @(@('CurrentUser','Software\Valve\Steam','SteamPath'),@('LocalMachine','SOFTWARE\WOW6432Node\Valve\Steam','InstallPath'),@('LocalMachine','SOFTWARE\Valve\Steam','InstallPath'))){
            $base=$null;$key=$null
            try{$hive=[Microsoft.Win32.RegistryHive]::$($entry[0]);$base=[Microsoft.Win32.RegistryKey]::OpenBaseKey($hive,[Microsoft.Win32.RegistryView]::Registry64);$key=$base.OpenSubKey($entry[1],$false);if($key){$v=$key.GetValue($entry[2]);if($v -is [string] -and $v){[void]$roots.Add($v)}}}catch{}finally{if($key){$key.Dispose()};if($base){$base.Dispose()}}
        }
        if(${env:ProgramFiles(x86)}){
            try{$pf=[SupermoonSetup.Native]::LocalPath(${env:ProgramFiles(x86)});[SupermoonSetup.Native]::NoReparse($pf);[void]$roots.Add([IO.Path]::Combine($pf,'Steam'))}
            catch{Write-SMSetupMessage 'An unsupported Steam root was ignored; manual selection remains available.'}
        }
        $libraries=New-Object 'Collections.Generic.HashSet[string]' ([StringComparer]::OrdinalIgnoreCase)
        foreach($steam in @($roots)){
            try{
                $steam=[SupermoonSetup.Native]::LocalPath($steam);[SupermoonSetup.Native]::NoReparse($steam);[void]$libraries.Add($steam)
                $raw=[SupermoonSetup.Native]::Read((Join-Path $steam 'steamapps/libraryfolders.vdf'),1048576)
                if($null -ne $raw){
                    $text=[Text.UTF8Encoding]::new($false,$true).GetString($raw)
                    foreach($m in [regex]::Matches($text,'"path"\s*"((?:\\.|[^"\\])*)"')){
                        if($libraries.Count -ge 128){throw 'Steam library bound exceeded'}
                        try{$library=[SupermoonSetup.Native]::LocalPath($m.Groups[1].Value.Replace('\\','\'));[SupermoonSetup.Native]::NoReparse($library);[void]$libraries.Add($library)}
                        catch{Write-SMSetupMessage 'An unsupported Steam library was ignored; manual selection remains available.'}
                    }
                }
            }catch{Write-SMSetupMessage 'A Steam library could not be inspected; manual selection remains available.'}
        }
        foreach($library in $libraries){try{
            $library=[SupermoonSetup.Native]::LocalPath($library);[SupermoonSetup.Native]::NoReparse($library)
            $candidate=[IO.Path]::Combine($library,"steamapps/common/No Man's Sky");[SupermoonSetup.Native]::NoReparse($candidate)
            if([IO.Directory]::Exists($candidate)){$c=New-SMContext $candidate $recipe $plan;$s=Get-SMStatus $c;if($s.game_supported){$c.Game}}
        }catch{}}
    }
    if($Interactive){
        Write-SMSetupMessage 'Supermoon setup - Python-free Windows installation'
        Write-SMSetupMessage 'Close NMS. Keep Steam updates and other installation tools idle.'
        Write-SMSetupMessage 'Only the two DLLs, activation marker and locally generated bank are managed.'
        Write-SMSetupMessage 'Your music, settings, Cache, State and diagnostics are preserved.'
        if(-not $Game){
            $found=@(Find-SteamGames)
            if($found.Count -eq 1){$Game=$found[0];Write-SMSetupMessage ('Detected supported Steam installation: '+$Game)}
            else{Write-SMSetupMessage 'Automatic detection did not find one supported installation.'}
            $entered=Read-Host 'Enter the NMS root containing Binaries and GAMEDATA, or press Enter to use the detected path'
            if($entered){$Game=$entered;if($Game.StartsWith('"') -and $Game.EndsWith('"')){$Game=$Game.Substring(1,$Game.Length-2)}}
        }
    }
    if(-not $Game){throw 'An NMS root is required. Run Setup.cmd or specify -Game with an absolute directory.'}
    $context=New-SMContext $Game $recipe $plan
    if($Interactive){
        $state=Get-SMStatus $context
        Write-SMSetupMessage ('Game directory: '+$context.Game)
        Write-SMSetupMessage ('Supported executable: '+$state.game_supported+'; installation state: '+$state.result)
        if($state.ContainsKey('error')){Write-SMSetupMessage $state.error}
        if($state.ContainsKey('game_inspection_error')){Write-SMSetupMessage ('Executable inspection failed. Verified Disable remains available: '+$state.game_inspection_error)}
        foreach($key in @('proxy','payload','bank','marker')){
            $label=if($null -eq $state.observed[$key]){'absent'}elseif($state.observed[$key]['sha256'] -ceq $context.Expected[$key]['sha256'] -and $state.observed[$key]['bytes'] -eq $context.Expected[$key]['bytes']){'matches this release'}else{'existing file; ownership must be verified before any change'}
            Write-SMSetupMessage ($key+': '+$label)
        }
        Write-SMSetupMessage '1 Status   2 Install   3 Update   4 Resume interrupted setup'
        Write-SMSetupMessage '5 Roll back to recorded prior state   6 Disable   7 Exit'
        $choice=Read-Host 'Choose an action'
        if($choice -eq '7'){exit 0}
        $menu=@{'1'='status';'2'='install';'3'='update';'4'='resume';'5'='rollback';'6'='disable'}
        if(-not $menu.ContainsKey($choice)){throw 'No valid action selected; no installation changes made.'};$Action=$menu[$choice]
    }
    if($Action -notin @('status','generate')){
        Write-SMSetupMessage ('Requested action: '+$Action+'; target: '+$context.Game)
        Write-SMSetupMessage 'Backups/recovery records and the four program files may be written. The native archive is never replaced.'
        if($Action -eq 'rollback'){Write-SMSetupMessage 'Rollback restores the recorded prior state, which may be absence. Personal files are not rolled back.'}
        $expected=$Action.ToUpperInvariant()
        if($ConfirmAction -cne $expected){
            if($Interactive){$answer=Read-Host ('Type '+$expected+' to proceed, or anything else to cancel')}
            else{Write-SMSetupMessage ('Type '+$expected+' to proceed, or anything else to cancel');$answer=[Console]::ReadLine()}
            if($answer -cne $expected){throw 'Cancelled; no installation changes made.'}
        }
        Write-SMSetupMessage 'Checking identities, process state, conflicts and recovery data...'
    }
    $result=Invoke-SMOperation $context $Action $Package $Record
    if(-not $Interactive){Write-SMSetupJson $result}
    $exitCode=if($result['result'] -like '*REFUSED*'){1}else{0}
    if($Interactive -and $exitCode -eq 0){
        Write-SMSetupMessage ('Completed: '+$result['result'])
        if($result.ContainsKey('recovery')){Write-SMSetupMessage ('Keep this recovery folder: '+$result['recovery'])}
        if($Action -eq 'status'){Write-SMSetupMessage ('Recovery records: '+$result['recovery_root'])}
        if($Action -in @('install','update','resume')){
            Write-SMSetupMessage ('Add WAV/MP3 songs to '+(Join-Path $context.Game 'Binaries/Supermoon/Music')+' or keep your existing configured library.')
            Write-SMSetupMessage 'Launch NMS normally; enable mods and the Supermoon audio entry in its normal settings.'
            Write-SMSetupMessage 'First registration may need a normal exit/restart. Select Supermoon in the personal ByteBeat player and press Play.'
            Write-SMSetupMessage 'Keep the matching setup folder and recovery backups. Copy recovery backups outside the game before moving/removing it.'
        }
    }
}catch{
    $failure=@{result='REFUSED_OR_INCOMPLETE';error=$_.Exception.Message;guidance='Keep NMS closed. An interrupted operation may already have changed files. Keep all records; inspect Status using matching tools. Never overwrite unknown files or edit recovery hashes.'}
    if($Interactive){Write-SMSetupMessage ('Setup failed: '+$failure.error);Write-SMSetupMessage $failure.guidance}else{Write-SMSetupJson $failure}
    Write-SMSetupMessage 'Setup did not complete. No automatic elevation or security-setting change was attempted.'
    $exitCode=1
}
exit $exitCode
