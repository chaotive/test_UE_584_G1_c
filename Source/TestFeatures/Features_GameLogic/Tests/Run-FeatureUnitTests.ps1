# Runs the standalone AngelScript suite with the runner from the pinned SDK.
# Uses paths relative to this script and returns the runner's exit code.
$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false

$Runtime = Join-Path $PSScriptRoot '..\..\..\ThirdParty\FeatureAngelScriptRuntime'
$Dependency = Get-Content -LiteralPath (Join-Path $Runtime 'Dependency.json') -Raw | ConvertFrom-Json
$Runner = Join-Path $Runtime "$($Dependency.Version)\tools\Win64\asrun.exe"
$Suite = Join-Path $PSScriptRoot 'FeatureGameLogicTests.as'

& $Runner $Suite
exit $LASTEXITCODE
