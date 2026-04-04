param(
    [string]$BmpPath
)

if (-not $BmpPath) {
    Write-Error "Must provide BmpPath"
    exit 1
}

$fileName = [System.IO.Path]::GetFileNameWithoutExtension($BmpPath)
$PngPath = "readme\$fileName.png"

Add-Type -AssemblyName System.Drawing
$bmp = New-Object System.Drawing.Bitmap($BmpPath)
$dir = [System.IO.Path]::GetDirectoryName($PngPath)
$fullDir = Join-Path (Get-Location) $dir
if (!(Test-Path $fullDir)) {
    New-Item -ItemType Directory -Force -Path $fullDir | Out-Null
}
$bmp.Save((Join-Path (Get-Location) $PngPath), [System.Drawing.Imaging.ImageFormat]::Png)

Write-Host "Generated $PngPath"
