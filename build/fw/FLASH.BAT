echo off

SET TARGET=flash

SET RVCT40LIB=%ARMDIR%\RV31\LIB
SET ARMCC="%ARMDIR%\BIN40\ArmCC"
SET ARMASM="%ARMDIR%\BIN40\ArmAsm"
SET ARMLINK="%ARMDIR%\BIN40\ArmLink"
SET ARMFROMELF="%ARMDIR%\bin40\fromelf.exe"

SET CFLAGS= -c --cpu ARM7TDMI -O3 -Otime --apcs=interwork -I %ARMDIR%\RV31\INC -I "%ARMDIR%\INC\Atmel\SAM7S" -DFLASH -D__MICROLIB
set ASMFLAGS= --cpu ARM7TDMI --apcs=interwork --pd "__MICROLIB SETA 1" -I "%ARMDIR%\INC" -I "%ARMDIR%\INC\Atmel\SAM7S" --xref
set LDFLAGS= --cpu ARM7TDMI --library_type=microlib  --strict --autoat --summary_stderr --info summarysizes --map --symbols --info sizes --info totals 
set LDOBJ= ".\%TARGET%\adc.o" ".\%TARGET%\chanmgr.o" ".\%TARGET%\cmd.o" ".\%TARGET%\fiq.o" ".\%TARGET%\main.o" ".\%TARGET%\pio.o" ".\%TARGET%\retarget.o" ".\%TARGET%\serial.o" ".\%TARGET%\spi.o" ".\%TARGET%\time.o" ".\%TARGET%\usb.o" ".\%TARGET%\sam7.o" ".\%TARGET%\swi.o"

mkdir %TARGET%
mkdir ..\..\out\fw

echo on

%ARMCC% %CFLAGS% -o ".\%TARGET%\adc.o" "..\..\fw\adc.c"
%ARMCC% %CFLAGS% -o ".\%TARGET%\chanmgr.o" "..\..\fw\chanmgr.c"
%ARMCC% %CFLAGS% -o ".\%TARGET%\cmd.o" "..\..\fw\cmd.c"
%ARMCC% %CFLAGS% -o ".\%TARGET%\fiq.o" "..\..\fw\fiq.c"
%ARMCC% %CFLAGS% -o ".\%TARGET%\main.o" "..\..\fw\main.c"
%ARMCC% %CFLAGS% -o ".\%TARGET%\pio.o" "..\..\fw\pio.c"
%ARMCC% %CFLAGS% -o ".\%TARGET%\retarget.o" "..\..\fw\retarget.c"
%ARMCC% %CFLAGS% -o ".\%TARGET%\serial.o" "..\..\fw\serial.c"
%ARMCC% %CFLAGS% -o ".\%TARGET%\spi.o" "..\..\fw\spi.c"
%ARMCC% %CFLAGS% -o ".\%TARGET%\time.o" "..\..\fw\time.c"
%ARMCC% %CFLAGS% -o ".\%TARGET%\usb.o" "..\..\fw\usb.c"

%ARMASM% %ASMFLAGS% -o ".\%TARGET%\SAM7.o" "..\..\fw\SAM7.s"
%ARMASM% %ASMFLAGS% -o ".\%TARGET%\swi.o" "..\..\fw\swi.s"

%ARMLINK% %LDFLAGS% %LDOBJ% --scatter ".\%TARGET%.sct" --list ".\%TARGET%\%TARGET%.map" -o ".\%TARGET%\%TARGET%.axf"

%ARMFROMELF% --bin --output ..\..\out\fw\%TARGET%.bin .\%TARGET%\%TARGET%.axf
copy prog_flash.bat ..\..\out\fw
copy prog_flash.tcl ..\..\out\fw