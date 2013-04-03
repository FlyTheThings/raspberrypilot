

.PHONY: default all clean
TARGET := uavlink_server
default: $(TARGET)
all: default


WHEREAMI := $(dir $(lastword $(MAKEFILE_LIST)))
TOP      := $(realpath $(WHEREAMI)/../)
BUILDDIR := $(TOP)/build/$(TARGET)
OBJDIR   := $(BUILDDIR)/obj

$(shell mkdir -p $(TOP)/build/)
$(shell mkdir -p $(BUILDDIR))
$(shell mkdir -p $(OBJDIR))

LIBS = -lm
CC = gcc
CFLAGS = -g -Wall


SRC = server.c 
SRC += serial.c 
SRC += udp.c
SRC += UAVLink/uavlink.c
SRC += UAVLink/crc.c

INCDIRS = inc
INCDIRS += UAVLink/inc

INCFLAGS = $(patsubst %,-I%,$(INCDIRS))

# Compile: create object files from C source files.
define COMPILE_C_TEMPLATE
$(OBJDIR)/$(notdir $(basename $(1))).o : $(1)
	@echo $(MSG_COMPILING) $$(call toprel, $$<)
	$(CC) $(CFLAGS) $(INCFLAGS) -c $$< -o $$@
endef


$(foreach src, $(SRC), $(eval $(call COMPILE_C_TEMPLATE, $(src))))




ALLSRCBASE = $(notdir $(basename $(SRC)))
OBJECTS = $(addprefix $(OBJDIR)/, $(addsuffix .o, $(ALLSRCBASE)))


uavlink_server: $(OBJECTS)
	$(CC) $(OBJECTS) $(LIBS) $(CFLAGS) -o $(BUILDDIR)/$@

clean:
	-rm -f $(OBJDIR)/*.o
	-rm -f $(BUILDDIR)/$(TARGET)

serve: uavlink_server
	$(info ------ MAKE COMPLETE - NOW RUNNING SERVER ---------)
	$(shell $(BUILDDIR)/uavlink_server)
