uspi_test -spi 0 -drdy 0 -nadc 3 -header 4 -time 1 -scbr 16 -loops 3 >log
uspi_check uspi0.dat >>log
uspi_check uspi1.dat >>log
uspi_check uspi2.dat >>log
del *.dat
