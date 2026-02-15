ifeq ($(strip $(PSL1GHT)),)
$(error "Please set PSL1GHT in your environment. export PSL1GHT=<path>")
endif

include $(PSL1GHT)/spu_rules

TARGET		:= spu
SOURCES		:= source
INCLUDES	:= ../include

OFILES		:= source/main.o
CFLAGS		= -O2 -Wall -I$(CURDIR)/../include $(LIBPSL1GHT_INC)
LDFLAGS		:= $(LIBPSL1GHT_LIB)
LIBS		:= -lsputhread

LD		:= $(CC)

export OUTPUT	:= $(CURDIR)/$(TARGET)

all: $(TARGET).elf

$(TARGET).elf: $(OFILES)

install: all
	@mkdir -p ../data
	@cp -f $(TARGET).elf ../data/spu.bin
	@echo "SPU binary installed to data/spu.bin"

clean:
	@rm -f source/*.o source/*.d $(TARGET).elf
