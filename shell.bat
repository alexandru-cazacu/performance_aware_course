@echo off

for /f "delims=" %%i in ('"C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property InstallationPath') do set VSPATH=%%i

echo Found location is %VSPATH%

call "%VSPATH%\VC\Auxiliary\Build\vcvarsall.bat" x64
