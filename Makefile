include $(PSL1GHT)/ppu_rules

TARGET		:=	hola_ps3
CONTENT_ID	:=	UP0001-PSL145310_00-0000000000000001
TITLE		:=	Hola Mundo PS3
APPID		:=	PSL145310

all: $(TARGET).self

clean:
	@rm -rf $(TARGET).elf $(TARGET).self $(TARGET).ppu.o