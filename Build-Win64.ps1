# Generates project files and archives Development, DebugGame, and Shipping builds for Win64.
# Id is an optional string; project and archive paths are rooted at this script.
[CmdletBinding()]
param(
    [string]$Id = 'Dev'
)

$ErrorActionPreference = 'Stop'
$Project = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot 'test_UE_584_G1_c.uproject')).Path
$EngineVersion = (Get-Content -LiteralPath $Project -Raw | ConvertFrom-Json).EngineAssociation
$EngineRoot = (Get-ItemProperty -LiteralPath "HKLM:\SOFTWARE\EpicGames\Unreal Engine\$EngineVersion" -Name InstalledDirectory -ErrorAction SilentlyContinue).InstalledDirectory
if (-not $EngineRoot) {
    $EngineRoot = "D:\Program Files\Epic Games\UE_$EngineVersion"
}
$Configuration = 'Development+DebugGame+Shipping'
$Archive = Join-Path $PSScriptRoot "Builds\$Id"

& "$EngineRoot\Engine\Build\BatchFiles\Build.bat" -ProjectFiles "-Project=$Project" -Game
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

$UatOptions = @(
    'BuildCookRun', "-project=$Project", '-target=test_UE_584_G1_c'
    '-platform=Win64', "-clientconfig=$Configuration", '-build', '-cook'
    '-stage', '-package', '-pak', '-iostore', '-archive', "-archivedirectory=$Archive"
    '-map=/Game/ThirdPerson/Lvl_ThirdPerson', '-unattended', '-nop4'
)
& "$EngineRoot\Engine\Build\BatchFiles\RunUAT.bat" @UatOptions
exit $LASTEXITCODE
