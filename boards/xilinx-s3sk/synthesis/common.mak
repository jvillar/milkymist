prom: build/system.mcs

timing: build/system-routed.twr

usage: build/system-routed.xdl
	../../../tools/xdlanalyze.pl build/system-routed.xdl 0

load: build/system.bit
	cd build && impact -batch ../load.cmd

flash: build/system.mcs
	cd build && impact -batch ../flash.cmd

flashmem: 
	- killall gtkterm
	../../../build_bios.sh
	cd ../../../tools/norflasher && impact -batch impact.batch
	../../../tools/norflasher/norflasher.py /dev/ttyUSB0 --erase --program ../../../software/bios/bios.mcs

implement: flashmem load
	gtkterm -p /dev/ttyUSB0 -s 115200 &

build/system.ncd: build/system.ngd
	cd build && map system.ngd

build/system-routed.ncd: build/system.ncd
	cd build && par -ol high -xe n -w system.ncd system-routed.ncd

build/system.bit: build/system-routed.ncd
	cd build && bitgen -w system-routed.ncd system.bit

build/system.mcs: build/system.bit
	cd build && promgen -w -u 0 system

build/system-routed.xdl: build/system-routed.ncd
	cd build && xdl -ncd2xdl system-routed.ncd system-routed.xdl

build/system-routed.twr: build/system-routed.ncd
	cd build && trce -v 10 system-routed.ncd system.pcf

clean:
	rm -rf build/*

.PHONY: prom timing usage flashmem load implement clean 
