# Fetches the pinned SDK into the ignored cache or rebuilds the distributable static library.
# Rebuild and FetchOnly are switch values; all paths are rooted at this script's project.
[CmdletBinding()]
param(
    [switch]$Rebuild,
    [switch]$FetchOnly
)

$ErrorActionPreference = 'Stop'
$ProjectRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../../..'))
$ManifestPath = Join-Path $PSScriptRoot 'Dependency.json'
$Dependency = Get-Content -LiteralPath $ManifestPath -Raw | ConvertFrom-Json
$ManifestHash = (Get-FileHash -LiteralPath $ManifestPath -Algorithm SHA256).Hash
$VersionRoot = Join-Path $PSScriptRoot $Dependency.Version
$Library = Join-Path $VersionRoot 'lib/Win64/Release/angelscript.lib'
$Header = Join-Path $VersionRoot 'include/angelscript.h'
$BuildInfoPath = Join-Path $VersionRoot 'BuildInfo.json'

if (-not $Rebuild -and -not $FetchOnly -and (Test-Path -LiteralPath $BuildInfoPath)) {
    $Existing = Get-Content -LiteralPath $BuildInfoPath -Raw | ConvertFrom-Json
    if ((Test-Path -LiteralPath $Library) -and (Test-Path -LiteralPath $Header) -and
        $Existing.ManifestSha256 -eq $ManifestHash -and
        $Existing.LibrarySha256 -eq (Get-FileHash -LiteralPath $Library -Algorithm SHA256).Hash -and
        $Existing.HeaderSha256 -eq (Get-FileHash -LiteralPath $Header -Algorithm SHA256).Hash) {
        Write-Output "Using the recorded AngelScript $($Dependency.Version) Win64 /MD library; no rebuild needed."
        exit 0
    }
}

$CacheRoot = Join-Path $ProjectRoot ".dependencies/AngelScript/$($Dependency.Version)"
New-Item -ItemType Directory -Path $CacheRoot -Force | Out-Null
$Archive = Join-Path $CacheRoot ([IO.Path]::GetFileName(([Uri]$Dependency.SourceUrl).AbsolutePath))
if (-not (Test-Path -LiteralPath $Archive)) {
    Invoke-WebRequest -Uri $Dependency.SourceUrl -OutFile $Archive
}
if ((Get-FileHash -LiteralPath $Archive -Algorithm SHA256).Hash -ne $Dependency.SourceSha256) {
    throw 'The AngelScript SDK archive does not match Dependency.json.'
}
# Extracts only the library SDK; upstream sample scripts are not project game logic.
# ZipArchive is locally owned and disposed after extracting its verified entries and collecting source paths.
$Zip = [IO.Compression.ZipFile]::OpenRead($Archive)
try {
    foreach ($Entry in $Zip.Entries | Where-Object { $_.FullName.StartsWith('sdk/angelscript/') -and $_.Name }) {
        $EntryPath = Join-Path $CacheRoot $Entry.FullName
        New-Item -ItemType Directory -Path (Split-Path $EntryPath) -Force | Out-Null
        [IO.Compression.ZipFileExtensions]::ExtractToFile($Entry, $EntryPath, $true)
    }
    $Sources = @($Zip.Entries | Where-Object { $_.FullName -match '^sdk/angelscript/source/[^/]+\.cpp$' } |
        ForEach-Object { Join-Path $CacheRoot $_.FullName } | Sort-Object)
}
finally {
    $Zip.Dispose()
}
$SdkRoot = Join-Path $CacheRoot 'sdk/angelscript'
if ($FetchOnly) {
    Write-Output "Verified AngelScript $($Dependency.Version) source: $SdkRoot"
    exit 0
}

$VsWhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
$Compiler = @(& $VsWhere -products '*' -all -find "VC/Tools/MSVC/$($Dependency.MsvcVersion)/bin/Hostx64/x64/cl.exe")[0]
if (-not $Compiler) {
    throw "Install MSVC $($Dependency.MsvcVersion), or explicitly update the pinned toolchain in Dependency.json."
}
$CompilerDirectory = Split-Path $Compiler
$MsvcRoot = [IO.Path]::GetFullPath((Join-Path $CompilerDirectory '../../..'))
$SdkDirectory = Join-Path ${env:ProgramFiles(x86)} 'Windows Kits/10'
$SdkVersion = $Dependency.WindowsSdkVersion
$env:INCLUDE = "$MsvcRoot/include;$SdkDirectory/Include/$SdkVersion/ucrt;$SdkDirectory/Include/$SdkVersion/shared;$SdkDirectory/Include/$SdkVersion/um"
$env:LIB = "$MsvcRoot/lib/x64;$SdkDirectory/Lib/$SdkVersion/ucrt/x64;$SdkDirectory/Lib/$SdkVersion/um/x64"

$ObjectRoot = Join-Path $CacheRoot 'build/Win64/Release'
New-Item -ItemType Directory -Path $ObjectRoot -Force | Out-Null
$CompileOptions = @($Dependency.CompileOptions) + @("/Fo$ObjectRoot\")
& $Compiler @CompileOptions @Sources
if ($LASTEXITCODE -ne 0) { throw 'AngelScript compilation failed.' }
$Objects = @($Sources | ForEach-Object { Join-Path $ObjectRoot ([IO.Path]::GetFileNameWithoutExtension($_) + '.obj') })
$BuiltLibrary = Join-Path $ObjectRoot 'angelscript.lib'
& (Join-Path $CompilerDirectory 'lib.exe') /nologo /Brepro "/OUT:$BuiltLibrary" @Objects
if ($LASTEXITCODE -ne 0) { throw 'AngelScript static-library creation failed.' }

New-Item -ItemType Directory -Path (Split-Path $Library), (Split-Path $Header) -Force | Out-Null
Copy-Item -LiteralPath $BuiltLibrary -Destination $Library -Force
Copy-Item -LiteralPath (Join-Path $SdkRoot 'include/angelscript.h') -Destination $Header -Force
$HeaderText = [IO.File]::ReadAllText($Header)
[IO.File]::WriteAllText((Join-Path $VersionRoot 'LICENSE.txt'), $HeaderText.Substring(0, $HeaderText.IndexOf('*/') + 2) + "`n")
$BuildInfo = [ordered]@{
    Version = $Dependency.Version
    Platform = 'Win64 x64'
    Runtime = '/MD'
    Configuration = 'Release'
    MsvcVersion = $Dependency.MsvcVersion
    WindowsSdkVersion = $Dependency.WindowsSdkVersion
    SourceSha256 = $Dependency.SourceSha256
    ManifestSha256 = $ManifestHash
    LibrarySha256 = (Get-FileHash -LiteralPath $Library -Algorithm SHA256).Hash
    HeaderSha256 = (Get-FileHash -LiteralPath $Header -Algorithm SHA256).Hash
}
[IO.File]::WriteAllText($BuildInfoPath, ($BuildInfo | ConvertTo-Json) + "`n", [Text.UTF8Encoding]::new($false))
Write-Output "Built AngelScript $($Dependency.Version): $Library"
Write-Output "The verified SDK source remains in the ignored cache: $SdkRoot"
