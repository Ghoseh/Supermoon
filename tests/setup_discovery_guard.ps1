# SPDX-License-Identifier: MIT
# Test-only provider guard, adapted from the audit reproduction. Blocks an unsafe
# Join-Path call rather than allowing any UNC/network traversal during a test.
param([string]$Setup,[string]$Trace,[string]$Forbidden)
$ErrorActionPreference='Stop'
$global:SMForbiddenRoots=@([IO.File]::ReadAllText($Forbidden)|ConvertFrom-Json)
$global:SMDiscoveryTrace=$Trace
function global:Join-Path {
    param([string]$Path,[string]$ChildPath)
    $denied=$Path.StartsWith('\\')
    foreach($root in $global:SMForbiddenRoots){if($Path.StartsWith($root,[StringComparison]::OrdinalIgnoreCase)){$denied=$true}}
    if($denied){[IO.File]::WriteAllText($global:SMDiscoveryTrace,$Path);throw 'TEST_BLOCKED_UNADMITTED_PROVIDER_PATH'}
    Microsoft.PowerShell.Management\Join-Path -Path $Path -ChildPath $ChildPath
}
& $Setup -Interactive
