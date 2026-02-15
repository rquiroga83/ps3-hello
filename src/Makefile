ifeq ($(strip $(PSL1GHT)),)
$(error "Please set PSL1GHT in your environment. export PSL1GHT=<path>")
endif

TARGET		:= hello_world
TITLE		:= Hola Mundo PS3
APPID		:= TEST00001

OFILES		:= main.o
CFLAGS		= -I$(PSL1GHT)/ppu/include -std=gnu99
LIBS		:= -lrsx -lgcm_sys -lio -lsysutil -lrt -llv2

include $(PSL1GHT)/ppu_rules

LD		:= $(CC)
BUILDDIR	:= build

all: $(TARGET).self

$(TARGET).elf: $(OFILES)

pkg: $(TARGET).pkg

clean:
	rm -f *.o *.d *.elf *.self *.fake.self *.pkg *.gnpdrm.pkg
	rm -rf build
