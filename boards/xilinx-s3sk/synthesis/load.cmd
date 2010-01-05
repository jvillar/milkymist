setMode -bscan
cleancablelock
setCable -p usb21
identify
assignfile -p 1 -file system.bit
program -p 1
quit
