#requires -Version 5.1
# SPDX-License-Identifier: MIT
# Python-free preparation/verification only. Never writes inside the selected game.
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop';$WarningPreference='Stop';$ProgressPreference='SilentlyContinue'
$Action='prepare';$Game=$null;$Output=$null;$Package=$null;$Interactive=$false;$exitCode=1
try {
    [Console]::OutputEncoding=[Text.UTF8Encoding]::new($false)
    $names=@{'-action'='Action';'-game'='Game';'-output'='Output';'-package'='Package'};$seen=@{}
    for($i=0;$i -lt $args.Count;$i++){
        $key=([string]$args[$i]).ToLowerInvariant()
        if($seen.ContainsKey($key)){throw 'Duplicate manual setup option.'};$seen[$key]=$true
        if($key -ceq '-interactive'){$Interactive=$true;continue}
        if(-not $names.ContainsKey($key)){throw 'Unknown manual setup option.'}
        if($i+1 -ge $args.Count -or ([string]$args[$i+1]).StartsWith('-')){throw 'Missing manual setup option value.'}
        $i++;Set-Variable -Name $names[$key] -Value ([string]$args[$i]) -Scope Script
    }
    $Action=$Action.ToLowerInvariant()
    if($Action -cnotin @('prepare','verify','removal-check')){throw 'Use prepare, verify or removal-check.'}
    if(-not $Package){$Package=[IO.Path]::GetDirectoryName([IO.Path]::GetDirectoryName($PSScriptRoot))}
    if($Interactive){
        Write-Host 'Automatic installation/upgrade remains available through Setup.cmd.'
        Write-Host 'Manual preparation is for fresh, inactive installations only. Close NMS.'
        Write-Host '1 Prepare files outside the game   2 Verify copied files   3 Check files before manual removal'
        switch(Read-Host 'Choose 1, 2 or 3'){'1'{$Action='prepare'}'2'{$Action='verify'}'3'{$Action='removal-check'}default{throw 'No action selected.'}}
        $Game=Read-Host 'Full NMS root containing Binaries and GAMEDATA'
        $Output=Read-Host 'Full prepared-folder path (new folder for Prepare; existing folder for Verify/removal check)'
        foreach($name in @('Game','Output')){
            $value=Get-Variable -Name $name -ValueOnly
            if($value.StartsWith('"') -and $value.EndsWith('"')){Set-Variable -Name $name -Value $value.Substring(1,$value.Length-2)}
        }
    }
    if(-not $Game -or -not $Output){throw 'Explicit -Game and -Output absolute paths are required.'}
    Import-Module (Join-Path $PSScriptRoot 'setup/Supermoon.Manual.psm1') -Force
    $result=Invoke-SMManualOperation -Game $Game -Package $Package -Output $Output -Action $Action
    [Console]::Out.WriteLine((ConvertTo-Json -InputObject $result -Depth 24))
    if($Interactive){
        Write-Host ('Result: '+$result.result)
        if($Action -ceq 'prepare'){Write-Host ('Read '+(Join-Path $Output 'READ_THIS_FIRST.txt')+' before copying anything.')}
        elseif($Action -ceq 'removal-check'){Write-Host 'No files removed. Follow MANUAL_INSTALL.md; retain personal data and all recovery records.'}
    }
    $exitCode=0
}catch{
    $failure=@{result='MANUAL_REFUSED';error=$_.Exception.Message;game_writes_attempted=$false;guidance='Keep NMS closed. Never overwrite unknown files. Do not discard recovery records. Incomplete preparation folders are not ready to copy.'}
    [Console]::Out.WriteLine((ConvertTo-Json -InputObject $failure -Depth 8))
    if($Interactive){Write-Host $failure.error}
}
exit $exitCode
