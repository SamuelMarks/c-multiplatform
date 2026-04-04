param()

Write-Host "Building screenshot_generator via CMake..."
$buildDir = "build"
if (!(Test-Path "CMakeLists.txt")) {
    Write-Error "Must be run from the root of the material-catalog project."
    exit 1
}

# Determine if we are building standalone or within c-multiplatform
if (Test-Path "..\..\..\build") {
    $buildDir = "..\..\..\build"
} elseif (!(Test-Path "build")) {
    New-Item -ItemType Directory -Force -Path "build" | Out-Null
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
}

Set-Location $buildDir

# Build the executable
cmake --build . --config Debug --target screenshot_generator --parallel 4
if ($LASTEXITCODE -ne 0) {
    Write-Error "Failed to build screenshot_generator."
    exit 1
}

Write-Host "Running screenshot_generator to capture snapshots..."
$exePath = ""
$possiblePaths = @(
    ".\bin\Debug\screenshot_generator.exe",
    ".\bin\screenshot_generator.exe",
    ".\screenshot_generator.exe",
    ".\examples\google\material-catalog\screenshot_generator.exe",
    ".\Debug\screenshot_generator.exe"
)
foreach ($path in $possiblePaths) {
    if (Test-Path $path) {
        $exePath = $path
        break
    }
}

if (-not $exePath) {
    Write-Error "Failed to find screenshot_generator executable."
    exit 1
}

& $exePath
if ($LASTEXITCODE -ne 0) {
    Write-Error "Failed to execute screenshot_generator."
    exit 1
}

# Capture the bmp path since we are in buildDir
$bmpFiles = Get-ChildItem -Path . -Filter "screenshot_*.bmp" | Select-Object -ExpandProperty FullName

# Go back to the sample root
Set-Location $PSScriptRoot

Write-Host "Converting BMPs to PNGs and organizing them..."
foreach ($bmpFile in $bmpFiles) {
    .\update_readme.ps1 -BmpPath $bmpFile
}

Write-Host "Screenshots automatically generated and updated successfully!"
