PROJ = xilprg

########################################
# 
VER_MJ := $(shell gawk '/VERSION_MAJOR/ {print $$3}' src/xilprg.h)
VER_MN := $(shell gawk '/VERSION_MINOR/ {print $$3}' src/xilprg.h)

########################################
# 
IS_CYGWIN=$(shell uname | grep -i cygwin > /dev/null 2>&1 && echo yes || echo no)

########################################
# Directories

SRC_DIR = src
ifeq ($(IS_CYGWIN),yes)
OBJ_DIR = obj/cygwin
else
OBJ_DIR = obj/linux
endif 

ifeq ($(IS_CYGWIN),yes)
BIN_EXT = .exe
else
BIN_EXT =
endif

XILPRG_DIR = xilprg-$(VER_MJ).$(VER_MN)

########################################
# Compiler options

SRCS = \
	$(SRC_DIR)/xilprg.cpp \
	$(SRC_DIR)/cmdline.cpp \
	$(SRC_DIR)/cmds.cpp \
	$(SRC_DIR)/utils.cpp \
	$(SRC_DIR)/vartable.cpp \
	$(SRC_DIR)/strtable.cpp \
	$(SRC_DIR)/prgfile.cpp \
	$(SRC_DIR)/chip.cpp \
	$(SRC_DIR)/parport.cpp \
	$(SRC_DIR)/cable.cpp \
	$(SRC_DIR)/digilent.cpp \
	$(SRC_DIR)/znuhtag.cpp \
	$(SRC_DIR)/xilinx.cpp \
	$(SRC_DIR)/amontec.cpp \
	$(SRC_DIR)/spi.cpp

OBJS = $(addsuffix .o, \
		$(addprefix $(OBJ_DIR)/, \
		$(basename $(notdir $(SRCS)))))
TARG = $(OBJ_DIR)/$(PROJ)$(BIN_EXT)

CC    = g++
LD    = g++

CPFLAGS = -g -Wall
LDFLAGS =

ifeq ($(IS_CYGWIN),yes)
LIBS = -lioperm -lreadline
else
LIBS = -lusb -lreadline -lncurses -lftdi
endif

########################################
# Rules

all: mkdirs $(TARG)

clean:
	-rm -rf $(OBJ_DIR)

$(TARG) : $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) $(LIBS) -o $@

mkdirs:
	-mkdir -p $(OBJ_DIR)

srctar:
	tar -cjvf $(XILPRG_DIR)-src.tar.bz2 -C .. \
		$(XILPRG_DIR)/src \
		$(XILPRG_DIR)/makefile \
		$(XILPRG_DIR)/xilprg.dsp \
		$(XILPRG_DIR)/xilprg.dsw \
		$(XILPRG_DIR)/xilprg.conf \
		$(XILPRG_DIR)/readme.txt \
		$(XILPRG_DIR)/license.txt

winbintar:
	tar -cjvf $(XILPRG_DIR)-win32.tar.bz2 \
		xilprg.conf readme.txt license.txt \
		-C obj/win32/Release xilprg.exe

linuxbintar:
	tar -cjvf $(XILPRG_DIR)-linux.tar.bz2 \
		xilprg.conf readme.txt license.txt \
		-C obj/linux xilprg

tar: srctar winbintar linuxbintar

#

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp
	$(CC) -c $(CPFLAGS) $< -o $@
