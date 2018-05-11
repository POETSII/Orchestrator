# Tinsel root
export TINSEL_ROOT=../Tinsel
export SOFTSW_ROOT=../Softswitch

# Common directories
TINSELINC = $(realpath $(TINSEL_ROOT)/include)
TINSELLIB = $(realpath $(TINSEL_ROOT)/lib)
TINSELBIN = $(realpath $(TINSEL_ROOT)/bin)
TINSELHL  = $(realpath $(TINSEL_ROOT)/hostlink)

SOFTSWINC = $(realpath $(SOFTSW_ROOT)/inc)
SOFTSWSRC = $(realpath $(SOFTSW_ROOT)/src)

ifndef RISCV_PATH
export RISCV_PATH := /usr/local/riscv
endif

# Application directories
DEFAULT_GEN_DIR = Generated
ifndef GENINC
GENINC := $(realpath ../$(DEFAULT_GEN_DIR)/inc)
endif
ifndef GENSRC
GENSRC := $(realpath ../$(DEFAULT_GEN_DIR)/src)
endif
ifndef BIN_DIR
BIN_DIR := ../bin
endif

# RISC-V tools
RV_ARCH     = rv32imf
RV_CC       = riscv32-unknown-elf-gcc
RV_CPPC     = riscv32-unknown-elf-g++
RV_LD       = riscv32-unknown-elf-ld
RV_OBJCOPY  = riscv32-unknown-elf-objcopy
RV_LIBS     = $(RISCV_PATH)/riscv32-unknown-elf/lib/rv32im/ilp32
RV_GCC_LIBS = $(RISCV_PATH)/lib/gcc/riscv32-unknown-elf/7.2.0/rv32im/ilp32
# this had -nostdlib; we need it for memcpy.
RV_CFLAGS   = -mabi=ilp32 -march=$(RV_ARCH) -static -mcmodel=medany \
              -fvisibility=hidden -nostartfiles \
              -fsingle-precision-constant -fno-builtin-printf

# Extend PATH
export PATH := $(PATH):$(TINSEL_ROOT):$(realpath $(TINSELBIN)):$(SOFTSWINC):$(RISCV_PATH)/bin

# Set path to config script
export CONFIG := $(realpath $(TINSEL_ROOT)/config.py)

# Set make path for prerequisites
# generated files look (first) in the generation directory
vpath vars_%.cpp $(GENSRC)
vpath handlers_%.cpp $(GENSRC)
vpath vars_%.h $(GENINC)
vpath handlers_%.h $(GENINC)
vpath libc.a $(RV_LIBS)
vpath libgcc.a $(RV_GCC_LIBS)
VPATH := $(SOFTSWSRC):$(SOFTSWINC):$(TINSELLIB):$(TINSELINC)

# Local compiler flags
CFLAGS = $(RV_CFLAGS) -O2 -I $(TINSELINC) -I $(SOFTSWINC) -I $(GENINC)
LDFLAGS = -melf32lriscv -G 0 -L$(RV_LIBS) -L$(RV_GCC_LIBS) -lgcc -lc

SECEXPCPPS := $(GENSRC)/%.cpp
SECEXPOS := %.o

# binary targets
TARGETS := $(patsubst $(GENINC)/vars_%.h,$(BIN_DIR)/softswitch_%.elf,$(wildcard $(GENINC)/vars_*.h))
# targets for thread-local variables 
THREADVARS := $(patsubst $(GENSRC)/%.cpp,%.o,$(wildcard $(GENSRC)/vars_*_*.cpp))

.PHONY: all clean printvars
all : $(TARGETS)
	$(foreach TARGET,$<,cp $(TARGET) $(BIN_DIR)/$(TARGET))

softswitch_%.o : softswitch_%.cpp softswitch_common.h tinsel.h $(TINSELINC)/config.h
	$(RV_CPPC) $(CFLAGS) -Wall -c -o $@ $<

io.o : io.c io.h tinsel.h $(TINSELINC)/config.h 
	$(RV_CPPC) $(CFLAGS) -Wall -c -o $@ $<

## the pattern substitution should find all vars_x_y.o created from vars_x_y.cpp
.SECONDEXPANSION:
$(BIN_DIR)/softswitch_%.elf : softswitch_main.o softswitch_common.o vars_%.o $$(patsubst $$(SECEXPCPPS),$$(SECEXPOS),$$(wildcard $(GENSRC)/vars_$$*_*.cpp)) handlers_%.o io.o link.ld entry.o libc.a libgcc.a
	$(RV_LD) $(LDFLAGS) -T link.ld -o $@ $(filter-out link.ld, $^)

# this convoluted rule (we hope) will establish as a prerequisite for any file
# vars_x_y.cpp to be compiled to vars_x_y.o a header file vars_x.h. We do this
# by a trick: get the c++ file in question via the automatic-variable-substituted
# filename for the subst expression, then use an inner substitution to replace
# the stem (the part x_y) with x.y and run basename to strip off the part after
# the dot. (If basename tries to get clever about recognising 'valid' filenames
# we can append the vars_ prefix after the inner substitution and/or replace the
# _ with .a instead of .)
$(THREADVARS) : vars_%.o: vars_%.cpp $$(subst $$*.cpp,$$(basename $$(subst _,.,$$*)).h,vars_$$*.cpp)
	$(RV_CPPC) $(CFLAGS) -Wall -c -o $@ $<

vars_%.o : vars_%.cpp vars_%.h
	$(RV_CPPC) $(CFLAGS) -Wall -c -o $@ $<

handlers_%.o : handlers_%.cpp handlers_%.h vars_%.h
	$(RV_CPPC) $(CFLAGS) -Wall -c -o $@ $<

entry.o : entry.S
	$(RV_CC) $(CFLAGS) -Wall -c -o $@ $<

link.ld : genld.sh
	$< > link.ld

$(TINSELINC)/config.h : $(CONFIG)
	make -C $(TINSELINC)

$(HL)/%.o :
	make -C $(TINSELHL)

clean :
	rm -f *.o *.elf link.ld $(BIN_DIR)/*.elf
	make clean -C $(TINSELINC)
