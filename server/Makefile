

.PHONY: default all clean
TARGET := uavlink_server
default: $(TARGET)
all: default


WHEREAMI := $(dir $(lastword $(MAKEFILE_LIST)))
TOP      := $(realpath $(WHEREAMI)/../)
$(BUILDDIR) := $(TOP)/$(TARGET)

TARGET = prog
LIBS = -lm
CC = gcc
CFLAGS = -g -Wall


SRC = server.c

$(BUILDDIR)/%.o : $(srcdir)/%.cpp
	$(CC) $(CCFLAGS) -c $< -o $@


OBJECTS = $(foreach SRC,$(patsubst %.c, $(BUILDDIR)/%.o, $(SRC)))


$(TARGET): $(OBJECTS)
    $(CC) $(OBJECTS) -Wall $(LIBS) -o $@

clean:
    -rm -f $(BUILDDIR)*.o
    -rm -f $(TARGET)