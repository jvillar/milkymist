# Mico32 toolchain
#
AS=lm32-elf-as
CC=lm32-elf-gcc
LD=lm32-elf-ld
OBJCOPY=lm32-elf-objcopy
OBJDUMP=lm32-elf-objdump
AR=lm32-elf-ar
RANLIB=lm32-elf-ranlib


# Toolchain options
#
INCLUDES=-I$(MMDIR)/software/include -I$(MMDIR)/tools
ASFLAGS=$(INCLUDES)
CFLAGS=-O9 -Wall -fomit-frame-pointer -mbarrel-shift-enabled -mmultiply-enabled -mdivide-enabled -msign-extend-enabled -fno-builtin -fsigned-char -fsingle-precision-constant $(INCLUDES)
# activate to debug network controller #CFLAGS=-O9 -Wall -Wcast-align -fomit-frame-pointer -mbarrel-shift-enabled -mmultiply-enabled -mdivide-enabled -msign-extend-enabled -fno-builtin -fsigned-char -fsingle-precision-constant $(INCLUDES)
LDFLAGS=-nostdlib -nodefaultlibs
