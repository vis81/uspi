rem write 5 to i2c slave 0x50/internal addr 0x0 on 100kHz speed
uspi_i2c.exe -s 100000 -a 50 -i 0 -w 5 