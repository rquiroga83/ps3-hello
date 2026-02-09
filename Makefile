#---------------------------------------------------------------------------------
# PPU Makefile - Hola PS3 con ejemplo SPE
#---------------------------------------------------------------------------------

#---------------------------------------------------------------------------------
# Clear the implicit built in rules
#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(PSL1GHT)),)
$(error "Please set PSL1GHT in your environment. export PSL1GHT=<path>")
endif

include $(PSL1GHT)/ppu_rules

#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing extra header files
#---------------------------------------------------------------------------------
TARGET		:= hello-ps3
BUILD		:= build
SOURCES		:= source
DATA		:= data
INCLUDES	:= include

TITLE		:= Hola Mundo PS3 - SPE Demo
APPID		:= PSL145310
CONTENTID	:= UP0001-PSL145310_00-0000000000000001

CFLAGS		+= -O2 -Wall $(MACHDEP) $(INCLUDE)

LD			:= $(CC)
LDFLAGS		:= -Wl,-N

#---------------------------------------------------------------------------------
# any extra libraries we wish to link with the project
#---------------------------------------------------------------------------------
LIBS		:= -lrt -llv2 -lm

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS		:=

#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export OUTPUT	:= $(CURDIR)/$(TARGET)

export VPATH	:= $(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
					$(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR	:= $(CURDIR)/$(BUILD)

export BUILDDIR	:= $(CURDIR)/$(BUILD)

#---------------------------------------------------------------------------------
# automatically build a list of object files for our project
#---------------------------------------------------------------------------------
CFILES		:= $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
SFILES		:= $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.S)))
# El archivo spu.bin se genera durante el build por el target spu, así que lo agregamos explícitamente
BINFILES	:= spu.bin $(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.bin)))
# Remover duplicados en caso de que spu.bin ya exista
BINFILES	:= $(sort $(BINFILES))

export OFILES	:= $(addsuffix .o,$(BINFILES)) \
					$(CFILES:.c=.o) \
					$(SFILES:.S=.o)

#---------------------------------------------------------------------------------
# build a list of include paths
#---------------------------------------------------------------------------------
export INCLUDE	:= $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
					$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
					$(LIBPSL1GHT_INC) \
					-I$(CURDIR)/$(BUILD)

#---------------------------------------------------------------------------------
# build a list of library paths
#---------------------------------------------------------------------------------
export LIBPATHS	:= $(foreach dir,$(LIBDIRS),-L$(dir)/lib) \
					$(LIBPSL1GHT_LIB)

.PHONY: $(BUILD) clean spu

#---------------------------------------------------------------------------------
# Primero compilar el programa SPU, luego el PPU
#---------------------------------------------------------------------------------
all: spu $(BUILD)
	@echo "Build complete: $(TARGET)"

spu:
	@$(MAKE) -C spu
	@$(MAKE) -C spu install

#---------------------------------------------------------------------------------
$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(BUILD) $(OUTPUT).elf $(OUTPUT).self data/spu.bin
	@$(MAKE) -C spu clean

#---------------------------------------------------------------------------------
# Generar .self y .pkg
#---------------------------------------------------------------------------------
pkg: all
	@$(MAKE_SELF) $(OUTPUT).elf $(OUTPUT).self $(CONTENTID)
	@$(MAKE_PKG) --content-id $(CONTENTID) $(OUTPUT).pkg $(CURDIR)/USRDIR

#---------------------------------------------------------------------------------
else

DEPENDS	:= $(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
$(OUTPUT).elf: $(OFILES)

#---------------------------------------------------------------------------------
# This rule links in binary data with the .bin extension
#---------------------------------------------------------------------------------
%.bin.o: %.bin
	@echo $(notdir $<)
	@$(bin2o) $< $(notdir $(<:.bin=_bin)) $@

-include $(DEPENDS)

#---------------------------------------------------------------------------------
endif