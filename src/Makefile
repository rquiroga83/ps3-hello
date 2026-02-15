ifeq ($(strip $(PSL1GHT)),)
$(error "Please set PSL1GHT in your environment. export PSL1GHT=<path>")
endif

TARGET		:= hello_world
TITLE		:= Hola Mundo PS3
APPID		:= TEST00001

OFILES		:= spu_bin.o main.o
CFLAGS		= -I$(PSL1GHT)/ppu/include -I$(CURDIR)/../include -std=gnu99
LIBS		:= -lrsx -lgcm_sys -lio -lsysutil -lrt -llv2 -lm

include $(PSL1GHT)/ppu_rules

LD		:= $(CC)
BUILDDIR	:= build

export VPATH	:= $(CURDIR)/../data

.PHONY: all spu clean

all: spu $(TARGET).self

spu:
	@$(MAKE) -C ../spu
	@$(MAKE) -C ../spu install

$(TARGET).elf: $(OFILES)

spu_bin.o: ../data/spu.bin
	@echo $(notdir $<)
	@$(bin2o) $< spu_bin $@

pkg: $(TARGET).pkg

clean:
	rm -f *.o *.d *.elf *.self *.fake.self *.pkg *.gnpdrm.pkg
	rm -rf build
	@$(MAKE) -C ../spu clean
	rm -f ../data/spu.bin
