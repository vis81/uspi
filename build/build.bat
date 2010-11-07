set drv=0
set app=0
set fw=0

if "%1"=="clean" (  
	rmdir /S /Q ..\out
	exit
)
if "%1"=="fw" (  
	set fw=1
) else if "%1"=="app" (
	set app=1
) else if "%1"=="drv" (
	set drv=1
) else (
	set drv=1
	set app=1
	set fw=1
)

SET ARMDIR=C:\Keil\ARM
SET MINGWDIR=C:\Program Files\CodeBlocks\MinGW
mkdir ..\out

if NOT %fw%==1 goto l_app
cd fw
call flash.bat
cd ..

:l_app
if NOT %app%==1 goto l_drv
cd app
call make.bat
cd ..

:l_drv
if NOT %drv%==1 goto l_exit
mkdir ..\out\drv
xcopy /S /Y drv ..\out\drv

:l_exit
exit