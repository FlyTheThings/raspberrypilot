

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

INCDIRS = uavlink/inc

INCFLAGS = $(patsubst %,-I%,$(INCDIRS))

	
$(OBJDIR)/%.o : $(SRC) $(BLD)
	$(CC) $(CFLAGS) $(INCFLAGS) -c $< -o $@

ALLSRCBASE = $(notdir $(basename $(SRC)))
OBJECTS = $(addprefix $(OBJDIR)/, $(addsuffix .o, $(ALLSRCBASE)))
 

uavlink_server: $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) $(CFLAGS) -o $(BUILDDIR)/$@
	
clean:
	-rm -f $(OBJDIR)/*.o
	-rm -f $(TARGET)
	
serve: uavlink_server
	$(info ------ MAKE COMPLETE - NOW RUNNING SERVER ---------)
	$(shell $(BUILDDIR)/uavlink_server)
	