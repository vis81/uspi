target remote localhost:3333
#monitor endian little
monitor reset init
#load
tbreak main
continue
