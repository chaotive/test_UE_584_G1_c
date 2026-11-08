# Runs the project's focused Unreal automation checks through Gauntlet's editor runner.
# The script accepts no arguments; it discovers the engine and returns AutomationTool's exit code.
$ErrorActionPreference = 'Stop'
$Project = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot 'test_UE_584_G1_c.uproject')).Path
$EngineVersion = (Get-Content -LiteralPath $Project -Raw | ConvertFrom-Json).EngineAssociation
$EngineRoot = (Get-ItemProperty -LiteralPath "HKLM:\SOFTWARE\EpicGames\Unreal Engine\$EngineVersion" -Name InstalledDirectory -ErrorAction SilentlyContinue).InstalledDirectory
if (-not $EngineRoot) {
    $EngineRoot = "D:\Program Files\Epic Games\UE_$EngineVersion"
}
$TestRoot = Join-Path $PSScriptRoot 'Intermediate\FeatureSmoke'
$ReportPath = Join-Path $TestRoot 'Report'
$PreviousLogFolder = $env:uebp_LogFolder
$PreviousFinalLogFolder = $env:uebp_FinalLogFolder
try {
    $env:uebp_LogFolder = Join-Path $TestRoot 'AutomationLogs'
    $env:uebp_FinalLogFolder = $env:uebp_LogFolder
    New-Item -ItemType Directory -Path $env:uebp_LogFolder -Force | Out-Null

    & "$EngineRoot\Engine\Build\BatchFiles\RunUAT.bat" RunUnreal "-project=$Project" '-build=Editor' '-platform=Win64' '-configuration=Development' '-test=UE.EditorAutomation' '-RunTest=Feature.Smoke' "-TempDir=$TestRoot" "-ReportExportPath=$ReportPath" '-NullRHI' '-unattended' '-nop4'
    $TestExitCode = $LASTEXITCODE
}
finally {
    $env:uebp_LogFolder = $PreviousLogFolder
    $env:uebp_FinalLogFolder = $PreviousFinalLogFolder
}
exit $TestExitCode
