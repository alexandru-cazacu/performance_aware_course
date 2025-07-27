@echo off

if not exist build mkdir build
pushd build
cl ..\sim8086.c /nologo /Zo /Zi
popd