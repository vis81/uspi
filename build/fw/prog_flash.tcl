################################################################################
#    SAM-BA history file
################################################################################

################################################################################
global target
################################################################################

# Unlock sectors if locked
set FLASH::ForceUnlockBeforeWrite 1

# Do not lock used sectors after write operation
set FLASH::ForceLockAfterWrite    0



send_file {Flash} "flash.bin" 0x100000 0
compare_file  {Flash} "flash.bin" 0x100000 0
go 0x100000
