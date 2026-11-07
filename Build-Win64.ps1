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

New-Item -ItemType Directory -Path $Archive -Force | Out-Null
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
$UatStartedAt = [DateTime]::UtcNow
& "$EngineRoot\Engine\Build\BatchFiles\RunUAT.bat" @UatOptions
$BuildExitCode = $LASTEXITCODE

$LogRoot = Join-Path $env:APPDATA 'Unreal Engine\AutomationTool\Logs'
if ($env:uebp_FinalLogFolder) {
    $LogRoot = $env:uebp_FinalLogFolder
}
elseif ($env:uebp_LogFolder) {
    $LogRoot = $env:uebp_LogFolder
}
$LatestLog = Get-ChildItem -LiteralPath $LogRoot -Filter 'Log.txt' -Recurse -File |
    Where-Object { $_.LastWriteTimeUtc -ge $UatStartedAt } |
    Sort-Object LastWriteTimeUtc -Descending |
    Where-Object { Select-String -LiteralPath $_.FullName -SimpleMatch "-project=$Project" -Quiet } |
    Select-Object -First 1
if ($LatestLog) {
    Copy-Item -LiteralPath $LatestLog.FullName -Destination (Join-Path $Archive 'Build.log') -Force
}
else {
    Write-Warning 'No AutomationTool Log.txt was found for this build.'
}
exit $BuildExitCode
