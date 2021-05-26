# Makefile to small project with two bin targets
TARGET_SERVER ?= server
TARGET_CLIENT ?= client
SRC_DIRS ?= ./src

SRCS_SERVER := $(SRC_DIRS)/server.c
SRCS_CLIENT := $(SRC_DIRS)/client.c
SRCS_COMMON := $(SRC_DIRS)/common.c
OBJS_SERVER := $(addsuffix .o,$(basename $(SRCS_SERVER)))
OBJS_CLIENT := $(addsuffix .o,$(basename $(SRCS_CLIENT)))
OBJS_COMMON := $(addsuffix .o,$(basename $(SRCS_COMMON)))
OBJS := $(OBJS_SERVER) $(OBJS_CLIENT) $(OBJS_COMMON)
DEPS_SERVER := $(OBJS_SERVER:.o=.d)
DEPS_CLIENT := $(OBJS_CLIENT:.o=.d)
DEPS_COMMON := $(OBJS_COMMON:.o=.d)
DEPS := $(DEPS_SERVER) $(DEPS_CLIENT) $(DEPS_COMMON)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CC = gcc
USER_FLAGS = 
#CFLAGS ?= $(INC_FLAGS) $(USER_FLAGS) -MMD -MP -Wall -Wextra -Wpedantic -Werror
CFLAGS ?= $(INC_FLAGS) $(USER_FLAGS) -MMD -MP -Wall -Wextra

all: $(TARGET_SERVER) $(TARGET_CLIENT)

$(TARGET_SERVER): $(OBJS_SERVER) $(OBJS_COMMON)
	$(CC) $(LDFLAGS) $(OBJS_SERVER) $(OBJS_COMMON) -o $@ $(LOADLIBES) $(LDLIBS)

$(TARGET_CLIENT): $(OBJS_CLIENT) $(OBJS_COMMON)
	$(CC) $(LDFLAGS) $(OBJS_CLIENT) $(OBJS_COMMON) -o $@ $(LOADLIBES) $(LDLIBS)

.PHONY: all clean
.PRECIOUS: %.o

clean:
	$(RM) $(TARGET_SERVER) $(TARGET_CLIENT) $(OBJS) $(DEPS)

-include $(DEPS)

