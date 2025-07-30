#Configuration, name, folders
NAME    = cdictest
OUTPUT  = out
BUILD   = build
SRC     = src

#COMPILER AND LIBRARY LOCATIONS
CDISDK = D:
OS9C   = $(CDISDK)/DOS/BIN
OS68K  = $(CDISDK)/OS9/68000
OS9CDI = $(CDISDK)/OS9/CDI
XCLIB  = $(OS68K)/lib
XCDEF  = $(OS68K)/defs
CLIB   = $(OS9CDI)/lib
CDEF   = $(OS9CDI)/defs
PATH   = $(OS9C);%PATH%
DOSBOX = C:/Software/DOSBox-0.74-3/DOSBox.exe
#COMPILER CONFIGURATION
CC      = xcc
AS      = xcc
CCFLAGS = -eas=$(OUTPUT) -tp=68K,sc -v=$(CDEF) -bc -r
ASFLAGS = -O=0 -S -R=$(OUTPUT) -T=$(OUTPUT) -TO=osk -TP=68kI
LD      = l68

#FILES TO COMPILE
FILES   = $(OUTPUT)/main.r $(OUTPUT)/irq.r $(OUTPUT)/crc.r $(OUTPUT)/slave.r \
	$(OUTPUT)/test_mode1_read.r \
	$(OUTPUT)/test_toc_read.r \
	$(OUTPUT)/test_xa_play.r \
	$(OUTPUT)/test_cdda_play.r \
	$(OUTPUT)/test_mode2_read.r \
	$(OUTPUT)/test_audiomap.r \
	$(OUTPUT)/test_audiomap_to_xa_play.r \
	$(OUTPUT)/ribbit_sample.r \
	$(OUTPUT)/sine_sample.r


#LINKER CONFIGURATION
LDPARAM = -a -n=cdi_$(NAME) -o=$(BUILD)/$(NAME) $(CLIB)/cstart.r $(FILES) -l=$(CLIB)/cdi.l -l=$(CLIB)/cdisys.l -l=$(CLIB)/clib.l -l=$(CLIB)/cio.l -l=$(CLIB)/math.l -l=$(CLIB)/sys.l -l=$(CLIB)/usr.l -l=$(XCLIB)/os_csl.l

cd: link
	$(DOSBOX) master.bat -exit

all: link

rebuild: clean cd

link_app: $(FILES)
	$(LD) -z=link.txt -o=build\$(NAME).app -n=play
	fixmod -ua=80ff $(BUILD)/$(NAME).app

link_cd: $(FILES)
	$(LD) -z=link.txt -o=build\$(NAME)
	fixmod -uo=0.0 $(BUILD)/$(NAME)

$(OUTPUT)/irq.r: $(SRC)/irq.a
	$(AS) $(CCFLAGS) -O=2 $(SRC)/irq.a

$(OUTPUT)/main.r: $(SRC)/main.c
	$(CC) $(CCFLAGS) -O=2 $(SRC)/main.c

$(OUTPUT)/crc.r: $(SRC)/crc.c
	$(CC) $(CCFLAGS) -O=2 $(SRC)/crc.c

$(OUTPUT)/slave.r: $(SRC)/slave.c
	$(CC) $(CCFLAGS) -O=2 $(SRC)/slave.c

$(OUTPUT)/test_mode2_read.r: $(SRC)/test_mode2_read.c
	$(CC) $(CCFLAGS) -O=2 $(SRC)/test_mode2_read.c

$(OUTPUT)/test_xa_play.r: $(SRC)/test_xa_play.c
	$(CC) $(CCFLAGS) -O=2 $(SRC)/test_xa_play.c

$(OUTPUT)/test_mode1_read.r: $(SRC)/test_mode1_read.c
	$(CC) $(CCFLAGS) -O=2 $(SRC)/test_mode1_read.c

$(OUTPUT)/test_cdda_play.r: $(SRC)/test_cdda_play.c
	$(CC) $(CCFLAGS) -O=2 $(SRC)/test_cdda_play.c

$(OUTPUT)/test_toc_read.r: $(SRC)/test_toc_read.c
	$(CC) $(CCFLAGS) -O=2 $(SRC)/test_toc_read.c

$(OUTPUT)/test_audiomap.r: $(SRC)/test_audiomap.c
	$(CC) $(CCFLAGS) -O=2 $(SRC)/test_audiomap.c

$(OUTPUT)/test_audiomap_to_xa_play.r: $(SRC)/test_audiomap_to_xa_play.c
	$(CC) $(CCFLAGS) -O=2 $(SRC)/test_audiomap_to_xa_play.c

$(OUTPUT)/ribbit_sample.r: $(SRC)/ribbit_sample.c
	$(CC) $(CCFLAGS) -O=2 $(SRC)/ribbit_sample.c

$(OUTPUT)/sine_sample.r: $(SRC)/sine_sample.c
	$(CC) $(CCFLAGS) -O=2 $(SRC)/sine_sample.c

clean:
	-@erase $(OUTPUT)/cm*
	-@erase $(OUTPUT)/*.tmp
	-@erase $(OUTPUT)/*.r
	-@erase $(BUILD)/$(NAME)

purge:
	-@erase $(OUTPUT)/cm*
	-@erase $(OUTPUT)/*.tmp
