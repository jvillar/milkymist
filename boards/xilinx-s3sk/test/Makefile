TOPDIR?=$(shell pwd)
BOARD_DIR=$(TOPDIR)/../rtl
CORES_DIR=$(TOPDIR)/../../../cores

include ../sources.mak

SIM_SRC=$(TOPDIR)/system_tb.v

SRC=$(SIM_SRC) $(BOARD_SRC) $(CORES_SRC)


all: isim.loadview

clean:
	rm -f bios.rom verilog.log system isim.prj fuse.log isim.log isim.compile isim.exe *~ *.wdb 
	rm -rf isim

.PHONY: clean cversim isim 

../../../software/bios/bios.bin:
	cd ../../../ && export BOARD="xilinx-s3sk" && ./build_bios.sh

bios.rom: ../../../software/bios/bios.bin
	../../../tools/bin2hex ../../../software/bios/bios.bin bios.rom 32768 8

# ISE Simulator
isim.prj: $(SRC) bios.rom
	rm -f isim.prj
	for i in `echo $(SRC)`; do \
	    echo "verilog work $$i" >> isim.prj; \
	done

isim.compile: isim.prj
	touch isim.compile
	vlogcomp -prj isim.prj -i ../rtl/ -d SIMULATION

isim.exe: isim.compile
	fuse -top system_tb -o isim.exe


isim.view: isim.exe
	./isim.exe -gui

isim.loadview: isim.exe isim.wcfg
	./isim.exe -gui -view isim.wcfg

# CVER
cversim: $(SRC) bios.rom
	cver +define+SIMULATION +incdir+$(BOARD_DIR) +incdir+$(CORES_DIR)/lm32/rtl $(SRC)


# Icarus verilog
icarus: system bios.rom
	./system

icarus.exe: $(SRC)
	iverilog -D SIMULATION -I $(BOARD_DIR) -I $(CORES_DIR)/lm32/rtl -o system $(SRC)


