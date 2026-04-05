param()
$ErrorActionPreference = "Stop"

if (-not (Get-Command "python" -ErrorAction SilentlyContinue)) {
    Write-Error "ERROR: Python 3 is required but not found in PATH."
    Write-Host "To install on Windows: winget install Python.Python.3.11"
    Write-Host "Alternatively, download from https://www.python.org/downloads/"
    exit 1
}

Write-Host "Running Python asset converter..."
python .\convert_assets.py
if ($LASTEXITCODE -ne 0) {
    Write-Error "Asset conversion failed."
    exit 1
}
Write-Host "Done."
