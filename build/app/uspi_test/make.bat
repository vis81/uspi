rem %CC% -mthreads -O2 -s  -I../../../out/inc -L../../../out/lib -L../../../libusb-win32-bin-1.2.1.0/lib/gcc ../../../app/uspi_test/main.c -luspi -lusb -o ../../../out/bin/uspi_test.exe
%CC% -mthreads -O2 -s  -I../../../out/inc -L../../../out/lib ../../../app/uspi_test/main.c -luspi -o ../../../out/bin/uspi_test.exe
