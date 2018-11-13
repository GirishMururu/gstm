# ==============================================================================
#
# Defines.common.mk
#
# ==============================================================================


CC       := gcc -std=gnu99
CFLAGS   +=-Wall -pthread 
CFLAGS   += -O3  
CFLAGS   += -I$(LIB)
CPP      := g++
CPPFLAGS += $(CFLAGS)
LD       := g++
LIBS     += -lpthread -lrt

# Remove these files when doing clean
OUTPUT +=

LIB := ../lib

STM := ../../tl2-x86-0.9.6

LOSTM := ../../OpenTM/lostm


# ==============================================================================
#
# End of Defines.common.mk
#
# ==============================================================================
