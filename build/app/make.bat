echo off

SET PATH=%PATH%;%MINGWDIR%\bin
SET CC=mingw32-gcc.exe
SET AR=ar.exe

mkdir ..\..\out
mkdir ..\..\out\lib
mkdir ..\..\out\inc
mkdir ..\..\out\bin

echo on

cd libuspi
call make.bat
cd ..\uspi_test
call make.bat
cd ..\uspi_check
call make.bat
cd ..\uspi_conv
call make.bat

cd ..
copy test.bat ..\..\out\bin
copy ..\..\app\LabView\uspi.vi ..\..\out\bin