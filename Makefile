#---------------------------------------------------------------------------------
# PPU Makefile - Hola PS3
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
#---------------------------------------------------------------------------------
TARGET		:= hello-ps3
BUILD		:= build
SOURCES		:= source

TITLE		:= Hola Mundo PS3
APPID		:= PSL145310
CONTENTID	:= UP0001-PSL145310_00-0000000000000001

CFLAGS		= -O2 -Wall -mcpu=cell $(MACHDEP) $(INCLUDE)

LD			:= $(CC)
LDFLAGS		= $(MACHDEP) -Wl,-Map,$(notdir $@).map

#---------------------------------------------------------------------------------
# any extra libraries we wish to link with the project
#---------------------------------------------------------------------------------
LIBS		:= -lrt -llv2

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

export VPATH	:= $(foreach dir,$(SOURCES),$(CURDIR)/$(dir))

export DEPSDIR	:= $(CURDIR)/$(BUILD)

export BUILDDIR	:= $(CURDIR)/$(BUILD)

#---------------------------------------------------------------------------------
# automatically build a list of object files for our project
#---------------------------------------------------------------------------------
CFILES		:= $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
SFILES		:= $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.S)))

export OFILES	:= $(CFILES:.c=.o) $(SFILES:.S=.o)

#---------------------------------------------------------------------------------
# build a list of include paths
#---------------------------------------------------------------------------------
export INCLUDE	:= $(foreach dir,$(LIBDIRS),-I$(dir)/include) \
					$(LIBPSL1GHT_INC) \
					-I$(CURDIR)/$(BUILD)

#---------------------------------------------------------------------------------
# build a list of library paths
#---------------------------------------------------------------------------------
export LIBPATHS	:= $(foreach dir,$(LIBDIRS),-L$(dir)/lib) \
					$(LIBPSL1GHT_LIB)

.PHONY: $(BUILD) clean

#---------------------------------------------------------------------------------
all: $(BUILD)
	@echo "Build complete: $(TARGET)"

#---------------------------------------------------------------------------------
$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(BUILD) $(OUTPUT).elf $(OUTPUT).self $(OUTPUT).pkg pkg_temp USRDIR

#---------------------------------------------------------------------------------
pkg: all
	@echo "Generating PKG..."
	@mkdir -p pkg_temp/USRDIR
	@fself $(OUTPUT).elf pkg_temp/USRDIR/EBOOT.BIN
	@python3 $(CURDIR)/scripts/create_sfo.py "$(TITLE)" "$(APPID)" pkg_temp/PARAM.SFO
	@cp -f pkg_temp/PARAM.SFO /PARAM.SFO
	@mkdir -p /USRDIR && cp -f pkg_temp/USRDIR/EBOOT.BIN /USRDIR/EBOOT.BIN
	@python3 /usr/local/ps3dev/bin/pkg.py --contentid=$(CONTENTID) pkg_temp $(OUTPUT).pkg
	@rm -f /PARAM.SFO /USRDIR/EBOOT.BIN && rmdir /USRDIR 2>/dev/null || true
	@echo "PKG created: $(OUTPUT).pkg"

#---------------------------------------------------------------------------------
else

DEPENDS	:= $(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
$(OUTPUT).elf: $(OFILES)

-include $(DEPENDS)

#---------------------------------------------------------------------------------
endif