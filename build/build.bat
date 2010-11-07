SET ARMDIR=C:\Keil\ARM
SET MINGWDIR=C:\Program Files\CodeBlocks\MinGW

mkdir ..\out
cd fw
call flash.bat
cd ..\app
call make.bat

cd ..
mkdir ..\out\drv
xcopy /S /Y drv ..\out\drv