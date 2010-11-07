%CC% -c -O2 -s -I../../../libusb-win32-bin-1.2.1.0/include ../../../app/libuspi/uspi.c -o uspi.o

copy ..\..\..\libusb-win32-bin-1.2.1.0\lib\gcc\libusb.a  ..\..\..\out\lib\libuspi.a
%AR% -r ../../../out/lib/libuspi.a uspi.o 

copy ..\..\..\app\libuspi\uspi.h  ..\..\..\out\inc