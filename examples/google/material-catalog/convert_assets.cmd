@echo off
setlocal

where python >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo ERROR: Python 3 is required but not found in PATH.
    echo To install on Windows: winget install Python.Python.3.11
    echo Alternatively, download from https://www.python.org/downloads/
    exit /b 1
)

echo Running Python asset converter...
python convert_assets.py
if %ERRORLEVEL% neq 0 (
    echo ERROR: Asset conversion failed.
    exit /b 1
)
echo Done.
endlocal
