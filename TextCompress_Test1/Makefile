# Copyright 2013-2023, MACHBASE Corporation or its subsidiaries.
# All rights reserved.

# $Id:
#

include ./machbase_env.mk

GCCVERSIONGTEQ7 := $(shell expr `gcc -dumpversion | cut -f1 -d.` \>= 7)

ifeq "$(GCCVERSIONGTEQ7)" "1"
    LD_FLAGS += -no-pie
endif

INCLUDES += $(INC_OPT)/$(MACHBASE_HOME)/include

CC_FLAGS += -D_APPEND_TIMETEST_

all : txtcompress1

txtcompress1 : txtcompress1.o
	$(LD_CC) $(LD_FLAGS) $(LD_OUT_OPT)$@ $< $(LIB_OPT)machbasecli$(LIB_AFT)  $(LIBDIR_OPT)$(MACHBASE_HOME)/lib $(LD_LIBS)

txtcompress1.o : txtcompress1.c
	$(COMPILE.cc) $(CC_FLAGS) $(INCLUDES) $(CC_OUT_OPT)$@ $<

clean :
	rm -f *.o txtcompress1
